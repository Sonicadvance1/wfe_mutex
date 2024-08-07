#include "detect.h"
#include "implementations.h"
#include "implementation_details_x86.h"

#include <limits>
#include <stdint.h>

#if defined(_M_X86_64) || defined(_M_X86_32)
template<typename T>
static inline void mwaitx_wait_for_value_impl(T *ptr, T value, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return;

	do {
		uint32_t extension = 0;
		uint32_t hints = 0;

		__asm volatile (
			"monitorx; # eax, ecx, edx\n"
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
			"mwaitx; # eax, ecx\n"
		:: "a" (waitx_hints)
		, "c" (waitx_extensions)
		: "memory");
	}
	while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
}

template<uint8_t expected_value, uint8_t not_expected_value, typename T>
T mwaitx_wait_for_bit_impl(T *ptr, uint8_t bit, bool low_power) {
	// Early return if the value is already set.
	T result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	if (((result >> bit) & 1) == expected_value) return result;

	do {
		uint32_t extension = 0;
		uint32_t hints = 0;

		__asm volatile (
			"monitorx; # eax, ecx, edx\n"
			:: "a" (ptr)
			, "c" (extension)
			, "d" (hints)
			: "memory");

		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		if (((result >> bit) & 1) == expected_value) return result;

		// bit [7:4] + 1 = cstate request.
		// Request C0 to wake up faster
		uint32_t waitx_hints = low_power ? 0 : (0xF << 4);
		// bit 0 = allow interrupts to wake.
		// bit 1 = ebx contains timeout.
		uint32_t waitx_extensions = 0;
		__asm volatile(
			"mwaitx; # eax, ecx\n"
		:: "a" (waitx_hints)
		, "c" (waitx_extensions)
		: "memory");

		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	}
	while (((result >> bit) & 1) == not_expected_value);
	return result;
}

template<typename T>
static inline bool mwaitx_wait_for_value_impl(T *ptr, T value, uint64_t nanoseconds, bool low_power) {
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
		const uint32_t cycles_remaining = cycles_u64 >= std::numeric_limits<uint32_t>::max() ? std::numeric_limits<uint32_t>::max() : cycles_u64;

		__asm volatile (
			"monitorx; # eax, ecx, edx\n"
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
			"mwaitx; # eax, ecx\n"
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

template<typename T>
static inline bool mwaitx_wait_for_value_spurious_oneshot_impl(T *ptr, T value, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return true;

	uint32_t extension = 0;
	uint32_t hints = 0;

	__asm volatile (
		"monitorx; # eax, ecx, edx\n"
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
	uint32_t waitx_extensions = (0U << 1);

	__asm volatile(
		"mwaitx; # eax, ecx\n"
	:: "a" (waitx_hints)
	, "c" (waitx_extensions)
	: "memory");

	return __atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value;
}

void mwaitx_wait_for_value_i8 (uint8_t *ptr,  uint8_t value, bool low_power) {
	mwaitx_wait_for_value_impl(ptr, value, low_power);
}

void mwaitx_wait_for_value_i16(uint16_t *ptr, uint16_t value, bool low_power) {
	mwaitx_wait_for_value_impl(ptr, value, low_power);
}

void mwaitx_wait_for_value_i32(uint32_t *ptr, uint32_t value, bool low_power) {
	mwaitx_wait_for_value_impl(ptr, value, low_power);
}

void mwaitx_wait_for_value_i64(uint64_t *ptr, uint64_t value, bool low_power) {
	mwaitx_wait_for_value_impl(ptr, value, low_power);
}

uint8_t mwaitx_wait_for_bit_set_i8 (uint8_t *ptr,  uint8_t bit, bool low_power) {
	return mwaitx_wait_for_bit_impl<1, 0>(ptr, bit, low_power);
}

uint16_t mwaitx_wait_for_bit_set_i16(uint16_t *ptr, uint8_t bit, bool low_power) {
	return mwaitx_wait_for_bit_impl<1, 0>(ptr, bit, low_power);
}

uint32_t mwaitx_wait_for_bit_set_i32(uint32_t *ptr, uint8_t bit, bool low_power) {
	return mwaitx_wait_for_bit_impl<1, 0>(ptr, bit, low_power);
}

uint64_t mwaitx_wait_for_bit_set_i64(uint64_t *ptr, uint8_t bit, bool low_power) {
	return mwaitx_wait_for_bit_impl<1, 0>(ptr, bit, low_power);
}

uint8_t mwaitx_wait_for_bit_not_set_i8 (uint8_t *ptr,  uint8_t bit, bool low_power) {
	return mwaitx_wait_for_bit_impl<0, 1>(ptr, bit, low_power);
}

uint16_t mwaitx_wait_for_bit_not_set_i16(uint16_t *ptr, uint8_t bit, bool low_power) {
	return mwaitx_wait_for_bit_impl<0, 1>(ptr, bit, low_power);
}

uint32_t mwaitx_wait_for_bit_not_set_i32(uint32_t *ptr, uint8_t bit, bool low_power) {
	return mwaitx_wait_for_bit_impl<0, 1>(ptr, bit, low_power);
}

uint64_t mwaitx_wait_for_bit_not_set_i64(uint64_t *ptr, uint8_t bit, bool low_power) {
	return mwaitx_wait_for_bit_impl<0, 1>(ptr, bit, low_power);
}

bool mwaitx_wait_for_value_timeout_i8 (uint8_t *ptr,  uint8_t value, uint64_t nanoseconds, bool low_power) {
	return mwaitx_wait_for_value_impl(ptr, value, nanoseconds, low_power);
}

bool mwaitx_wait_for_value_timeout_i16(uint16_t *ptr, uint16_t value, uint64_t nanoseconds, bool low_power) {
	return mwaitx_wait_for_value_impl(ptr, value, nanoseconds, low_power);
}

bool mwaitx_wait_for_value_timeout_i32(uint32_t *ptr, uint32_t value, uint64_t nanoseconds, bool low_power) {
	return mwaitx_wait_for_value_impl(ptr, value, nanoseconds, low_power);
}

bool mwaitx_wait_for_value_timeout_i64(uint64_t *ptr, uint64_t value, uint64_t nanoseconds, bool low_power) {
	return mwaitx_wait_for_value_impl(ptr, value, nanoseconds, low_power);
}

bool mwaitx_wait_for_value_spurious_oneshot_i8 (uint8_t *ptr,  uint8_t value, bool low_power) {
	return mwaitx_wait_for_value_spurious_oneshot_impl(ptr, value, low_power);
}

bool mwaitx_wait_for_value_spurious_oneshot_i16(uint16_t *ptr, uint16_t value, bool low_power) {
	return mwaitx_wait_for_value_spurious_oneshot_impl(ptr, value, low_power);
}

bool mwaitx_wait_for_value_spurious_oneshot_i32(uint32_t *ptr, uint32_t value, bool low_power) {
	return mwaitx_wait_for_value_spurious_oneshot_impl(ptr, value, low_power);
}

bool mwaitx_wait_for_value_spurious_oneshot_i64(uint64_t *ptr, uint64_t value, bool low_power) {
	return mwaitx_wait_for_value_spurious_oneshot_impl(ptr, value, low_power);
}

#endif

