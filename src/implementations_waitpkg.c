#include "detect.h"
#include "implementations.h"

#include <stdio.h>

#if defined(_M_X86_64)
static uint64_t read_cycle_counter() {
	return __rdtsc();
}

void waitpkg_wait_for_value_i8 (uint8_t *ptr,  uint8_t value, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return;

	if (low_power) {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.2 for larger power savings.
			uint32_t power_state = 0;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");
		}
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}
	else {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.1 for faster wakeup.
			uint32_t power_state = 1;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");
		}
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}
}

void waitpkg_wait_for_value_i16 (uint16_t *ptr,  uint16_t value, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return;

	if (low_power) {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.2 for larger power savings.
			uint32_t power_state = 0;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");
		}
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}
	else {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.1 for faster wakeup.
			uint32_t power_state = 1;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");
		}
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}
}

void waitpkg_wait_for_value_i32 (uint32_t *ptr,  uint32_t value, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return;

	if (low_power) {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.2 for larger power savings.
			uint32_t power_state = 0;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");
		}
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}
	else {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.1 for faster wakeup.
			uint32_t power_state = 1;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");
		}
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}
}

void waitpkg_wait_for_value_i64 (uint64_t *ptr,  uint64_t value, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return;

	if (low_power) {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.2 for larger power savings.
			uint32_t power_state = 0;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");
		}
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}
	else {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.1 for faster wakeup.
			uint32_t power_state = 1;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");
		}
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}
}

uint8_t waitpkg_wait_for_bit_set_i8 (uint8_t *ptr,  uint8_t bit, bool low_power) {
	// Early return if the value is already set.
	uint8_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	if (((result >> bit) & 1) == 1) return result;

	if (low_power) {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.2 for larger power savings.
			uint32_t power_state = 0;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
		while (((result >> bit) & 1) == 0);
	}
	else {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.1 for faster wakeup.
			uint32_t power_state = 1;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
		while (((result >> bit) & 1) == 0);
	}
	return result;
}

uint16_t waitpkg_wait_for_bit_set_i16 (uint16_t *ptr,  uint8_t bit, bool low_power) {
	// Early return if the value is already set.
	uint8_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	if (((result >> bit) & 1) == 1) return result;

	if (low_power) {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.2 for larger power savings.
			uint32_t power_state = 0;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
		while (((result >> bit) & 1) == 0);
	}
	else {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.1 for faster wakeup.
			uint32_t power_state = 1;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
		while (((result >> bit) & 1) == 0);
	}
	return result;
}

uint32_t waitpkg_wait_for_bit_set_i32 (uint32_t *ptr,  uint8_t bit, bool low_power) {
	// Early return if the value is already set.
	uint8_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	if (((result >> bit) & 1) == 1) return result;

	if (low_power) {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.2 for larger power savings.
			uint32_t power_state = 0;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
		while (((result >> bit) & 1) == 0);
	}
	else {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.1 for faster wakeup.
			uint32_t power_state = 1;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
		while (((result >> bit) & 1) == 0);
	}
	return result;
}

uint64_t waitpkg_wait_for_bit_set_i64 (uint64_t *ptr,  uint8_t bit, bool low_power) {
	// Early return if the value is already set.
	uint8_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	if (((result >> bit) & 1) == 1) return result;

	if (low_power) {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.2 for larger power savings.
			uint32_t power_state = 0;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
		while (((result >> bit) & 1) == 0);
	}
	else {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.1 for faster wakeup.
			uint32_t power_state = 1;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
		while (((result >> bit) & 1) == 0);
	}
	return result;
}

uint8_t waitpkg_wait_for_bit_not_set_i8 (uint8_t *ptr,  uint8_t bit, bool low_power) {
	// Early return if the value is already set.
	uint8_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	if (((result >> bit) & 1) == 0) return result;

	if (low_power) {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.2 for larger power savings.
			uint32_t power_state = 0;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
		while (((result >> bit) & 1) == 1);
	}
	else {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.1 for faster wakeup.
			uint32_t power_state = 1;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
		while (((result >> bit) & 1) == 1);
	}
	return result;
}

