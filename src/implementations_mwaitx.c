#include "detect.h"
#include "implementations.h"

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#if defined(_M_X86_64) || defined(_M_X86_32)
#if defined(_M_X86_64)
static uint64_t read_cycle_counter() {
	return __rdtsc();
}
#elif defined(_M_X86_32)
static uint64_t read_cycle_counter() {
	uint32_t high, low;

	__asm volatile(
	"rdtsc;"
	: "=a" (low)
	, "=d" (high)
	:: "memory");

	uint64_t result = high;
	result <<= 32;
	result |= low;
	return result;
}
#endif

void mwaitx_wait_for_value_i8 (uint8_t *ptr,  uint8_t value, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return;

	do {
		uint32_t extension = 0;
		uint32_t hints = 0;

		__asm volatile (
			"monitorx; # eax, ecx, edx"
			:: "a" (ptr)
			, "c" (extension)
			, "d" (hints)
			: "memory");

		if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return;

		// bit [7:4] + 1 = cstate request.
		// Request C0 to wake up faster
		uint32_t waitx_hints = low_power ? 0 : (0xF << 4);
		// bit 0 = allow interrupts to wake.
		// bit 1 = ebx contains timeout.
		uint32_t waitx_extensions = 0;
		__asm volatile(
			"mwaitx; # eax, ecx"
		:: "a" (waitx_hints)
		, "c" (waitx_extensions)
		: "memory");
	}
	while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
}

void mwaitx_wait_for_value_i16(uint16_t *ptr, uint16_t value, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return;

	do {
		uint32_t extension = 0;
		uint32_t hints = 0;

		__asm volatile (
			"monitorx; # eax, ecx, edx"
			:: "a" (ptr)
			, "c" (extension)
			, "d" (hints)
			: "memory");

		if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return;

		// bit [7:4] + 1 = cstate request.
		// Request C0 to wake up faster
		uint32_t waitx_hints = low_power ? 0 : (0xF << 4);
		// bit 0 = allow interrupts to wake.
		// bit 1 = ebx contains timeout.
		uint32_t waitx_extensions = 0;
		__asm volatile(
			"mwaitx; # eax, ecx"
		:: "a" (waitx_hints)
		, "c" (waitx_extensions)
		: "memory");
	}
	while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
}

void mwaitx_wait_for_value_i32(uint32_t *ptr, uint32_t value, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return;

	do {
		uint32_t extension = 0;
		uint32_t hints = 0;

		__asm volatile (
			"monitorx; # eax, ecx, edx"
			:: "a" (ptr)
			, "c" (extension)
			, "d" (hints)
			: "memory");

		if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return;

		// bit [7:4] + 1 = cstate request.
		// Request C0 to wake up faster
		uint32_t waitx_hints = low_power ? 0 : (0xF << 4);
		// bit 0 = allow interrupts to wake.
		// bit 1 = ebx contains timeout.
		uint32_t waitx_extensions = 0;
		__asm volatile(
			"mwaitx; # eax, ecx"
		:: "a" (waitx_hints)
		, "c" (waitx_extensions)
		: "memory");
	}
	while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
}

void mwaitx_wait_for_value_i64(uint64_t *ptr, uint64_t value, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return;

	do {
		uint32_t extension = 0;
		uint32_t hints = 0;

		__asm volatile (
			"monitorx; # eax, ecx, edx"
			:: "a" (ptr)
			, "c" (extension)
			, "d" (hints)
			: "memory");

		if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return;

		// bit [7:4] + 1 = cstate request.
		// Request C0 to wake up faster
		uint32_t waitx_hints = low_power ? 0 : (0xF << 4);
		// bit 0 = allow interrupts to wake.
		// bit 1 = ebx contains timeout.
		uint32_t waitx_extensions = 0;
		__asm volatile(
			"mwaitx; # eax, ecx"
		:: "a" (waitx_hints)
		, "c" (waitx_extensions)
		: "memory");
	}
	while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
}

