#pragma once
#include <stdint.h>

#if defined(_M_X86_64) || defined(_M_X86_32)
#if defined(_M_X86_64)
static inline uint64_t read_cycle_counter() {
	return __rdtsc();
}
#elif defined(_M_X86_32)
static inline uint64_t read_cycle_counter() {
	uint32_t high, low;

	__asm volatile(
	"rdtsc;\n"
	: "=a" (low)
	, "=d" (high)
	:: "memory");

	uint64_t result = high;
	result <<= 32;
	result |= low;
	return result;
}
#endif
static inline void do_yield() {
	__asm volatile("pause;\n");
}
#endif
