#include "microbench.h"
#include <atomic>
#include <thread>
#include <string_view>

#include <limits.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>

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

__attribute__((aligned(2048)))
static std::atomic<uint64_t> ThreadRunning{};

__attribute__((aligned(2048)))
static std::atomic<uint64_t> ThreadCounter{};

template<bool low_power>
void CycleLatencyThread_read_write_lock() {
	while (ThreadRunning.load()) {
		Done = 0;

		wfe_mutex_rwlock_wrlock(&read_write_lock, low_power);
		Ready.store(1);
		// Ensure the other thread attempts locking and falls asleep
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		ThreadCounter.store(read_cycle_counter());
		wfe_mutex_rwlock_unlock(&read_write_lock);
		while (Done.load() == 0);
	}
}

template<bool low_power>
void CycleLatencyThread_mutex_lock() {
	while (ThreadRunning.load()) {
		Done = 0;
		wfe_mutex_lock_lock(&mutex_lock, low_power);
		Ready.store(1);
		// Ensure the other thread attempts locking and falls asleep
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		ThreadCounter.store(read_cycle_counter());
		wfe_mutex_lock_unlock(&mutex_lock);
		while (Done.load() == 0);
	}
}

void CycleLatencyThread_pthread_mutex_lock() {
	while (ThreadRunning.load()) {
		Done = 0;
		pthread_mutex_lock(&pthread_lock);
		Ready.store(1);
		// Ensure the other thread attempts locking and falls asleep
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		ThreadCounter.store(read_cycle_counter());
		pthread_mutex_unlock(&pthread_lock);
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

void Test_spinloop_rwlock_unique() {
	fprintf(stderr, "Wait implementation:         %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type));
	fprintf(stderr, "Wait timeout implementation: %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type_timeout));
	fprintf(stderr, "Monitor granule size min:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_min);
	fprintf(stderr, "Monitor granule size max:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_max);

	constexpr size_t IterationCount = 1000;
	{
		auto Begin = std::chrono::high_resolution_clock::now();
		ThreadCounter = 0;
		Ready = 0;
		Done = 0;
		ThreadRunning = 1;
		uint64_t Min {~0ULL};
		uint64_t Average{};
		uint64_t Max = 0;
		std::thread t {CycleLatencyThread_read_write_lock<false>};

		for (size_t i = 0; i < IterationCount; ++i) {
			// Spin-loop until ready.
			while (Ready.load() == 0);
			wfe_mutex_rwlock_wrlock(&read_write_lock, false);
			const uint64_t LocalCounter = read_cycle_counter();
			wfe_mutex_rwlock_unlock(&read_write_lock);
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
		fprintf(stderr, "Wall clock time of test: %ld nanoseconds\n", std::chrono::duration_cast<std::chrono::nanoseconds>(Diff).count());
		fprintf(stderr, "Took %lf cycles latency average for local thread to consume lock\n", (double)Average / (double)IterationCount);
		fprintf(stderr, "\tMin: %ld\n", Min);
		fprintf(stderr, "\tMax: %ld\n", Max);
	}
}

template<bool low_power>
void Test_monitor_rwlock_unique() {
	wfe_mutex_init();

	fprintf(stderr, "Wait implementation:         %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type));
	fprintf(stderr, "Wait timeout implementation: %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type_timeout));
	fprintf(stderr, "Monitor granule size min:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_min);
	fprintf(stderr, "Monitor granule size max:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_max);

	if (wfe_mutex_get_features()->wait_type == WAIT_TYPE_SPIN) {
		fprintf(stderr, "Wait type was spin instead of a monitor type. Skipping\n");
		return;
	}

	constexpr size_t IterationCount = 1000;
	{
		auto Begin = std::chrono::high_resolution_clock::now();
		ThreadCounter = 0;
		Ready = 0;
		Done = 0;
		ThreadRunning = 1;
		uint64_t Min {~0ULL};
		uint64_t Average{};
		uint64_t Max = 0;
		std::thread t {CycleLatencyThread_read_write_lock<low_power>};

		for (size_t i = 0; i < IterationCount; ++i) {
			// Spin-loop until ready.
			while (Ready.load() == 0);
			wfe_mutex_rwlock_wrlock(&read_write_lock, low_power);
			const uint64_t LocalCounter = read_cycle_counter();
			wfe_mutex_rwlock_unlock(&read_write_lock);
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
		fprintf(stderr, "Wall clock time of test: %ld nanoseconds\n", std::chrono::duration_cast<std::chrono::nanoseconds>(Diff).count());
		fprintf(stderr, "Took %lf cycles latency average for local thread to consume lock\n", (double)Average / (double)IterationCount);
		fprintf(stderr, "\tMin: %ld\n", Min);
		fprintf(stderr, "\tMax: %ld\n", Max);
	}
}

void Test_spinloop_rwlock_shared() {
	fprintf(stderr, "Wait implementation:         %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type));
	fprintf(stderr, "Wait timeout implementation: %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type_timeout));
	fprintf(stderr, "Monitor granule size min:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_min);
	fprintf(stderr, "Monitor granule size max:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_max);

	constexpr size_t IterationCount = 1000;
	{
		auto Begin = std::chrono::high_resolution_clock::now();
		ThreadCounter = 0;
		Ready = 0;
		Done = 0;
		ThreadRunning = 1;
		uint64_t Min {~0ULL};
		uint64_t Average{};
		uint64_t Max = 0;
		std::thread t {CycleLatencyThread_read_write_lock<false>};

		for (size_t i = 0; i < IterationCount; ++i) {
			// Spin-loop until ready.
			while (Ready.load() == 0);
			wfe_mutex_rwlock_rdlock(&read_write_lock, false);
			const uint64_t LocalCounter = read_cycle_counter();
			wfe_mutex_rwlock_read_unlock(&read_write_lock);
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
		fprintf(stderr, "Wall clock time of test: %ld nanoseconds\n", std::chrono::duration_cast<std::chrono::nanoseconds>(Diff).count());
		fprintf(stderr, "Took %lf cycles latency average for local thread to consume lock\n", (double)Average / (double)IterationCount);
		fprintf(stderr, "\tMin: %ld\n", Min);
		fprintf(stderr, "\tMax: %ld\n", Max);
	}
}

template<bool low_power>
void Test_monitor_rwlock_shared() {
	wfe_mutex_init();

	fprintf(stderr, "Wait implementation:         %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type));
	fprintf(stderr, "Wait timeout implementation: %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type_timeout));
	fprintf(stderr, "Monitor granule size min:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_min);
	fprintf(stderr, "Monitor granule size max:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_max);

	if (wfe_mutex_get_features()->wait_type == WAIT_TYPE_SPIN) {
		fprintf(stderr, "Wait type was spin instead of a monitor type. Skipping\n");
		return;
	}

	constexpr size_t IterationCount = 1000;
	{
		auto Begin = std::chrono::high_resolution_clock::now();
		ThreadCounter = 0;
		Ready = 0;
		Done = 0;
		ThreadRunning = 1;
		uint64_t Min {~0ULL};
		uint64_t Average{};
		uint64_t Max = 0;
		std::thread t {CycleLatencyThread_read_write_lock<low_power>};

		for (size_t i = 0; i < IterationCount; ++i) {
			// Spin-loop until ready.
			while (Ready.load() == 0);
			wfe_mutex_rwlock_rdlock(&read_write_lock, low_power);
			const uint64_t LocalCounter = read_cycle_counter();
			wfe_mutex_rwlock_read_unlock(&read_write_lock);
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
		fprintf(stderr, "Wall clock time of test: %ld nanoseconds\n", std::chrono::duration_cast<std::chrono::nanoseconds>(Diff).count());
		fprintf(stderr, "Took %lf cycles latency average for local thread to consume lock\n", (double)Average / (double)IterationCount);
		fprintf(stderr, "\tMin: %ld\n", Min);
		fprintf(stderr, "\tMax: %ld\n", Max);
	}
}

template<bool low_power>
void Test_spinloop_lock_unique() {
	fprintf(stderr, "Wait implementation:         %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type));
	fprintf(stderr, "Wait timeout implementation: %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type_timeout));
	fprintf(stderr, "Monitor granule size min:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_min);
	fprintf(stderr, "Monitor granule size max:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_max);

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
		std::thread t {CycleLatencyThread_mutex_lock<low_power>};

		for (size_t i = 0; i < IterationCount; ++i) {
			// Spin-loop until ready.
			while (Ready.load() == 0);
			wfe_mutex_lock_lock(&mutex_lock, low_power);
			const uint64_t LocalCounter = read_cycle_counter();
			wfe_mutex_lock_unlock(&mutex_lock);
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
		fprintf(stderr, "Wall clock time of test: %ld nanoseconds\n", std::chrono::duration_cast<std::chrono::nanoseconds>(Diff).count());
		fprintf(stderr, "Took %lf cycles latency average for local thread to consume lock\n", (double)Average / (double)IterationCount);
		fprintf(stderr, "\tMin: %ld\n", Min);
		fprintf(stderr, "\tMax: %ld\n", Max);
	}
}

template<bool low_power>
void Test_monitor_lock_unique() {
	wfe_mutex_init();

	fprintf(stderr, "Wait implementation:         %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type));
	fprintf(stderr, "Wait timeout implementation: %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type_timeout));
	fprintf(stderr, "Monitor granule size min:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_min);
	fprintf(stderr, "Monitor granule size max:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_max);

	if (wfe_mutex_get_features()->wait_type == WAIT_TYPE_SPIN) {
		fprintf(stderr, "Wait type was spin instead of a monitor type. Skipping\n");
		return;
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
		std::thread t {CycleLatencyThread_mutex_lock<low_power>};

		for (size_t i = 0; i < IterationCount; ++i) {
			// Spin-loop until ready.
			while (Ready.load() == 0);
			wfe_mutex_lock_lock(&mutex_lock, low_power);
			const uint64_t LocalCounter = read_cycle_counter();
			wfe_mutex_lock_unlock(&mutex_lock);
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
		fprintf(stderr, "Wall clock time of test: %ld nanoseconds\n", std::chrono::duration_cast<std::chrono::nanoseconds>(Diff).count());
		fprintf(stderr, "Took %lf cycles latency average for local thread to consume lock\n", (double)Average / (double)IterationCount);
		fprintf(stderr, "\tMin: %ld\n", Min);
		fprintf(stderr, "\tMax: %ld\n", Max);
	}
}

void Test_pthread_lock_unique() {
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
		std::thread t {CycleLatencyThread_pthread_mutex_lock};

		for (size_t i = 0; i < IterationCount; ++i) {
			// Spin-loop until ready.
			while (Ready.load() == 0);
			pthread_mutex_lock(&pthread_lock);
			const uint64_t LocalCounter = read_cycle_counter();
			pthread_mutex_unlock(&pthread_lock);
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
		fprintf(stderr, "Wall clock time of test: %ld nanoseconds\n", std::chrono::duration_cast<std::chrono::nanoseconds>(Diff).count());
		fprintf(stderr, "Took %lf cycles latency average for local thread to consume lock\n", (double)Average / (double)IterationCount);
		fprintf(stderr, "\tMin: %ld\n", Min);
		fprintf(stderr, "\tMax: %ld\n", Max);
	}
}

void Test_pthread_rwlock_shared() {
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
		std::thread t {CycleLatencyThread_pthread_rw_lock};

		for (size_t i = 0; i < IterationCount; ++i) {
			// Spin-loop until ready.
			while (Ready.load() == 0);
			while (pthread_rwlock_rdlock(&pthread_read_write_lock) != 0);
			const uint64_t LocalCounter = read_cycle_counter();
			pthread_rwlock_unlock(&pthread_read_write_lock);
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
		fprintf(stderr, "Wall clock time of test: %ld nanoseconds\n", std::chrono::duration_cast<std::chrono::nanoseconds>(Diff).count());
		fprintf(stderr, "Took %lf cycles latency average for local thread to consume lock\n", (double)Average / (double)IterationCount);
		fprintf(stderr, "\tMin: %ld\n", Min);
		fprintf(stderr, "\tMax: %ld\n", Max);
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
		fprintf(stderr, "Wall clock time of test: %ld nanoseconds\n", std::chrono::duration_cast<std::chrono::nanoseconds>(Diff).count());
		fprintf(stderr, "Took %lf cycles latency average for local thread to consume lock\n", (double)Average / (double)IterationCount);
		fprintf(stderr, "\tMin: %ld\n", Min);
		fprintf(stderr, "\tMax: %ld\n", Max);
	}
}

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <test_name>\n", argv[0]);
		return 0;
	}

	std::string_view test = argv[1];
	if (test == "spinloop_rw_unique") {
		Test_spinloop_rwlock_unique();
	}
	else if (test == "monitor_rw_unique") {
		Test_monitor_rwlock_unique<false>();
	}
	else if (test == "spinloop_rw_shared") {
		Test_spinloop_rwlock_shared();
	}
	else if (test == "monitor_rw_shared") {
		Test_monitor_rwlock_shared<false>();
	}
	else if (test == "spinloop_mutex_unique") {
		Test_spinloop_lock_unique<false>();
	}
	else if (test == "spinloop_mutex_unique_lp") {
		Test_spinloop_lock_unique<true>();
	}
	else if (test == "monitor_mutex_unique") {
		Test_monitor_lock_unique<false>();
	}
	else if (test == "monitor_rw_unique_lp") {
		Test_monitor_rwlock_unique<true>();
	}
	else if (test == "monitor_rw_shared_lp") {
		Test_monitor_rwlock_shared<true>();
	}
	else if (test == "monitor_mutex_unique_lp") {
		Test_monitor_lock_unique<true>();
	}
	else if (test == "pthread_rw_shared") {
		Test_pthread_rwlock_shared();
	}
	else if (test == "pthread_mutex_unique") {
		Test_pthread_lock_unique();
	}
	else if (test == "futex_wakeup") {
		Test_futex();
	}
	else {
		fprintf(stderr, "Unknown microbench\n");
	}
	return 0;
}