uint8_t mwaitx_wait_for_bit_set_i8 (uint8_t *ptr,  uint8_t bit, bool low_power) {
	// Early return if the value is already set.
	uint8_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	if (((result >> bit) & 1) == 1) return result;

	do {
		uint32_t extension = 0;
		uint32_t hints = 0;

		__asm volatile (
			"monitorx; # eax, ecx, edx"
			:: "a" (ptr)
			, "c" (extension)
			, "d" (hints)
			: "memory");

		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		if (((result >> bit) & 1) == 1) return result;

		// bit [7:4] + 1 = cstate request.
		// Request C0 to wake up faster
		uint32_t waitx_hints = low_power ? 0 : (0xF << 4);
		// bit 0 = allow interrupts to wake.
		// bit 1 = ebx contains timeout.
		uint32_t waitx_extensions = 0;
		__asm volatile(
			"mwaitx; # eax, ecx"
		:: "a" (waitx_hints)
		, "c" (waitx_extensions)
		: "memory");

		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	}
	while (((result >> bit) & 1) == 0);
	return result;
}

uint16_t mwaitx_wait_for_bit_set_i16(uint16_t *ptr, uint8_t bit, bool low_power) {
	// Early return if the value is already set.
	uint16_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	if (((result >> bit) & 1) == 1) return result;

	do {
		uint32_t extension = 0;
		uint32_t hints = 0;

		__asm volatile (
			"monitorx; # eax, ecx, edx"
			:: "a" (ptr)
			, "c" (extension)
			, "d" (hints)
			: "memory");

		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		if (((result >> bit) & 1) == 1) return result;

		// bit [7:4] + 1 = cstate request.
		// Request C0 to wake up faster
		uint32_t waitx_hints = low_power ? 0 : (0xF << 4);
		// bit 0 = allow interrupts to wake.
		// bit 1 = ebx contains timeout.
		uint32_t waitx_extensions = 0;
		__asm volatile(
			"mwaitx; # eax, ecx"
		:: "a" (waitx_hints)
		, "c" (waitx_extensions)
		: "memory");

		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	}
	while (((result >> bit) & 1) == 0);
	return result;
}

uint32_t mwaitx_wait_for_bit_set_i32(uint32_t *ptr, uint8_t bit, bool low_power) {
	// Early return if the value is already set.
	uint32_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	if (((result >> bit) & 1) == 1) return result;

	do {
		uint32_t extension = 0;
		uint32_t hints = 0;

		__asm volatile (
			"monitorx; # eax, ecx, edx"
			:: "a" (ptr)
			, "c" (extension)
			, "d" (hints)
			: "memory");

		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		if (((result >> bit) & 1) == 1) return result;

		// bit [7:4] + 1 = cstate request.
		// Request C0 to wake up faster
		uint32_t waitx_hints = low_power ? 0 : (0xF << 4);
		// bit 0 = allow interrupts to wake.
		// bit 1 = ebx contains timeout.
		uint32_t waitx_extensions = 0;
		__asm volatile(
			"mwaitx; # eax, ecx"
		:: "a" (waitx_hints)
		, "c" (waitx_extensions)
		: "memory");

		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	}
	while (((result >> bit) & 1) == 0);
	return result;
}

uint64_t mwaitx_wait_for_bit_set_i64(uint64_t *ptr, uint8_t bit, bool low_power) {
	// Early return if the value is already set.
	uint64_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	if (((result >> bit) & 1) == 1) return result;

	do {
		uint32_t extension = 0;
		uint32_t hints = 0;

		__asm volatile (
			"monitorx; # eax, ecx, edx"
			:: "a" (ptr)
			, "c" (extension)
			, "d" (hints)
			: "memory");

		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		if (((result >> bit) & 1) == 1) return result;

		// bit [7:4] + 1 = cstate request.
		// Request C0 to wake up faster
		uint32_t waitx_hints = low_power ? 0 : (0xF << 4);
		// bit 0 = allow interrupts to wake.
		// bit 1 = ebx contains timeout.
		uint32_t waitx_extensions = 0;
		__asm volatile(
			"mwaitx; # eax, ecx"
		:: "a" (waitx_hints)
		, "c" (waitx_extensions)
		: "memory");

		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	}
	while (((result >> bit) & 1) == 0);
	return result;
}

uint8_t mwaitx_wait_for_bit_not_set_i8 (uint8_t *ptr,  uint8_t bit, bool low_power) {
	// Early return if the value is already not set.
	uint8_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	if (((result >> bit) & 1) == 0) return result;

	do {
		uint32_t extension = 0;
		uint32_t hints = 0;

		__asm volatile (
			"monitorx; # eax, ecx, edx"
			:: "a" (ptr)
			, "c" (extension)
			, "d" (hints)
			: "memory");

		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		if (((result >> bit) & 1) == 0) return result;

		// bit [7:4] + 1 = cstate request.
		// Request C0 to wake up faster
		uint32_t waitx_hints = low_power ? 0 : (0xF << 4);
		// bit 0 = allow interrupts to wake.
		// bit 1 = ebx contains timeout.
		uint32_t waitx_extensions = 0;
		__asm volatile(
			"mwaitx; # eax, ecx"
		:: "a" (waitx_hints)
		, "c" (waitx_extensions)
		: "memory");

		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	}
	while (((result >> bit) & 1) == 1);
	return result;
}

