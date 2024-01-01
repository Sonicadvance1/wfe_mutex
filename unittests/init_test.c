#include <wfe_mutex/wfe_mutex.h>

#include <stdio.h>
#include <time.h>

uint64_t ms_to_ns(uint64_t milliseconds) {
	const uint64_t NanosecondsInMillisecond = 1000000ULL;
	return milliseconds * NanosecondsInMillisecond;
}

uint64_t gettime_in_nanoseconds() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	const uint64_t NanosecondsInSecond = 1000000000ULL;
	return ts.tv_sec * NanosecondsInSecond + ts.tv_nsec;
}

bool wait_timeout_u8() {
	uint8_t Value = 0;
	bool Result = wfe_mutex_wait_for_value_timeout_i8(&Value, 1, 0, false);
	fprintf(stderr, "Result: %d\n", Result);

	size_t begin = gettime_in_nanoseconds();
	Result = wfe_mutex_wait_for_value_timeout_i8(&Value, 1, ms_to_ns(1999), false);
	size_t end = gettime_in_nanoseconds();

	fprintf(stderr, "Result: %d: Timed out after %zu nanoseconds\n", Result, end - begin);

	Result = wfe_mutex_wait_for_value_timeout_i8(&Value, 0, 0, false);
	fprintf(stderr, "Result: %d\n", Result);

	return true;
}

int init_test() {
	wfe_mutex_init();
	const wfe_mutex_features *Features = wfe_mutex_get_features();
	fprintf(stderr, "Cycle hz: %ld\n", Features->cycle_hz);

	if (!wait_timeout_u8()) return 1;
	return 0;
}
