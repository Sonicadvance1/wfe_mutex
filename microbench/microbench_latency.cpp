#include "microbench.h"
#include <atomic>
#include <thread>
#include <string_view>

#include <cinttypes>
#include <limits.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <map>
#include <string>

// Ensure these atomics are worst-case far away from each other.
__attribute__((aligned(2048)))
static std::atomic<uint32_t> Ready{};
__attribute__((aligned(2048)))
static std::atomic<uint32_t> Done{};

__attribute__((aligned(2048)))
static wfe_mutex_rwlock read_write_lock = WFE_MUTEX_RWLOCK_INITIALIZER;

__attribute__((aligned(2048)))
static wfe_mutex_lock mutex_lock = WFE_MUTEX_LOCK_INITIALIZER;

__attribute__((aligned(2048)))
static pthread_rwlock_t pthread_read_write_lock = PTHREAD_RWLOCK_INITIALIZER;

__attribute__((aligned(2048)))
static pthread_mutex_t pthread_lock = PTHREAD_MUTEX_INITIALIZER;

__attribute__((aligned(2048)))
static std::atomic<uint32_t> linux_futex{};

#if defined(_M_ARM_64) || defined(_M_ARM_32)
#if defined(_M_ARM_64)
static uint64_t read_cycle_counter() {
	uint64_t result;
	__asm volatile(
		"isb;\n"
		"mrs %[Res], CNTVCT_EL0;\n"
		: [Res] "=r" (result));
	return result;
}

