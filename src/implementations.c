#include "detect.h"
#include "implementations.h"

#include <stdio.h>

#ifdef _M_ARM_64
uint64_t read_cycle_counter() {
	uint64_t result;
	__asm volatile(
		"isb;"
		"mrs %[Res], CNTVCT_EL0;"
		: [Res] "=r" (result));
	return result;
}

#define SPINLOOP_WFE_BODY(LoadExclusiveOp, LoadAtomicOp, RegSize) \
	/* Prime the exclusive monitor with the passed in address. */ \
  #LoadExclusiveOp  " %" #RegSize "[Tmp], [%[Futex]];" \
	/* WFE will wait for either the memory to change or spurious wake-up. */ \
	"wfe;" \
	/* Load with acquire to get the result of memory. */ \
	#LoadAtomicOp  " %" #RegSize "[Result], [%[Futex]];"
#define SPINLOOP_WFE_8BIT  SPINLOOP_WFE_BODY(ldaxrb, ldarb, w)
#define SPINLOOP_WFE_16BIT SPINLOOP_WFE_BODY(ldaxrh, ldarh, w)
#define SPINLOOP_WFE_32BIT SPINLOOP_WFE_BODY(ldaxr,  ldar,  w)
#define SPINLOOP_WFE_64BIT SPINLOOP_WFE_BODY(ldaxr,  ldar,  x)

#define SPINLOOP_WFET_BODY(LoadExclusiveOp, LoadAtomicOp, RegSize) \
	/* Prime the exclusive monitor with the passed in address. */ \
  #LoadExclusiveOp  " %" #RegSize "[Tmp], [%[Futex]];" \
	/* WFET will wait for either the memory to change or spurious wake-up or timeout. */ \
	"wfet %[WaitCycles];" \
	/* Load with acquire to get the result of memory. */ \
	#LoadAtomicOp  " %" #RegSize "[Result], [%[Futex]];"
#define SPINLOOP_WFET_8BIT  SPINLOOP_WFE_BODY(ldaxrb, ldarb, w)
#define SPINLOOP_WFET_16BIT SPINLOOP_WFE_BODY(ldaxrh, ldarh, w)
#define SPINLOOP_WFET_32BIT SPINLOOP_WFE_BODY(ldaxr,  ldar,  w)
#define SPINLOOP_WFET_64BIT SPINLOOP_WFE_BODY(ldaxr,  ldar,  x)

void wfe_wait_for_value_i8 (uint8_t *ptr,  uint8_t value, bool low_power) {
	uint8_t tmp;
	uint8_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return;

	do {
		__asm volatile(SPINLOOP_WFE_8BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			: [Futex] "r" (ptr)
			: "memory");
	} while (result != value);
}

void wfe_wait_for_value_i16(uint16_t *ptr, uint16_t value, bool low_power) {
	uint16_t tmp;
	uint16_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return;

	do {
		__asm volatile(SPINLOOP_WFE_16BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			: [Futex] "r" (ptr)
			: "memory");
	} while (result != value);
}

void wfe_wait_for_value_i32(uint32_t *ptr, uint32_t value, bool low_power) {
	uint32_t tmp;
	uint32_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return;

	do {
		__asm volatile(SPINLOOP_WFE_32BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			: [Futex] "r" (ptr)
			: "memory");
	} while (result != value);
}

void wfe_wait_for_value_i64(uint64_t *ptr, uint64_t value, bool low_power) {
	uint64_t tmp;
	uint64_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return;

	do {
		__asm volatile(SPINLOOP_WFE_64BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			: [Futex] "r" (ptr)
			: "memory");
	} while (result != value);
}

bool wfe_wait_for_value_timeout_i8 (uint8_t *ptr,  uint8_t value, uint64_t nanoseconds, bool low_power) {
	uint8_t tmp;
	uint8_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	do {
		__asm volatile(SPINLOOP_WFE_8BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			:: "memory");

		if (read_cycle_counter() >= cycles_end) {
			return false;
		}
	} while (result != value);

	return true;
}

bool wfe_wait_for_value_timeout_i16(uint16_t *ptr, uint16_t value, uint64_t nanoseconds, bool low_power) {
	uint16_t tmp;
	uint16_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	do {
		__asm volatile(SPINLOOP_WFE_16BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			:: "memory");

		if (read_cycle_counter() >= cycles_end) {
			return false;
		}
	} while (result != value);

	return true;
}

bool wfe_wait_for_value_timeout_i32(uint32_t *ptr, uint32_t value, uint64_t nanoseconds, bool low_power) {
	uint32_t tmp;
	uint32_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	do {
		__asm volatile(SPINLOOP_WFE_32BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			:: "memory");

		if (read_cycle_counter() >= cycles_end) {
			return false;
		}
	} while (result != value);

	return true;
}