uint16_t waitpkg_wait_for_bit_not_set_i16 (uint16_t *ptr,  uint8_t bit, bool low_power) {
	// Early return if the value is already set.
	uint8_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	if (((result >> bit) & 1) == 0) return result;

	if (low_power) {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.2 for larger power savings.
			uint32_t power_state = 0;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
		while (((result >> bit) & 1) == 1);
	}
	else {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.1 for faster wakeup.
			uint32_t power_state = 1;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
		while (((result >> bit) & 1) == 1);
	}
	return result;
}

uint32_t waitpkg_wait_for_bit_not_set_i32 (uint32_t *ptr,  uint8_t bit, bool low_power) {
	// Early return if the value is already set.
	uint8_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	if (((result >> bit) & 1) == 0) return result;

	if (low_power) {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.2 for larger power savings.
			uint32_t power_state = 0;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
		while (((result >> bit) & 1) == 1);
	}
	else {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.1 for faster wakeup.
			uint32_t power_state = 1;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
		while (((result >> bit) & 1) == 1);
	}
	return result;
}

uint64_t waitpkg_wait_for_bit_not_set_i64 (uint64_t *ptr,  uint8_t bit, bool low_power) {
	// Early return if the value is already set.
	uint8_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	if (((result >> bit) & 1) == 0) return result;

	if (low_power) {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.2 for larger power savings.
			uint32_t power_state = 0;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
		while (((result >> bit) & 1) == 1);
	}
	else {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.1 for faster wakeup.
			uint32_t power_state = 1;

			// Max timeout, will likely be clamped to IA32_UMWAIT_CONTROL
			uint32_t timeout_lower = ~0U;
			uint32_t timeout_upper = ~0U;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
		while (((result >> bit) & 1) == 1);
	}
	return result;
}

bool waitpkg_wait_for_value_timeout_i8 (uint8_t *ptr,  uint8_t value, uint64_t nanoseconds, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	uint64_t last_cycle_counter = begin_cycles;

	if (low_power) {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.2 for larger power savings.
			uint32_t power_state = 0;

			// Have no way to know if our timeout is larger than IA32_UMWAIT_CONTROL.
			// Although we don't really care if we overrun the os deadline, we'll just loop again.
			// umwait behaviour is slightly different than mwaitx behaviour with timeout.
			// umwait waits until absolute TSC timestamp has elapsed instead of relative cycles.
			uint32_t timeout_lower = cycles_end;
			uint32_t timeout_upper = cycles_end >> 32;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			last_cycle_counter = read_cycle_counter();
			if (last_cycle_counter >= cycles_end) {
				return false;
			}
		}
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}
	else {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.1 for faster wakeup.
			uint32_t power_state = 1;

			// Have no way to know if our timeout is larger than IA32_UMWAIT_CONTROL.
			// Although we don't really care if we overrun the os deadline, we'll just loop again.
			// umwait behaviour is slightly different than mwaitx behaviour with timeout.
			// umwait waits until absolute TSC timestamp has elapsed instead of relative cycles.
			uint32_t timeout_lower = cycles_end;
			uint32_t timeout_upper = cycles_end >> 32;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			last_cycle_counter = read_cycle_counter();
			if (last_cycle_counter >= cycles_end) {
				return false;
			}
		}
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}

	return true;
}

bool waitpkg_wait_for_value_timeout_i16 (uint16_t *ptr,  uint16_t value, uint64_t nanoseconds, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	uint64_t last_cycle_counter = begin_cycles;

	if (low_power) {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.2 for larger power savings.
			uint32_t power_state = 0;

			// Have no way to know if our timeout is larger than IA32_UMWAIT_CONTROL.
			// Although we don't really care if we overrun the os deadline, we'll just loop again.
			// umwait behaviour is slightly different than mwaitx behaviour with timeout.
			// umwait waits until absolute TSC timestamp has elapsed instead of relative cycles.
			uint32_t timeout_lower = cycles_end;
			uint32_t timeout_upper = cycles_end >> 32;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			last_cycle_counter = read_cycle_counter();
			if (last_cycle_counter >= cycles_end) {
				return false;
			}
		}
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}
	else {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.1 for faster wakeup.
			uint32_t power_state = 1;

			// Have no way to know if our timeout is larger than IA32_UMWAIT_CONTROL.
			// Although we don't really care if we overrun the os deadline, we'll just loop again.
			// umwait behaviour is slightly different than mwaitx behaviour with timeout.
			// umwait waits until absolute TSC timestamp has elapsed instead of relative cycles.
			uint32_t timeout_lower = cycles_end;
			uint32_t timeout_upper = cycles_end >> 32;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			last_cycle_counter = read_cycle_counter();
			if (last_cycle_counter >= cycles_end) {
				return false;
			}
		}
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}

	return true;
}