#else
static uint64_t read_cycle_counter() {
	uint32_t result_low, result_high;

	// Read cntvct
	__asm volatile(
		"isb;\n"
		"mrrc p15, 1, %[Res_Lower], %[Res_Upper], c14;\n"
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
	"lfence;\n"
	"rdtsc;\n"
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

__attribute__((aligned(2048)))
static std::atomic<uint64_t> ThreadRunning{};

__attribute__((aligned(2048)))
static std::atomic<uint64_t> ThreadCounter{};

template<auto lock_func, auto unlock_func, bool low_power, typename lock_type>
void template_read_write_lock(lock_type *lock) {
	while (ThreadRunning.load()) {
		Done = 0;

		lock_func(lock, low_power);
		Ready.store(1);
		// Ensure the other thread attempts locking and falls asleep
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		ThreadCounter.store(read_cycle_counter());
		unlock_func(lock);
		while (Done.load() == 0);
	}
}

void CycleLatencyThread_pthread_rw_lock() {
	while (ThreadRunning.load()) {
		Done = 0;
		pthread_rwlock_wrlock(&pthread_read_write_lock);
		Ready.store(1);
		// Ensure the other thread attempts locking and falls asleep
		std::this_thread::sleep_for(std::chrono::milliseconds(5));

		ThreadCounter.store(read_cycle_counter());
		pthread_rwlock_unlock(&pthread_read_write_lock);
		while (Done.load() == 0);
	}
}

void CycleLatencyThread_futex_lock() {
	while (ThreadRunning.load()) {
		Done = 0;
		Ready.store(1);
		while (::syscall(SYS_futex, &linux_futex, FUTEX_WAIT, 0, nullptr) == EAGAIN);
		ThreadCounter.store(read_cycle_counter());
		while (Done.load() == 0);
	}
}

static inline void pthread_mutex_lock_func(pthread_mutex_t *lock, bool low_power) {
	pthread_mutex_lock(lock);
};

static inline void pthread_mutex_unlock_func(pthread_mutex_t *lock) {
	pthread_mutex_unlock(lock);
};

static inline void pthread_rwlock_lock_func(pthread_rwlock_t *lock, bool low_power) {
	pthread_rwlock_wrlock(lock);
};

static inline void pthread_rwlock_unlock_func(pthread_rwlock_t *lock) {
	pthread_rwlock_unlock(lock);
};

static inline void pthread_rwlock_rdlock_func(pthread_rwlock_t *lock, bool low_power) {
	while (pthread_rwlock_wrlock(lock) != 0);
};

static inline void pthread_rwlock_rdunlock_func(pthread_rwlock_t *lock) {
	pthread_rwlock_unlock(lock);
};

template<
	bool init_wfe, bool print_wfe_info, bool needs_non_spin_impl,
	auto lock_func, auto unlock_func,
	auto shared_lock_func, auto shared_unlock_func,
	typename lock_type, auto lock,
	bool low_power>
void Test_mutex_test() {

	if (init_wfe) {
		wfe_mutex_init();
	}

	if (print_wfe_info) {
		fprintf(stderr, "Wait implementation:         %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type));
		fprintf(stderr, "Wait timeout implementation: %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type_timeout));
		fprintf(stderr, "Monitor granule size min:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_min);
		fprintf(stderr, "Monitor granule size max:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_max);
		fprintf(stderr, "Cycle counter hz:            %" PRId64 "\n", wfe_mutex_get_features()->cycle_hz);
	}

	if (needs_non_spin_impl) {
		if (wfe_mutex_get_features()->wait_type == WAIT_TYPE_SPIN) {
			fprintf(stderr, "Wait type was spin instead of a monitor type. Skipping\n");
			return;
		}
	}

	constexpr size_t IterationCount = 100;
	{
		auto Begin = std::chrono::high_resolution_clock::now();
		ThreadCounter = 0;
		Ready = 0;
		Done = 0;
		ThreadRunning = 1;
		uint64_t Min {~0ULL};
		uint64_t Average{};
		uint64_t Max = 0;
		std::thread t {template_read_write_lock<lock_func, unlock_func, low_power, lock_type>, lock};

		for (size_t i = 0; i < IterationCount; ++i) {
			// Spin-loop until ready.
			while (Ready.load() == 0);
			shared_lock_func(lock, low_power);
			const uint64_t LocalCounter = read_cycle_counter();
			shared_unlock_func(lock);
			const auto LocalThreadCounter = ThreadCounter.load();

			const auto Diff = LocalCounter - LocalThreadCounter;
			Average += Diff;
			Min = std::min(Min, Diff);
			Max = std::max(Max, Diff);

			Ready = 0;
			ThreadCounter = 0;
			if ((i + 1) != IterationCount) {
				Done.store(1);
			}
		}

		ThreadRunning.store(0);
		Done.store(1);

		t.join();

		auto End = std::chrono::high_resolution_clock::now();
		auto Diff = End - Begin;
		fprintf(stderr, "Wall clock time of test: %" PRId64 " nanoseconds\n", std::chrono::duration_cast<std::chrono::nanoseconds>(Diff).count());
		fprintf(stderr, "Took %lf cycles latency average for local thread to consume lock\n", (double)Average / (double)IterationCount);
		fprintf(stderr, "\tMin: %" PRId64 "\n", Min);
		fprintf(stderr, "\tMax: %" PRId64 "\n", Max);
	}
}

void Test_futex() {
	constexpr size_t IterationCount = 100;
	{
		auto Begin = std::chrono::high_resolution_clock::now();
		ThreadCounter = 0;
		Ready = 0;
		Done = 0;
		linux_futex = 0;
		ThreadRunning = 1;
		uint64_t Min {~0ULL};
		uint64_t Average{};
		uint64_t Max = 0;
		std::thread t {CycleLatencyThread_futex_lock};

		for (size_t i = 0; i < IterationCount; ++i) {
			// Spin-loop until ready.
			while (Ready.load() == 0);
			// Do a little sleep to ensure the other thread is sleeping on a mutex.
			std::this_thread::sleep_for(std::chrono::milliseconds(50));

			// Write to the futex value and wake it
			linux_futex = 1;
			// Read the counter before doing the syscall.
			// This ensures the time a syscall takes is counted towards wakeup time.
			const uint64_t LocalCounter = read_cycle_counter();
			::syscall(SYS_futex, &linux_futex, FUTEX_WAKE, INT_MAX);

			// Wait for the write to occur.
			while (ThreadCounter.load() == 0);
			const auto LocalThreadCounter = ThreadCounter.load();

			Ready = 0;
			linux_futex = 0;
			ThreadCounter = 0;

			if (LocalThreadCounter < LocalCounter) {
				if ((i + 1) != IterationCount) {
					Done.store(1);
				}
				continue;
			}
			const auto Diff = LocalThreadCounter - LocalCounter;
			Average += Diff;
			Min = std::min(Min, Diff);
			Max = std::max(Max, Diff);

			if ((i + 1) != IterationCount) {
				Done.store(1);
			}
		}

		ThreadRunning.store(0);
		Done.store(1);

		t.join();

		auto End = std::chrono::high_resolution_clock::now();
		auto Diff = End - Begin;
		fprintf(stderr, "Wall clock time of test: %" PRId64 " nanoseconds\n", std::chrono::duration_cast<std::chrono::nanoseconds>(Diff).count());
		fprintf(stderr, "Took %lf cycles latency average for local thread to consume lock\n", (double)Average / (double)IterationCount);
		fprintf(stderr, "\tMin: %" PRId64 "\n", Min);
		fprintf(stderr, "\tMax: %" PRId64 "\n", Max);
	}
}

int main(int argc, char **argv) {
	enum class Test {
		SPINLOOP_RW_UNIQUE,
		SPINLOOP_RW_UNIQUE_LP,
		SPINLOOP_RW_SHARED,
		SPINLOOP_RW_SHARED_LP,
		SPINLOOP_MUTEX_UNIQUE,
		SPINLOOP_MUTEX_UNIQUE_LP,
		MONITOR_RW_UNIQUE,
		MONITOR_RW_UNIQUE_LP,
		MONITOR_RW_SHARED,
		MONITOR_RW_SHARED_LP,
		MONITOR_MUTEX_UNIQUE,
		MONITOR_MUTEX_UNIQUE_LP,
		PTHREAD_RW_SHARED,
		PTHREAD_MUTEX_UNIQUE,
		FUTEX_WAKEUP,

		//< Special
		ALL_NO_LP,
		ALL,
	};

	const static std::map<std::string_view, Test> NameToTest = {{
		{"spinloop_rw_unique",       Test::SPINLOOP_RW_UNIQUE},
		{"spinloop_rw_unique_lp",    Test::SPINLOOP_RW_UNIQUE_LP},
		{"spinloop_rw_shared",       Test::SPINLOOP_RW_SHARED},
		{"spinloop_rw_shared_lp",    Test::SPINLOOP_RW_SHARED_LP},
		{"spinloop_mutex_unique",    Test::SPINLOOP_MUTEX_UNIQUE},
		{"spinloop_mutex_unique_lp", Test::SPINLOOP_MUTEX_UNIQUE_LP},

		{"monitor_rw_unique",       Test::MONITOR_RW_UNIQUE},
		{"monitor_rw_unique_lp",    Test::MONITOR_RW_UNIQUE_LP},
		{"monitor_rw_shared",       Test::MONITOR_RW_SHARED},
		{"monitor_rw_shared_lp",    Test::MONITOR_RW_SHARED_LP},
		{"monitor_mutex_unique",    Test::MONITOR_MUTEX_UNIQUE},
		{"monitor_mutex_unique_lp", Test::MONITOR_MUTEX_UNIQUE_LP},


		{"pthread_rw_shared",       Test::PTHREAD_RW_SHARED},
		{"pthread_mutex_unique",    Test::PTHREAD_MUTEX_UNIQUE},
		{"futex_wakeup",    Test::FUTEX_WAKEUP},

		{"all_no_lp", Test::ALL_NO_LP},
		{"all", Test::ALL},
	}};

	const static std::map<Test, std::string_view> TestToName = {{
		{Test::SPINLOOP_RW_UNIQUE, "spinloop_rw_unique"},
		{Test::SPINLOOP_RW_UNIQUE_LP, "spinloop_rw_unique_lp"},
		{Test::SPINLOOP_RW_SHARED, "spinloop_rw_shared"},
		{Test::SPINLOOP_RW_SHARED_LP, "spinloop_rw_shared_lp"},
		{Test::SPINLOOP_MUTEX_UNIQUE, "spinloop_mutex_unique"},
		{Test::SPINLOOP_MUTEX_UNIQUE_LP, "spinloop_mutex_unique_lp"},

		{Test::MONITOR_RW_UNIQUE, "monitor_rw_unique"},
		{Test::MONITOR_RW_UNIQUE_LP, "monitor_rw_unique_lp"},
		{Test::MONITOR_RW_SHARED, "monitor_rw_shared"},
		{Test::MONITOR_RW_SHARED_LP, "monitor_rw_shared_lp"},
		{Test::MONITOR_MUTEX_UNIQUE, "monitor_mutex_unique"},
		{Test::MONITOR_MUTEX_UNIQUE_LP, "monitor_mutex_unique_lp"},

		{Test::PTHREAD_RW_SHARED, "pthread_rw_shared"},
		{Test::PTHREAD_MUTEX_UNIQUE, "pthread_mutex_unique"},
		{Test::FUTEX_WAKEUP, "futex_wakeup"},

		{Test::ALL_NO_LP, "all_no_lp"},
		{Test::ALL, "all"},
	}};

	const std::vector<Test> TestsAll = {{
		Test::SPINLOOP_RW_UNIQUE,
		Test::SPINLOOP_RW_UNIQUE_LP,
		Test::SPINLOOP_RW_SHARED,
		Test::SPINLOOP_RW_SHARED_LP,
		Test::SPINLOOP_MUTEX_UNIQUE,
		Test::SPINLOOP_MUTEX_UNIQUE_LP,
		Test::MONITOR_RW_UNIQUE,
		Test::MONITOR_RW_UNIQUE_LP,
		Test::MONITOR_RW_SHARED,
		Test::MONITOR_RW_SHARED_LP,
		Test::MONITOR_MUTEX_UNIQUE,
		Test::MONITOR_MUTEX_UNIQUE_LP,
		Test::PTHREAD_RW_SHARED,
		Test::PTHREAD_MUTEX_UNIQUE,
		Test::FUTEX_WAKEUP,
	}};

	const std::vector<Test> TestsNoLP = {{
		Test::SPINLOOP_RW_UNIQUE,
		Test::SPINLOOP_RW_SHARED,
		Test::SPINLOOP_MUTEX_UNIQUE,
		Test::MONITOR_RW_UNIQUE,
		Test::MONITOR_RW_SHARED,
		Test::MONITOR_MUTEX_UNIQUE,
		Test::PTHREAD_RW_SHARED,
		Test::PTHREAD_MUTEX_UNIQUE,
		Test::FUTEX_WAKEUP,
	}};

	std::vector<Test> TestsToRun;

	Test SelectedTest{};
	if (argc < 2) {
		SelectedTest = Test::ALL;
	}
	else {
		std::string_view test = argv[1];
		auto it = NameToTest.find(test);
		if (it == NameToTest.end()) {
			fprintf(stderr, "Unknown test name: '%s'\n", argv[1]);
			return 1;
		}
		SelectedTest = it->second;
	}

	if (SelectedTest == Test::ALL) {
		TestsToRun = TestsAll;
	}
	else if (SelectedTest == Test::ALL_NO_LP) {
		TestsToRun = TestsNoLP;
	}
	else {
		TestsToRun.emplace_back(SelectedTest);
	}

	for (auto Test : TestsToRun) {
		std::string TestName = std::string(TestToName.find(Test)->second);
		fprintf(stderr, "Test: %s\n", TestName.c_str());

		if (Test == Test::SPINLOOP_RW_UNIQUE) {
			constexpr auto lock_func = wfe_mutex_rwlock_wrlock;
			constexpr auto unlock_func = wfe_mutex_rwlock_unlock;
			constexpr auto shared_lock_func = wfe_mutex_rwlock_wrlock;
			constexpr auto shared_unlock_func = wfe_mutex_rwlock_unlock;
			constexpr auto lock = &read_write_lock;
			using lock_type = std::remove_pointer_t<decltype(lock)>;
			constexpr bool low_power = false;

			read_write_lock = WFE_MUTEX_RWLOCK_INITIALIZER;
			Test_mutex_test<false, true, false, lock_func, unlock_func, shared_lock_func, shared_unlock_func, lock_type, lock, low_power>();
		}
		else if (Test == Test::SPINLOOP_RW_UNIQUE_LP) {
			constexpr auto lock_func = wfe_mutex_rwlock_wrlock;
			constexpr auto unlock_func = wfe_mutex_rwlock_unlock;
			constexpr auto shared_lock_func = wfe_mutex_rwlock_wrlock;
			constexpr auto shared_unlock_func = wfe_mutex_rwlock_unlock;
			constexpr auto lock = &read_write_lock;
			using lock_type = std::remove_pointer_t<decltype(lock)>;
			constexpr bool low_power = true;

			read_write_lock = WFE_MUTEX_RWLOCK_INITIALIZER;
			Test_mutex_test<false, true, false, lock_func, unlock_func, shared_lock_func, shared_unlock_func, lock_type, lock, low_power>();
		}
		else if (Test == Test::MONITOR_RW_UNIQUE) {
			constexpr auto lock_func = wfe_mutex_rwlock_wrlock;
			constexpr auto unlock_func = wfe_mutex_rwlock_unlock;
			constexpr auto shared_lock_func = wfe_mutex_rwlock_wrlock;
			constexpr auto shared_unlock_func = wfe_mutex_rwlock_unlock;
			constexpr auto lock = &read_write_lock;
			using lock_type = std::remove_pointer_t<decltype(lock)>;
			constexpr bool low_power = false;

			read_write_lock = WFE_MUTEX_RWLOCK_INITIALIZER;
			Test_mutex_test<true, true, true, lock_func, unlock_func, shared_lock_func, shared_unlock_func, lock_type, lock, low_power>();
		}
		else if (Test == Test::MONITOR_RW_UNIQUE_LP) {
			constexpr auto lock_func = wfe_mutex_rwlock_wrlock;
			constexpr auto unlock_func = wfe_mutex_rwlock_unlock;
			constexpr auto shared_lock_func = wfe_mutex_rwlock_wrlock;
			constexpr auto shared_unlock_func = wfe_mutex_rwlock_unlock;
			constexpr auto lock = &read_write_lock;
			using lock_type = std::remove_pointer_t<decltype(lock)>;
			constexpr bool low_power = true;

			read_write_lock = WFE_MUTEX_RWLOCK_INITIALIZER;
			Test_mutex_test<true, true, true, lock_func, unlock_func, shared_lock_func, shared_unlock_func, lock_type, lock, low_power>();
		}
		else if (Test == Test::SPINLOOP_RW_SHARED) {
			constexpr auto lock_func = wfe_mutex_rwlock_wrlock;
			constexpr auto unlock_func = wfe_mutex_rwlock_unlock;
			constexpr auto shared_lock_func = wfe_mutex_rwlock_rdlock;
			constexpr auto shared_unlock_func = wfe_mutex_rwlock_read_unlock;
			constexpr auto lock = &read_write_lock;
			using lock_type = std::remove_pointer_t<decltype(lock)>;
			constexpr bool low_power = false;

			read_write_lock = WFE_MUTEX_RWLOCK_INITIALIZER;
			Test_mutex_test<false, true, false, lock_func, unlock_func, shared_lock_func, shared_unlock_func, lock_type, lock, low_power>();
		}
		else if (Test == Test::SPINLOOP_RW_SHARED_LP) {
			constexpr auto lock_func = wfe_mutex_rwlock_wrlock;
			constexpr auto unlock_func = wfe_mutex_rwlock_unlock;
			constexpr auto shared_lock_func = wfe_mutex_rwlock_rdlock;
			constexpr auto shared_unlock_func = wfe_mutex_rwlock_read_unlock;
			constexpr auto lock = &read_write_lock;
			using lock_type = std::remove_pointer_t<decltype(lock)>;
			constexpr bool low_power = true;

			Test_mutex_test<false, true, false, lock_func, unlock_func, shared_lock_func, shared_unlock_func, lock_type, lock, low_power>();
		}
		else if (Test == Test::MONITOR_RW_SHARED) {
			constexpr auto lock_func = wfe_mutex_rwlock_wrlock;
			constexpr auto unlock_func = wfe_mutex_rwlock_unlock;
			constexpr auto shared_lock_func = wfe_mutex_rwlock_rdlock;
			constexpr auto shared_unlock_func = wfe_mutex_rwlock_read_unlock;
			constexpr auto lock = &read_write_lock;
			using lock_type = std::remove_pointer_t<decltype(lock)>;
			constexpr bool low_power = false;

			read_write_lock = WFE_MUTEX_RWLOCK_INITIALIZER;
			Test_mutex_test<true, true, true, lock_func, unlock_func, shared_lock_func, shared_unlock_func, lock_type, lock, low_power>();
		}
		else if (Test == Test::MONITOR_RW_SHARED_LP) {
			constexpr auto lock_func = wfe_mutex_rwlock_wrlock;
			constexpr auto unlock_func = wfe_mutex_rwlock_unlock;
			constexpr auto shared_lock_func = wfe_mutex_rwlock_rdlock;
			constexpr auto shared_unlock_func = wfe_mutex_rwlock_read_unlock;
			constexpr auto lock = &read_write_lock;
			using lock_type = std::remove_pointer_t<decltype(lock)>;
			constexpr bool low_power = true;
			read_write_lock = WFE_MUTEX_RWLOCK_INITIALIZER;
			Test_mutex_test<true, true, true, lock_func, unlock_func, shared_lock_func, shared_unlock_func, lock_type, lock, low_power>();
		}
		else if (Test == Test::SPINLOOP_MUTEX_UNIQUE) {
			constexpr auto lock_func = wfe_mutex_lock_lock;
			constexpr auto unlock_func = wfe_mutex_lock_unlock;
			constexpr auto shared_lock_func = wfe_mutex_lock_lock;
			constexpr auto shared_unlock_func = wfe_mutex_lock_unlock;
			constexpr auto lock = &mutex_lock;
			using lock_type = std::remove_pointer_t<decltype(lock)>;
			constexpr bool low_power = false;

			mutex_lock = WFE_MUTEX_LOCK_INITIALIZER;
			Test_mutex_test<false, true, false, lock_func, unlock_func, shared_lock_func, shared_unlock_func, lock_type, lock, low_power>();
		}
		else if (Test == Test::SPINLOOP_MUTEX_UNIQUE_LP) {
			constexpr auto lock_func = wfe_mutex_lock_lock;
			constexpr auto unlock_func = wfe_mutex_lock_unlock;
			constexpr auto shared_lock_func = wfe_mutex_lock_lock;
			constexpr auto shared_unlock_func = wfe_mutex_lock_unlock;
			constexpr auto lock = &mutex_lock;
			using lock_type = std::remove_pointer_t<decltype(lock)>;
			constexpr bool low_power = true;

			mutex_lock = WFE_MUTEX_LOCK_INITIALIZER;
			Test_mutex_test<false, true, false, lock_func, unlock_func, shared_lock_func, shared_unlock_func, lock_type, lock, low_power>();
		}
		else if (Test == Test::MONITOR_MUTEX_UNIQUE) {
			constexpr auto lock_func = wfe_mutex_lock_lock;
			constexpr auto unlock_func = wfe_mutex_lock_unlock;
			constexpr auto shared_lock_func = wfe_mutex_lock_lock;
			constexpr auto shared_unlock_func = wfe_mutex_lock_unlock;
			constexpr auto lock = &mutex_lock;
			using lock_type = std::remove_pointer_t<decltype(lock)>;
			constexpr bool low_power = false;

			mutex_lock = WFE_MUTEX_LOCK_INITIALIZER;
			Test_mutex_test<true, true, true, lock_func, unlock_func, shared_lock_func, shared_unlock_func, lock_type, lock, low_power>();
		}
		else if (Test == Test::MONITOR_MUTEX_UNIQUE_LP) {
			constexpr auto lock_func = wfe_mutex_lock_lock;
			constexpr auto unlock_func = wfe_mutex_lock_unlock;
			constexpr auto shared_lock_func = wfe_mutex_lock_lock;
			constexpr auto shared_unlock_func = wfe_mutex_lock_unlock;
			constexpr auto lock = &mutex_lock;
			using lock_type = std::remove_pointer_t<decltype(lock)>;
			constexpr bool low_power = true;

			mutex_lock = WFE_MUTEX_LOCK_INITIALIZER;
			Test_mutex_test<true, true, true, lock_func, unlock_func, shared_lock_func, shared_unlock_func, lock_type, lock, low_power>();
		}
		else if (Test == Test::PTHREAD_RW_SHARED) {
			constexpr auto lock_func = pthread_rwlock_lock_func;
			constexpr auto unlock_func = pthread_rwlock_unlock_func;
			constexpr auto shared_lock_func = pthread_rwlock_rdlock_func;
			constexpr auto shared_unlock_func = pthread_rwlock_rdunlock_func;
			constexpr auto lock = &pthread_read_write_lock;
			using lock_type = std::remove_pointer_t<decltype(lock)>;

			pthread_read_write_lock = PTHREAD_RWLOCK_INITIALIZER;
			Test_mutex_test<false, false, false, lock_func, unlock_func, shared_lock_func, shared_unlock_func, lock_type, lock, false>();
		}
		else if (Test == Test::PTHREAD_MUTEX_UNIQUE) {
			constexpr auto lock_func = pthread_mutex_lock_func;
			constexpr auto unlock_func = pthread_mutex_unlock_func;
			constexpr auto shared_lock_func = pthread_mutex_lock_func;
			constexpr auto shared_unlock_func = pthread_mutex_unlock_func;
			constexpr auto lock = &pthread_lock;
			using lock_type = std::remove_pointer_t<decltype(lock)>;

			pthread_lock = PTHREAD_MUTEX_INITIALIZER;
			Test_mutex_test<false, false, false, lock_func, unlock_func, shared_lock_func, shared_unlock_func, lock_type, lock, false>();
		}
		else if (Test == Test::FUTEX_WAKEUP) {
			Test_futex();
		}
		else {
			fprintf(stderr, "Unknown microbench\n");
		}
	}
	return 0;
}