bool wfe_wait_for_value_timeout_i64(uint64_t *ptr, uint64_t value, uint64_t nanoseconds, bool low_power) {
	uint64_t tmp;
	uint64_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	do {
		__asm volatile(SPINLOOP_WFE_64BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			:: "memory");

		if (read_cycle_counter() >= cycles_end) {
			return false;
		}
	} while (result != value);

	return true;
}

bool wfet_wait_for_value_timeout_i8 (uint8_t *ptr,  uint8_t value, uint64_t nanoseconds, bool low_power) {
	uint8_t tmp;
	uint8_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	uint64_t last_cycle_counter = begin_cycles;

	do {
		const uint64_t cycles_remaining = cycles_end - last_cycle_counter;
		__asm volatile(SPINLOOP_WFET_8BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			: [WaitCycles] "r" (cycles_remaining)
			: "memory");

		last_cycle_counter = read_cycle_counter();
		if (last_cycle_counter >= cycles_end) {
			return false;
		}
	} while (result != value);

	return true;
}

bool wfet_wait_for_value_timeout_i16(uint16_t *ptr, uint16_t value, uint64_t nanoseconds, bool low_power) {
	uint16_t tmp;
	uint16_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	uint64_t last_cycle_counter = begin_cycles;

	do {
		const uint64_t cycles_remaining = cycles_end - last_cycle_counter;
		__asm volatile(SPINLOOP_WFET_8BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			: [WaitCycles] "r" (cycles_remaining)
			: "memory");

		last_cycle_counter = read_cycle_counter();
		if (last_cycle_counter >= cycles_end) {
			return false;
		}
	} while (result != value);

	return true;
}

bool wfet_wait_for_value_timeout_i32(uint32_t *ptr, uint32_t value, uint64_t nanoseconds, bool low_power) {
	uint32_t tmp;
	uint32_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	uint64_t last_cycle_counter = begin_cycles;

	do {
		const uint64_t cycles_remaining = cycles_end - last_cycle_counter;
		__asm volatile(SPINLOOP_WFET_8BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			: [WaitCycles] "r" (cycles_remaining)
			: "memory");

		last_cycle_counter = read_cycle_counter();
		if (last_cycle_counter >= cycles_end) {
			return false;
		}
	} while (result != value);

	return true;
}

bool wfet_wait_for_value_timeout_i64(uint64_t *ptr, uint64_t value, uint64_t nanoseconds, bool low_power) {
	uint64_t tmp;
	uint64_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	uint64_t last_cycle_counter = begin_cycles;

	do {
		const uint64_t cycles_remaining = cycles_end - last_cycle_counter;
		__asm volatile(SPINLOOP_WFET_8BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			: [WaitCycles] "r" (cycles_remaining)
			: "memory");

		last_cycle_counter = read_cycle_counter();
		if (last_cycle_counter >= cycles_end) {
			return false;
		}
	} while (result != value);

	return true;
}

#elif defined(_M_X86_64)
uint64_t read_cycle_counter() {
	return __rdtsc();
}