bool waitpkg_wait_for_value_timeout_i32 (uint32_t *ptr,  uint32_t value, uint64_t nanoseconds, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	uint64_t last_cycle_counter = begin_cycles;

	if (low_power) {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.2 for larger power savings.
			uint32_t power_state = 0;

			// Have no way to know if our timeout is larger than IA32_UMWAIT_CONTROL.
			// Although we don't really care if we overrun the os deadline, we'll just loop again.
			// umwait behaviour is slightly different than mwaitx behaviour with timeout.
			// umwait waits until absolute TSC timestamp has elapsed instead of relative cycles.
			uint32_t timeout_lower = cycles_end;
			uint32_t timeout_upper = cycles_end >> 32;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			last_cycle_counter = read_cycle_counter();
			if (last_cycle_counter >= cycles_end) {
				return false;
			}
		}
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}
	else {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.1 for faster wakeup.
			uint32_t power_state = 1;

			// Have no way to know if our timeout is larger than IA32_UMWAIT_CONTROL.
			// Although we don't really care if we overrun the os deadline, we'll just loop again.
			// umwait behaviour is slightly different than mwaitx behaviour with timeout.
			// umwait waits until absolute TSC timestamp has elapsed instead of relative cycles.
			uint32_t timeout_lower = cycles_end;
			uint32_t timeout_upper = cycles_end >> 32;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			last_cycle_counter = read_cycle_counter();
			if (last_cycle_counter >= cycles_end) {
				return false;
			}
		}
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}

	return true;
}

bool waitpkg_wait_for_value_timeout_i64 (uint64_t *ptr,  uint64_t value, uint64_t nanoseconds, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	uint64_t last_cycle_counter = begin_cycles;

	if (low_power) {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.2 for larger power savings.
			uint32_t power_state = 0;

			// Have no way to know if our timeout is larger than IA32_UMWAIT_CONTROL.
			// Although we don't really care if we overrun the os deadline, we'll just loop again.
			// umwait behaviour is slightly different than mwaitx behaviour with timeout.
			// umwait waits until absolute TSC timestamp has elapsed instead of relative cycles.
			uint32_t timeout_lower = cycles_end;
			uint32_t timeout_upper = cycles_end >> 32;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			last_cycle_counter = read_cycle_counter();
			if (last_cycle_counter >= cycles_end) {
				return false;
			}
		}
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}
	else {
		do {
			__asm volatile (
				"umonitor %[ptr]; "
				:: [ptr] "r" (ptr)
				: "memory");

			// bit 0 = Power state
			//     0 = C0.2 (Larger power savings, slower wakeup)
			//     1 = C0.1 (Faster wakeup, small power savings)
			// bits [31:1] = reserved

			// Request C0.1 for faster wakeup.
			uint32_t power_state = 1;

			// Have no way to know if our timeout is larger than IA32_UMWAIT_CONTROL.
			// Although we don't really care if we overrun the os deadline, we'll just loop again.
			// umwait behaviour is slightly different than mwaitx behaviour with timeout.
			// umwait waits until absolute TSC timestamp has elapsed instead of relative cycles.
			uint32_t timeout_lower = cycles_end;
			uint32_t timeout_upper = cycles_end >> 32;

			// umwait writes to CF if the the instruction timed out due to OS time limit.
			// It does not write CF if it timed out due to provided timeout.
			__asm volatile(
				"umwait %[power_state]; # eax, edx"
			:
			: "a" (timeout_lower)
			, "d" (timeout_upper)
			, [power_state] "r" (power_state)
			: "memory", "cc");

			last_cycle_counter = read_cycle_counter();
			if (last_cycle_counter >= cycles_end) {
				return false;
			}
		}
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}

	return true;
}

#endif
