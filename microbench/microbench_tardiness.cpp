#include "microbench.h"

#include <atomic>
#include <bits/chrono.h>
#include <chrono>
#include <condition_variable>
#include <pthread.h>
#include <thread>
#include <string_view>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

__attribute__((aligned(2048)))
static wfe_mutex_lock mutex_lock = WFE_MUTEX_LOCK_INITIALIZER;

__attribute__((aligned(2048)))
static pthread_mutex_t pthread_lock = PTHREAD_MUTEX_INITIALIZER;

__attribute__((aligned(2048)))
static std::atomic<uint32_t> linux_futex{};

#if defined(_M_ARM_64) || defined(_M_ARM_32)
#if defined(_M_ARM_64)
static uint64_t read_cycle_counter() {
	uint64_t result;
	__asm volatile(
		"isb;"
		"mrs %[Res], CNTVCT_EL0;"
		: [Res] "=r" (result));
	return result;
}

#else
static uint64_t read_cycle_counter() {
	uint32_t result_low, result_high;

	// Read cntvct
	__asm volatile(
		"isb;"
		"mrrc p15, 1, %[Res_Lower], %[Res_Upper], c14;"
		: [Res_Lower] "=r" (result_low)
		, [Res_Upper] "=r" (result_high));
	uint64_t result = result_high;
	result <<= 32;
	result |= result_low;
	return result;
}
#endif
#elif defined(_M_X86_64)
static uint64_t read_cycle_counter() {
	__builtin_ia32_lfence();
	return __rdtsc();
}

#elif defined(_M_X86_32)
static uint64_t read_cycle_counter() {
	uint32_t high, low;

	__asm volatile(
	"lfence;"
	"rdtsc;"
	: "=a" (low)
	, "=d" (high)
	:: "memory");

	uint64_t result = high;
	result <<= 32;
	result |= low;
	return result;
}
#else
static uint64_t read_cycle_counter() {
	// Unsupported platform. Read current nanosections from clock_gettime.
	// Hopefully this is a VDSO call on this unsupported platform to be relatively fast.
	// Returns the number of nanoseconds.
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	const uint64_t NanosecondsInSecond = 1000000000ULL;
	return ts.tv_sec * NanosecondsInSecond + ts.tv_nsec;
}
#endif

const uint64_t NanosecondsInSecond = 1000000000ULL;
constexpr uint64_t SecsToNano(uint64_t Seconds) {
	return Seconds * NanosecondsInSecond;
}

template<
	bool init_wfe, bool print_wfe_info, bool needs_non_spin_impl,
	auto lock_func, auto unlock_func,
	auto timeout_func,
	typename lock_type, auto lock,
	bool low_power>