void mwaitx_wait_for_value_i8 (uint8_t *ptr,  uint8_t value, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return;

	if (low_power) {
		do {
			uint32_t extension = 0;
			uint32_t hints = 0;

			__asm volatile (
				"monitorx; # eax, ecx, edx"
				:: "a" (ptr)
				, "c" (extension)
				, "d" (hints)
				: "memory");

			// bit [7:4] + 1 = cstate request.
			// Request C1 for low power.
			uint32_t waitx_hints = 0;
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
	else {
		do {
			uint32_t extension = 0;
			uint32_t hints = 0;

			__asm volatile (
				"monitorx; # eax, ecx, edx"
				:: "a" (ptr)
				, "c" (extension)
				, "d" (hints)
				: "memory");

			// bit [7:4] + 1 = cstate request.
			// Request C0 to wake up faster
			uint32_t waitx_hints = (0xF << 4);
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
}

void mwaitx_wait_for_value_i16(uint16_t *ptr, uint16_t value, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return;

	if (low_power) {
		do {
			uint32_t extension = 0;
			uint32_t hints = 0;

			__asm volatile (
				"monitorx; # eax, ecx, edx"
				:: "a" (ptr)
				, "c" (extension)
				, "d" (hints)
				: "memory");

			// bit [7:4] + 1 = cstate request.
			// Request C1 for low power.
			uint32_t waitx_hints = 0;
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
	else {
		do {
			uint32_t extension = 0;
			uint32_t hints = 0;

			__asm volatile (
				"monitorx; # eax, ecx, edx"
				:: "a" (ptr)
				, "c" (extension)
				, "d" (hints)
				: "memory");

			// bit [7:4] + 1 = cstate request.
			// Request C0 to wake up faster
			uint32_t waitx_hints = (0xF << 4);
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
}

void mwaitx_wait_for_value_i32(uint32_t *ptr, uint32_t value, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return;

	if (low_power) {
		do {
			uint32_t extension = 0;
			uint32_t hints = 0;

			__asm volatile (
				"monitorx; # eax, ecx, edx"
				:: "a" (ptr)
				, "c" (extension)
				, "d" (hints)
				: "memory");

			// bit [7:4] + 1 = cstate request.
			// Request C1 for low power.
			uint32_t waitx_hints = 0;
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
	else {
		do {
			uint32_t extension = 0;
			uint32_t hints = 0;

			__asm volatile (
				"monitorx; # eax, ecx, edx"
				:: "a" (ptr)
				, "c" (extension)
				, "d" (hints)
				: "memory");

			// bit [7:4] + 1 = cstate request.
			// Request C0 to wake up faster
			uint32_t waitx_hints = (0xF << 4);
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
}

void mwaitx_wait_for_value_i64(uint64_t *ptr, uint64_t value, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return;

	if (low_power) {
		do {
			uint32_t extension = 0;
			uint32_t hints = 0;

			__asm volatile (
				"monitorx; # eax, ecx, edx"
				:: "a" (ptr)
				, "c" (extension)
				, "d" (hints)
				: "memory");

			// bit [7:4] + 1 = cstate request.
			// Request C1 for low power.
			uint32_t waitx_hints = 0;
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
	else {
		do {
			uint32_t extension = 0;
			uint32_t hints = 0;

			__asm volatile (
				"monitorx; # eax, ecx, edx"
				:: "a" (ptr)
				, "c" (extension)
				, "d" (hints)
				: "memory");

			// bit [7:4] + 1 = cstate request.
			// Request C0 to wake up faster
			uint32_t waitx_hints = (0xF << 4);
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
}

bool mwaitx_wait_for_value_timeout_i8 (uint8_t *ptr,  uint8_t value, uint64_t nanoseconds, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	uint64_t last_cycle_counter = begin_cycles;

	if (low_power) {
		do {
			uint32_t extension = 0;
			uint32_t hints = 0;

			__asm volatile (
				"monitorx; # eax, ecx, edx"
				:: "a" (ptr)
				, "c" (extension)
				, "d" (hints)
				: "memory");

			// bit [7:4] + 1 = cstate request.
			// Request C1 for low power.
			uint32_t waitx_hints = 0;
			// bit 0 = allow interrupts to wake.
			// bit 1 = ebx contains timeout.
			uint32_t waitx_extensions = (1U << 1);

			const uint64_t cycles_remaining = cycles_end - last_cycle_counter;

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
	}
	else {
		do {
			uint32_t extension = 0;
			uint32_t hints = 0;

			const uint64_t cycles_remaining = cycles_end - last_cycle_counter;

			__asm volatile (
				"monitorx; # eax, ecx, edx"
				:: "a" (ptr)
				, "c" (extension)
				, "d" (hints)
				: "memory");

			// bit [7:4] + 1 = cstate request.
			// Request C0 to wake up faster
			uint32_t waitx_hints = (0xF << 4);
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
	}

	return true;
}

bool mwaitx_wait_for_value_timeout_i16(uint16_t *ptr, uint16_t value, uint64_t nanoseconds, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	uint64_t last_cycle_counter = begin_cycles;

	if (low_power) {
		do {
			uint32_t extension = 0;
			uint32_t hints = 0;

			__asm volatile (
				"monitorx; # eax, ecx, edx"
				:: "a" (ptr)
				, "c" (extension)
				, "d" (hints)
				: "memory");

			// bit [7:4] + 1 = cstate request.
			// Request C1 for low power.
			uint32_t waitx_hints = 0;
			// bit 0 = allow interrupts to wake.
			// bit 1 = ebx contains timeout.
			uint32_t waitx_extensions = (1U << 1);

			const uint64_t cycles_remaining = cycles_end - last_cycle_counter;

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
	}
	else {
		do {
			uint32_t extension = 0;
			uint32_t hints = 0;

			__asm volatile (
				"monitorx; # eax, ecx, edx"
				:: "a" (ptr)
				, "c" (extension)
				, "d" (hints)
				: "memory");

			// bit [7:4] + 1 = cstate request.
			// Request C0 to wake up faster
			uint32_t waitx_hints = (0xF << 4);
			// bit 0 = allow interrupts to wake.
			// bit 1 = ebx contains timeout.
			uint32_t waitx_extensions = (1U << 1);

			const uint64_t cycles_remaining = cycles_end - last_cycle_counter;

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
	}

	return true;
}

bool mwaitx_wait_for_value_timeout_i32(uint32_t *ptr, uint32_t value, uint64_t nanoseconds, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	uint64_t last_cycle_counter = begin_cycles;

	if (low_power) {
		do {
			uint32_t extension = 0;
			uint32_t hints = 0;

			__asm volatile (
				"monitorx; # eax, ecx, edx"
				:: "a" (ptr)
				, "c" (extension)
				, "d" (hints)
				: "memory");

			// bit [7:4] + 1 = cstate request.
			// Request C1 for low power.
			uint32_t waitx_hints = 0;
			// bit 0 = allow interrupts to wake.
			// bit 1 = ebx contains timeout.
			uint32_t waitx_extensions = (1U << 1);

			const uint64_t cycles_remaining = cycles_end - last_cycle_counter;

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
	}
	else {
		do {
			uint32_t extension = 0;
			uint32_t hints = 0;

			__asm volatile (
				"monitorx; # eax, ecx, edx"
				:: "a" (ptr)
				, "c" (extension)
				, "d" (hints)
				: "memory");

			// bit [7:4] + 1 = cstate request.
			// Request C0 to wake up faster
			uint32_t waitx_hints = (0xF << 4);
			// bit 0 = allow interrupts to wake.
			// bit 1 = ebx contains timeout.
			uint32_t waitx_extensions = (1U << 1);

			const uint64_t cycles_remaining = cycles_end - last_cycle_counter;

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
	}

	return true;
}

bool mwaitx_wait_for_value_timeout_i64(uint64_t *ptr, uint64_t value, uint64_t nanoseconds, bool low_power) {
	// Early return if the value is already set.
	if (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	uint64_t last_cycle_counter = begin_cycles;

	if (low_power) {
		do {
			uint32_t extension = 0;
			uint32_t hints = 0;

			__asm volatile (
				"monitorx; # eax, ecx, edx"
				:: "a" (ptr)
				, "c" (extension)
				, "d" (hints)
				: "memory");

			// bit [7:4] + 1 = cstate request.
			// Request C1 for low power.
			uint32_t waitx_hints = 0;
			// bit 0 = allow interrupts to wake.
			// bit 1 = ebx contains timeout.
			uint32_t waitx_extensions = (1U << 1);

			const uint64_t cycles_remaining = cycles_end - last_cycle_counter;

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
	}
	else {
		do {
			uint32_t extension = 0;
			uint32_t hints = 0;

			__asm volatile (
				"monitorx; # eax, ecx, edx"
				:: "a" (ptr)
				, "c" (extension)
				, "d" (hints)
				: "memory");

			// bit [7:4] + 1 = cstate request.
			// Request C0 to wake up faster
			uint32_t waitx_hints = (0xF << 4);
			// bit 0 = allow interrupts to wake.
			// bit 1 = ebx contains timeout.
			uint32_t waitx_extensions = (1U << 1);

			const uint64_t cycles_remaining = cycles_end - last_cycle_counter;

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
	}

	return true;
}

#endif

void spinloop_wait_for_value_i8 (uint8_t *ptr,  uint8_t value, bool low_power) {
	while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
}

void spinloop_wait_for_value_i16(uint16_t *ptr, uint16_t value, bool low_power) {
	while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
}

void spinloop_wait_for_value_i32(uint32_t *ptr, uint32_t value, bool low_power) {
	while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
}

void spinloop_wait_for_value_i64(uint64_t *ptr, uint64_t value, bool low_power) {
	while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value);
}

bool spinloop_wait_for_value_timeout_i8 (uint8_t *ptr,  uint8_t value, uint64_t nanoseconds, bool low_power) {
	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value) {
		if (read_cycle_counter() >= cycles_end) {
			return false;
		}
	}

	return true;
}

bool spinloop_wait_for_value_timeout_i16(uint16_t *ptr, uint16_t value, uint64_t nanoseconds, bool low_power) {
	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value) {
		if (read_cycle_counter() >= cycles_end) {
			return false;
		}
	}

	return true;
}

bool spinloop_wait_for_value_timeout_i32(uint32_t *ptr, uint32_t value, uint64_t nanoseconds, bool low_power) {
	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value) {
		if (read_cycle_counter() >= cycles_end) {
			return false;
		}
	}

	return true;
}

bool spinloop_wait_for_value_timeout_i64(uint64_t *ptr, uint64_t value, uint64_t nanoseconds, bool low_power) {
	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	while (__atomic_load_n(ptr, __ATOMIC_ACQUIRE) != value) {
		if (read_cycle_counter() >= cycles_end) {
			return false;
		}
	}

	return true;
}
