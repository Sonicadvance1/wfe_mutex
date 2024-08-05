#pragma once
#include <stdint.h>

#if defined(_M_ARM_64) || defined(_M_ARM_32)

#if defined(_M_ARM_64)
static inline uint64_t read_cycle_counter() {
	uint64_t result;
	__asm volatile(
		"isb;\n"
		"mrs %[Res], CNTVCT_EL0;\n"
		: [Res] "=r" (result));
	return result;
}
#else
static inline uint64_t read_cycle_counter() {
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

static inline void do_yield() {
	__asm volatile("yield;\n");
}
#endif