void DoTimeout(uint64_t Nanoseconds) {
	if (init_wfe) {
		wfe_mutex_init();
	}

	if (print_wfe_info) {
		fprintf(stderr, "Wait implementation:         %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type));
		fprintf(stderr, "Wait timeout implementation: %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type_timeout));
		fprintf(stderr, "Monitor granule size min:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_min);
		fprintf(stderr, "Monitor granule size max:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_max);
		fprintf(stderr, "Cycle counter hz:            %ld\n", wfe_mutex_get_features()->cycle_hz);
		fprintf(stderr, "Cycle counter multiplier:    %ld\n", wfe_mutex_get_features()->cycles_per_nanosecond_multiplier);
		fprintf(stderr, "Cycle counter divisor:    %ld\n", wfe_mutex_get_features()->cycles_per_nanosecond_divisor);
	}

	if (needs_non_spin_impl) {
		if (wfe_mutex_get_features()->wait_type == WAIT_TYPE_SPIN) {
			fprintf(stderr, "Wait type was spin instead of a monitor type. Skipping\n");
			return;
		}
	}

	lock_func(lock, low_power);
	for (size_t i = 0; i < 5; ++i) {
		const auto Now = std::chrono::high_resolution_clock::now();
		timeout_func(lock, Nanoseconds, low_power);
		const auto End = std::chrono::high_resolution_clock::now();
		const auto Diff = End - Now;
		const auto TookNS = std::chrono::duration_cast<std::chrono::nanoseconds>(Diff).count();
		fprintf(stderr, "%ld\n", TookNS - Nanoseconds);
	}

	unlock_func(lock);

}

void mutex_timeout_func(wfe_mutex_lock *lock, uint64_t Nanoseconds, bool low_power) {
	wfe_mutex_lock_timedlock(lock, Nanoseconds, low_power);
};

static inline void pthread_mutex_lock_func(pthread_mutex_t *lock, bool low_power) {
	pthread_mutex_lock(lock);
};

static inline void pthread_mutex_unlock_func(pthread_mutex_t *lock) {
	pthread_mutex_unlock(lock);
};

void pthread_mutex_timeout_func(pthread_mutex_t *lock, uint64_t Nanoseconds, bool low_power) {
	timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += Nanoseconds / NanosecondsInSecond;
	ts.tv_nsec += Nanoseconds % NanosecondsInSecond;
	fprintf(stderr, "Sleeping for %ld seconds and %ld nanoseconds\n", ts.tv_sec, ts.tv_nsec);
	int res = pthread_mutex_timedlock(lock, &ts);
	fprintf(stderr, "Res: %d\n", res);
};

static inline void futex_lock_func(std::atomic<uint32_t> *lock, bool low_power) {
	lock->store(1);
};

static inline void futex_unlock_func(std::atomic<uint32_t> *lock) {
	lock->store(0);
};

void futex_timeout_func(std::atomic<uint32_t> *lock, uint64_t Nanoseconds, bool low_power) {
	timespec ts;
	ts.tv_sec = Nanoseconds / NanosecondsInSecond;
	ts.tv_nsec = Nanoseconds % NanosecondsInSecond;

	::syscall(SYS_futex, lock, FUTEX_PRIVATE_FLAG | FUTEX_WAIT, 1, &ts);
};

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <test_name>\n", argv[0]);
		return 0;
	}

	// Spin a thread that does nothing to ensure pthreads doesn't hit specialized non-threaded mutex case.
	std::thread t(Thread);
	t.join();

	std::string_view test = argv[1];

	if (test == "monitor_mutex_unique") {
		constexpr auto lock_func = wfe_mutex_lock_lock;
		constexpr auto unlock_func = wfe_mutex_lock_unlock;
		constexpr auto lock = &mutex_lock;
		constexpr auto timeout_func = mutex_timeout_func;
		using lock_type = std::remove_pointer_t<decltype(lock)>;
		constexpr bool low_power = false;

		DoTimeout<true, true, true, lock_func, unlock_func, timeout_func, lock_type, lock, low_power>(SecsToNano(2));
	}
	else if (test == "monitor_mutex_unique_lp") {
		constexpr auto lock_func = wfe_mutex_lock_lock;
		constexpr auto unlock_func = wfe_mutex_lock_unlock;
		constexpr auto lock = &mutex_lock;
		constexpr auto timeout_func = mutex_timeout_func;
		using lock_type = std::remove_pointer_t<decltype(lock)>;
		constexpr bool low_power = true;

		DoTimeout<true, true, true, lock_func, unlock_func, timeout_func, lock_type, lock, low_power>(SecsToNano(2));
	}

	else if (test == "pthread_mutex") {
		constexpr auto lock_func = pthread_mutex_lock_func;
		constexpr auto unlock_func = pthread_mutex_unlock_func;
		constexpr auto lock = &pthread_lock;
		constexpr auto timeout_func = pthread_mutex_timeout_func;

		using lock_type = std::remove_pointer_t<decltype(lock)>;
		constexpr bool low_power = true;

		DoTimeout<true, false, false, lock_func, unlock_func, timeout_func, lock_type, lock, low_power>(SecsToNano(2));
	}
	else if (test == "futex_wakeup") {
		constexpr auto lock_func = futex_lock_func;
		constexpr auto unlock_func = futex_unlock_func;
		constexpr auto lock = &linux_futex;
		constexpr auto timeout_func = futex_timeout_func;

		using lock_type = std::remove_pointer_t<decltype(lock)>;
		constexpr bool low_power = true;

		DoTimeout<true, false, false, lock_func, unlock_func, timeout_func, lock_type, lock, low_power>(SecsToNano(2));
	}
	return 0;
}
