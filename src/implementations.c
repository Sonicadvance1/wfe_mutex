#include "detect.h"
#include "implementations.h"

#include <stdio.h>

#if !(defined(_M_ARM_64) || defined(_M_ARM_32) || defined(_M_X86_64) || defined(_M_X86_32))
#include <time.h>
#endif

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

static void do_yield() {
	__asm volatile("yield;");
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
static void do_yield() {
	__asm volatile("yield;");
}
#endif
#elif defined(_M_X86_64)
static uint64_t read_cycle_counter() {
	return __rdtsc();
}

static void do_yield() {
	__asm volatile("pause;");
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
static void do_yield() {
	__asm volatile("pause;");
}
#else
static uint64_t read_cycle_counter() {
	// Unsupported platform. Read current nanosections from clock_gettime.
	// Hopefully this is a VDSO call on this unsupported platform to be relatively fast.
	// Returns the number of nanoseconds.
	// `wfe_mutex_detect_calculate_cycles_for_nanoseconds` on an unsupported platform returns nanoseconds unmodified.
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	const uint64_t NanosecondsInSecond = 1000000000ULL;
	return ts.tv_sec * NanosecondsInSecond + ts.tv_nsec;
}

static void do_yield() {
	// Unsupported architecture, can't yield.
}
#endif

void spinloop_wait_for_value_i8 (uint8_t *ptr,  uint8_t value, bool low_power) {
	if (low_power) {
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value) {
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			do_yield();
		}
	}
	else {
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}
}

void spinloop_wait_for_value_i16(uint16_t *ptr, uint16_t value, bool low_power) {
	if (low_power) {
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value) {
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			do_yield();
		}
	}
	else {
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}
}

void spinloop_wait_for_value_i32(uint32_t *ptr, uint32_t value, bool low_power) {
	if (low_power) {
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value) {
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			do_yield();
		}
	}
	else {
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}
}

void spinloop_wait_for_value_i64(uint64_t *ptr, uint64_t value, bool low_power) {
	if (low_power) {
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value) {
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			do_yield();
		}
	}
	else {
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
	}
}

uint8_t spinloop_wait_for_bit_set_i8 (uint8_t *ptr,  uint8_t bit, bool low_power) {
	uint8_t result;
	if (low_power) {
		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		while (((result >> bit) & 1) == 0) {
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		};
	}
	else {
		do {
			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		} while (((result >> bit) & 1) == 0);
	}
	return result;
}

uint16_t spinloop_wait_for_bit_set_i16(uint16_t *ptr, uint8_t bit, bool low_power) {
	uint16_t result;
	if (low_power) {
		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		while (((result >> bit) & 1) == 0) {
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		};
	}
	else {
		do {
			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		} while (((result >> bit) & 1) == 0);
	}
	return result;
}

uint32_t spinloop_wait_for_bit_set_i32(uint32_t *ptr, uint8_t bit, bool low_power) {
	uint32_t result;
	if (low_power) {
		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		while (((result >> bit) & 1) == 0) {
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		};
	}
	else {
		do {
			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		} while (((result >> bit) & 1) == 0);
	}
	return result;
}

uint64_t spinloop_wait_for_bit_set_i64(uint64_t *ptr, uint8_t bit, bool low_power) {
	uint64_t result;
	if (low_power) {
		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		while (((result >> bit) & 1) == 0) {
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		};
	}
	else {
		do {
			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		} while (((result >> bit) & 1) == 0);
	}
	return result;
}

uint8_t spinloop_wait_for_bit_not_set_i8 (uint8_t *ptr,  uint8_t bit, bool low_power) {
	uint8_t result;
	if (low_power) {
		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		while (((result >> bit) & 1) == 1) {
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
	}
	else {
		do {
			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		} while (((result >> bit) & 1) == 1);
	}
	return result;
}

uint16_t spinloop_wait_for_bit_not_set_i16(uint16_t *ptr, uint8_t bit, bool low_power) {
	uint16_t result;
	if (low_power) {
		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		while (((result >> bit) & 1) == 1) {
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
	}
	else {
		do {
			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		} while (((result >> bit) & 1) == 1);
	}
	return result;
}

uint32_t spinloop_wait_for_bit_not_set_i32(uint32_t *ptr, uint8_t bit, bool low_power) {
	uint32_t result;
	if (low_power) {
		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		while (((result >> bit) & 1) == 1) {
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
	}
	else {
		do {
			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		} while (((result >> bit) & 1) == 1);
	}
	return result;
}

uint64_t spinloop_wait_for_bit_not_set_i64(uint64_t *ptr, uint8_t bit, bool low_power) {
	uint64_t result;
	if (low_power) {
		result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		while (((result >> bit) & 1) == 1) {
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		}
	}
	else {
		do {
			result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
		} while (((result >> bit) & 1) == 1);
	}
	return result;
}

bool spinloop_wait_for_value_timeout_i8 (uint8_t *ptr,  uint8_t value, uint64_t nanoseconds, bool low_power) {
	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	if (low_power) {
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value) {
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			if (read_cycle_counter() >= cycles_end) {
				return false;
			}
		}
	}
	else {
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value) {
			if (read_cycle_counter() >= cycles_end) {
				return false;
			}
		}
	}

	return true;
}

bool spinloop_wait_for_value_timeout_i16(uint16_t *ptr, uint16_t value, uint64_t nanoseconds, bool low_power) {
	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	if (low_power) {
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value) {
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			if (read_cycle_counter() >= cycles_end) {
				return false;
			}
		}
	}
	else {
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value) {
			if (read_cycle_counter() >= cycles_end) {
				return false;
			}
		}
	}

	return true;
}

bool spinloop_wait_for_value_timeout_i32(uint32_t *ptr, uint32_t value, uint64_t nanoseconds, bool low_power) {
	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	if (low_power) {
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value) {
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			if (read_cycle_counter() >= cycles_end) {
				return false;
			}
		}
	}
	else {
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value) {
			if (read_cycle_counter() >= cycles_end) {
				return false;
			}
		}
	}

	return true;
}

bool spinloop_wait_for_value_timeout_i64(uint64_t *ptr, uint64_t value, uint64_t nanoseconds, bool low_power) {
	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	if (low_power) {
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value) {
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			do_yield();
			if (read_cycle_counter() >= cycles_end) {
				return false;
			}
		}
	}
	else {
		while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value) {
			if (read_cycle_counter() >= cycles_end) {
				return false;
			}
		}
	}

	return true;
}
