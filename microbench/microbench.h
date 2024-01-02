#pragma once
#define WFE_MUTEX_DEBUG 0

#include <wfe_mutex/wfe_mutex.h>

#include <chrono>
#include <cstddef>
#include <functional>
#include <stdio.h>

static inline void Thread() {
	// Intentionally do nothing.
}

static inline const char *get_wait_type_name(wfe_mutex_wait_types wait_type) {
	switch (wait_type) {
	case WAIT_TYPE_SPIN:
		return "spin";
	case WAIT_TYPE_WFE:
		return "wfe";
	case WAIT_TYPE_WFET:
		return "wfet";
	case WAIT_TYPE_WAITPKG:
		return "waitpkg";
	case WAIT_TYPE_MONITORX:
		return "monitorx";
	default: return "<Unknown>";
	}
}

template<typename F>
void Benchmark(size_t Count, F&& f) {
	const auto Begin = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < Count; ++i) {
		std::forward<F>(f)();
	}
	const auto End = std::chrono::high_resolution_clock::now();
	const auto Diff = End - Begin;
	double LocksPerNanosecond = (double)Count / (double)std::chrono::duration_cast<std::chrono::nanoseconds>(Diff).count();
	double NanosecondInSecond = 1'000'000'000.0;

	fprintf(stderr, "\t%lf\n", LocksPerNanosecond * NanosecondInSecond);
}

static inline size_t CalculateDesiredSpinCount() {
	size_t Count = 10;
	while (true) {
		wfe_mutex_rwlock lock = WFE_MUTEX_RWLOCK_INITIALIZER;

		auto Begin = std::chrono::high_resolution_clock::now();
		for (size_t i = 0; i < Count; ++i) {
			wfe_mutex_rwlock_rdlock(&lock, false);
			wfe_mutex_rwlock_read_unlock(&lock);
		}
		auto End = std::chrono::high_resolution_clock::now();
		auto Diff = End - Begin;
		if (std::chrono::duration_cast<std::chrono::seconds>(Diff) >= std::chrono::seconds(1)) {
			fprintf(stderr, "Took us %zd iterations to take longer than a second\n", Count);
			break;
		}
		Count *= 5;
	}

	return Count;
}