uint16_t mwaitx_wait_for_bit_not_set_i16(uint16_t *ptr, uint8_t bit, bool low_power) {
	// Early return if the value is already not set.
	uint16_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	if (((result >> bit) & 1) == 0) return result;

	do {
		uint32_t extension = 0;
		uint32_t hints = 0;

		__asm volatile (
			"monitorx; # eax, ecx, edx"
			:: "a" (ptr)
			, "c" (extension)
			, "d" (hints)
			: "memory");

		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		if (((result >> bit) & 1) == 0) return result;

		// bit [7:4] + 1 = cstate request.
		// Request C0 to wake up faster
		uint32_t waitx_hints = low_power ? 0 : (0xF << 4);
		// bit 0 = allow interrupts to wake.
		// bit 1 = ebx contains timeout.
		uint32_t waitx_extensions = 0;
		__asm volatile(
			"mwaitx; # eax, ecx"
		:: "a" (waitx_hints)
		, "c" (waitx_extensions)
		: "memory");

		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	}
	while (((result >> bit) & 1) == 1);
	return result;
}

uint32_t mwaitx_wait_for_bit_not_set_i32(uint32_t *ptr, uint8_t bit, bool low_power) {
	// Early return if the value is already not set.
	uint32_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	if (((result >> bit) & 1) == 0) return result;

	do {
		uint32_t extension = 0;
		uint32_t hints = 0;

		__asm volatile (
			"monitorx; # eax, ecx, edx"
			:: "a" (ptr)
			, "c" (extension)
			, "d" (hints)
			: "memory");

		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		if (((result >> bit) & 1) == 0) return result;

		// bit [7:4] + 1 = cstate request.
		// Request C0 to wake up faster
		uint32_t waitx_hints = low_power ? 0 : (0xF << 4);
		// bit 0 = allow interrupts to wake.
		// bit 1 = ebx contains timeout.
		uint32_t waitx_extensions = 0;
		__asm volatile(
			"mwaitx; # eax, ecx"
		:: "a" (waitx_hints)
		, "c" (waitx_extensions)
		: "memory");

		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	}
	while (((result >> bit) & 1) == 1);
	return result;
}

uint64_t mwaitx_wait_for_bit_not_set_i64(uint64_t *ptr, uint8_t bit, bool low_power) {
	// Early return if the value is already not set.
	uint64_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	if (((result >> bit) & 1) == 0) return result;

	do {
		uint32_t extension = 0;
		uint32_t hints = 0;

		__asm volatile (
			"monitorx; # eax, ecx, edx"
			:: "a" (ptr)
			, "c" (extension)
			, "d" (hints)
			: "memory");

		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		if (((result >> bit) & 1) == 0) return result;

		// bit [7:4] + 1 = cstate request.
		// Request C0 to wake up faster
		uint32_t waitx_hints = low_power ? 0 : (0xF << 4);
		// bit 0 = allow interrupts to wake.
		// bit 1 = ebx contains timeout.
		uint32_t waitx_extensions = 0;
		__asm volatile(
			"mwaitx; # eax, ecx"
		:: "a" (waitx_hints)
		, "c" (waitx_extensions)
		: "memory");

		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	}
	while (((result >> bit) & 1) == 1);
	return result;
}

bool mwaitx_wait_for_value_timeout_i8 (uint8_t *ptr,  uint8_t value, uint64_t nanoseconds, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	uint64_t last_cycle_counter = begin_cycles;

	do {
		uint32_t extension = 0;
		uint32_t hints = 0;

		const uint64_t cycles_u64 = cycles_end - last_cycle_counter;
		const size_t cycles_remaining = cycles_u64 >= ULONG_MAX ? ~0UL : cycles_u64;

		__asm volatile (
			"monitorx; # eax, ecx, edx"
			:: "a" (ptr)
			, "c" (extension)
			, "d" (hints)
			: "memory");

		if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return true;

		// bit [7:4] + 1 = cstate request.
		// Request C0 to wake up faster
		uint32_t waitx_hints = low_power ? 0 : (0xF << 4);
		// bit 0 = allow interrupts to wake.
		// bit 1 = ebx contains timeout.
		uint32_t waitx_extensions = (1U << 1);

		__asm volatile(
			"mwaitx; # eax, ecx"
		:: "a" (waitx_hints)
		, "b" (cycles_remaining)
		, "c" (waitx_extensions)
		: "memory");

		last_cycle_counter = read_cycle_counter();
		if (last_cycle_counter >= cycles_end) {
			return false;
		}
	}
	while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);

	return true;
}

