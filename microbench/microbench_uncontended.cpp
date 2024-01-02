#include "microbench.h"

#include <chrono>
#include <cmath>

#include <stdio.h>
#include <vector>
#include <pthread.h>
#include <thread>

int main() {
	size_t Count = CalculateDesiredSpinCount();
	constexpr size_t IterationCount = 5;

	{
		fprintf(stderr, "rwlock - shared lock - spinloop\n");
		for (size_t j = 0; j < IterationCount; ++j) {
			wfe_mutex_rwlock lock = WFE_MUTEX_RWLOCK_INITIALIZER;
			Benchmark (Count, [&lock]() {
				wfe_mutex_rwlock_rdlock(&lock, false);
				wfe_mutex_rwlock_read_unlock(&lock);
			});
		}
	}

	{
		fprintf(stderr, "rwlock - unique lock - spinloop\n");
		for (size_t j = 0; j < IterationCount; ++j) {
			wfe_mutex_rwlock lock = WFE_MUTEX_RWLOCK_INITIALIZER;
			Benchmark (Count, [&lock]() {
				wfe_mutex_rwlock_wrlock(&lock, false);
				wfe_mutex_rwlock_unlock(&lock);
			});
		}
	}

	{
		fprintf(stderr, "mutex - unique lock - spinloop\n");
		for (size_t j = 0; j < IterationCount; ++j) {
			wfe_mutex_lock lock = WFE_MUTEX_LOCK_INITIALIZER;
			Benchmark (Count, [&lock]() {
				wfe_mutex_lock_lock(&lock, false);
				wfe_mutex_lock_unlock(&lock);
			});
		}
	}

	wfe_mutex_init();

	fprintf(stderr, "Wait implementation:         %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type));
	fprintf(stderr, "Wait timeout implementation: %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type_timeout));
	fprintf(stderr, "Monitor granule size min:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_min);
	fprintf(stderr, "Monitor granule size max:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_max);

	// Spin a thread that does nothing to ensure pthreads doesn't hit specialized non-threaded mutex case.
	std::thread t(Thread);
	t.join();

	{
		fprintf(stderr, "rwlock - shared lock\n");
		for (size_t j = 0; j < IterationCount; ++j) {
			wfe_mutex_rwlock lock = WFE_MUTEX_RWLOCK_INITIALIZER;
			Benchmark (Count, [&lock]() {
				wfe_mutex_rwlock_rdlock(&lock, false);
				wfe_mutex_rwlock_read_unlock(&lock);
			});
		}
	}

	{
		fprintf(stderr, "rwlock - unique lock - monitor\n");
		for (size_t j = 0; j < IterationCount; ++j) {
			wfe_mutex_rwlock lock = WFE_MUTEX_RWLOCK_INITIALIZER;
			Benchmark (Count, [&lock]() {
				wfe_mutex_rwlock_wrlock(&lock, false);
				wfe_mutex_rwlock_unlock(&lock);
			});
		}
	}

	{
		fprintf(stderr, "mutex - unique lock\n");
		for (size_t j = 0; j < IterationCount; ++j) {
			wfe_mutex_lock lock = WFE_MUTEX_LOCK_INITIALIZER;
			Benchmark (Count, [&lock]() {
				wfe_mutex_lock_lock(&lock, false);
				wfe_mutex_lock_unlock(&lock);
			});
		}
	}

	{
		fprintf(stderr, "pthread rwlock - shared lock\n");
		for (size_t j = 0; j < IterationCount; ++j) {
			pthread_rwlock_t lock = PTHREAD_RWLOCK_INITIALIZER;
			Benchmark (Count, [&lock]() {
				while (pthread_rwlock_rdlock(&lock) != 0);
				pthread_rwlock_unlock(&lock);
			});
		}
	}

	{
		fprintf(stderr, "pthread mutex - unique lock\n");
		for (size_t j = 0; j < IterationCount; ++j) {
			pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
			Benchmark (Count, [&lock]() {
				while (pthread_mutex_lock(&lock) != 0);
				pthread_mutex_unlock(&lock);
			});
		}
	}
}
