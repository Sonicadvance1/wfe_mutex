#include "microbench.h"

#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdio>

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

static uint64_t Value{};
int main() {

	wfe_mutex_init();

	fprintf(stderr, "Wait implementation:         %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type));
	fprintf(stderr, "Wait timeout implementation: %s\n", get_wait_type_name(wfe_mutex_get_features()->wait_type_timeout));
	fprintf(stderr, "Monitor granule size min:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_min);
	fprintf(stderr, "Monitor granule size max:    %d\n", wfe_mutex_get_features()->monitor_granule_size_bytes_max);
	fprintf(stderr, "Cycle counter hz:            %" PRId64 "\n", wfe_mutex_get_features()->cycle_hz);

	size_t LoopAmount = 5000;
	uint64_t Total{};
	uint64_t Max{};
	uint64_t Min{~0ULL};
	for (size_t i = 0; i < LoopAmount; ++i) {
		uint64_t Cycles = read_cycle_counter();
		wfe_mutex_get_features()->wait_for_value_spurious_oneshot_i64(&Value, 1, false);
		Cycles = read_cycle_counter() - Cycles;
		Total += Cycles;
		Max = std::max(Cycles, Max);
		Min = std::min(Cycles, Min);
	}

	fprintf(stderr, "Min, Max: %ld, %ld\n", Min, Max);
	double Average = (double)Total / (double)LoopAmount;
	fprintf(stderr, "Average: %lf\n", Average);
	return 0;
}