bool mwaitx_wait_for_value_timeout_i16(uint16_t *ptr, uint16_t value, uint64_t nanoseconds, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	uint64_t last_cycle_counter = begin_cycles;

	do {
		uint32_t extension = 0;
		uint32_t hints = 0;

		const uint64_t cycles_u64 = cycles_end - last_cycle_counter;
		const size_t cycles_remaining = cycles_u64 >= ULONG_MAX ? ~0UL : cycles_u64;

		__asm volatile (
			"monitorx; # eax, ecx, edx"
			:: "a" (ptr)
			, "c" (extension)
			, "d" (hints)
			: "memory");

		if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return true;

		// bit [7:4] + 1 = cstate request.
		// Request C0 to wake up faster
		uint32_t waitx_hints = low_power ? 0 : (0xF << 4);
		// bit 0 = allow interrupts to wake.
		// bit 1 = ebx contains timeout.
		uint32_t waitx_extensions = (1U << 1);

		__asm volatile(
			"mwaitx; # eax, ecx"
		:: "a" (waitx_hints)
		, "b" (cycles_remaining)
		, "c" (waitx_extensions)
		: "memory");

		last_cycle_counter = read_cycle_counter();
		if (last_cycle_counter >= cycles_end) {
			return false;
		}
	}
	while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);

	return true;
}

bool mwaitx_wait_for_value_timeout_i32(uint32_t *ptr, uint32_t value, uint64_t nanoseconds, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	uint64_t last_cycle_counter = begin_cycles;

	do {
		uint32_t extension = 0;
		uint32_t hints = 0;

		const uint64_t cycles_u64 = cycles_end - last_cycle_counter;
		const size_t cycles_remaining = cycles_u64 >= ULONG_MAX ? ~0UL : cycles_u64;

		__asm volatile (
			"monitorx; # eax, ecx, edx"
			:: "a" (ptr)
			, "c" (extension)
			, "d" (hints)
			: "memory");

		if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return true;

		// bit [7:4] + 1 = cstate request.
		// Request C0 to wake up faster
		uint32_t waitx_hints = low_power ? 0 : (0xF << 4);
		// bit 0 = allow interrupts to wake.
		// bit 1 = ebx contains timeout.
		uint32_t waitx_extensions = (1U << 1);

		__asm volatile(
			"mwaitx; # eax, ecx"
		:: "a" (waitx_hints)
		, "b" (cycles_remaining)
		, "c" (waitx_extensions)
		: "memory");

		last_cycle_counter = read_cycle_counter();
		if (last_cycle_counter >= cycles_end) {
			return false;
		}
	}
	while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);

	return true;
}

bool mwaitx_wait_for_value_timeout_i64(uint64_t *ptr, uint64_t value, uint64_t nanoseconds, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	uint64_t last_cycle_counter = begin_cycles;

	do {
		uint32_t extension = 0;
		uint32_t hints = 0;

		const uint64_t cycles_u64 = cycles_end - last_cycle_counter;
		const size_t cycles_remaining = cycles_u64 >= ULONG_MAX ? ~0UL : cycles_u64;

		__asm volatile (
			"monitorx; # eax, ecx, edx"
			:: "a" (ptr)
			, "c" (extension)
			, "d" (hints)
			: "memory");

		if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return true;

		// bit [7:4] + 1 = cstate request.
		// Request C0 to wake up faster
		uint32_t waitx_hints = low_power ? 0 : (0xF << 4);
		// bit 0 = allow interrupts to wake.
		// bit 1 = ebx contains timeout.
		uint32_t waitx_extensions = (1U << 1);

		__asm volatile(
			"mwaitx; # eax, ecx"
		:: "a" (waitx_hints)
		, "b" (cycles_remaining)
		, "c" (waitx_extensions)
		: "memory");

		last_cycle_counter = read_cycle_counter();
		if (last_cycle_counter >= cycles_end) {
			return false;
		}
	}
	while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);

	return true;
}

#endif

