#include "detect.h"
#include "implementations.h"
#include "implementation_details_arm.h"

#if defined(_M_ARM_64) || defined(_M_ARM_32)
#define LOADEXCLUSIVE(LoadExclusiveOp, RegSize) \
	/* Prime the exclusive monitor with the passed in address. */ \
  #LoadExclusiveOp  " %" #RegSize "[Result], [%[Futex]];\n"

#define SPINLOOP_WFE_BODY(LoadAtomicOp, RegSize) \
	/* WFE will wait for either the memory to change or spurious wake-up. */ \
	"wfe;\n" \
	/* Load with acquire to get the result of memory. */ \
	#LoadAtomicOp  " %" #RegSize "[Result], [%[Futex]];\n"

#if defined(_M_ARM_64)
#define SPINLOOP_WFE_LDX_8BIT  LOADEXCLUSIVE(ldaxrb, w)
#define SPINLOOP_WFE_LDX_16BIT LOADEXCLUSIVE(ldaxrh, w)
#define SPINLOOP_WFE_LDX_32BIT LOADEXCLUSIVE(ldaxr,  w)
#define SPINLOOP_WFE_LDX_64BIT LOADEXCLUSIVE(ldaxr,  x)

#define SPINLOOP_WFE_8BIT  SPINLOOP_WFE_BODY(ldarb, w)
#define SPINLOOP_WFE_16BIT SPINLOOP_WFE_BODY(ldarh, w)
#define SPINLOOP_WFE_32BIT SPINLOOP_WFE_BODY(ldar,  w)
#define SPINLOOP_WFE_64BIT SPINLOOP_WFE_BODY(ldar,  x)

#define SPINLOOP_WFET_BODY(LoadAtomicOp, RegSize) \
	/* WFET will wait for either the memory to change or spurious wake-up or timeout. */ \
	/* Hardcoded `wfet x2`. Required to bypass compile-time checks */ \
	".word 0b11010101000000110001000000000010;\n" \
	/* Load with acquire to get the result of memory. */ \
	#LoadAtomicOp  " %" #RegSize "[Result], [%[Futex]];\n"

#define SPINLOOP_WFET_8BIT  SPINLOOP_WFET_BODY(ldarb, w)
#define SPINLOOP_WFET_16BIT SPINLOOP_WFET_BODY(ldarh, w)
#define SPINLOOP_WFET_32BIT SPINLOOP_WFET_BODY(ldar,  w)
#define SPINLOOP_WFET_64BIT SPINLOOP_WFET_BODY(ldar,  x)
#else

#define SPINLOOP_WFE_LDX_8BIT  LOADEXCLUSIVE(ldaexb, )
#define SPINLOOP_WFE_LDX_16BIT LOADEXCLUSIVE(ldaexh, )
#define SPINLOOP_WFE_LDX_32BIT LOADEXCLUSIVE(ldaex,  )
#define SPINLOOP_WFE_LDX_64BIT LOADEXCLUSIVE(ldaex,  )

#define SPINLOOP_WFE_8BIT  SPINLOOP_WFE_BODY(ldab, )
#define SPINLOOP_WFE_16BIT SPINLOOP_WFE_BODY(ldah, )
#define SPINLOOP_WFE_32BIT SPINLOOP_WFE_BODY(lda,  )
#endif

void wfe_wait_for_value_i8 (uint8_t *ptr,  uint8_t value, bool low_power) {
	uint8_t tmp;
	uint8_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return;

	do {
		__asm volatile(SPINLOOP_WFE_LDX_8BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");
		if (result == value) return;

		__asm volatile(SPINLOOP_WFE_8BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			:: "memory");
	} while (result != value);
}

void wfe_wait_for_value_i16(uint16_t *ptr, uint16_t value, bool low_power) {
	uint16_t tmp;
	uint16_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return;

	do {
		__asm volatile(SPINLOOP_WFE_LDX_16BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");
		if (result == value) return;

		__asm volatile(SPINLOOP_WFE_16BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			:: "memory");
	} while (result != value);
}

void wfe_wait_for_value_i32(uint32_t *ptr, uint32_t value, bool low_power) {
	uint32_t tmp;
	uint32_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return;

	do {
		__asm volatile(SPINLOOP_WFE_LDX_32BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");
		if (result == value) return;

		__asm volatile(SPINLOOP_WFE_32BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			:: "memory");
	} while (result != value);
}

#if defined(_M_ARM_64)
void wfe_wait_for_value_i64(uint64_t *ptr, uint64_t value, bool low_power) {
	uint64_t tmp;
	uint64_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return;

	do {
		__asm volatile(SPINLOOP_WFE_LDX_64BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");
		if (result == value) return;

		__asm volatile(SPINLOOP_WFE_64BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			:: "memory");
	} while (result != value);
}
#endif

uint8_t wfe_wait_for_bit_set_i8 (uint8_t *ptr,  uint8_t bit, bool low_power) {
	uint8_t tmp;
	uint8_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (((result >> bit) & 1) == 1) return result;

	do {
		__asm volatile(SPINLOOP_WFE_LDX_8BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");
		if (((result >> bit) & 1) == 1) return result;

		__asm volatile(SPINLOOP_WFE_8BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			:: "memory");
	} while (((result >> bit) & 1) == 0);

	return result;
}

uint16_t wfe_wait_for_bit_set_i16(uint16_t *ptr, uint8_t bit, bool low_power) {
	uint16_t tmp;
	uint16_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (((result >> bit) & 1) == 1) return result;

	do {
		__asm volatile(SPINLOOP_WFE_LDX_16BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");
		if (((result >> bit) & 1) == 1) return result;

		__asm volatile(SPINLOOP_WFE_16BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			:: "memory");
	} while (((result >> bit) & 1) == 0);

	return result;
}

uint32_t wfe_wait_for_bit_set_i32(uint32_t *ptr, uint8_t bit, bool low_power) {
	uint32_t tmp;
	uint32_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (((result >> bit) & 1) == 1) return result;

	do {
		__asm volatile(SPINLOOP_WFE_LDX_32BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");
		if (((result >> bit) & 1) == 1) return result;

		__asm volatile(SPINLOOP_WFE_32BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			:: "memory");
	} while (((result >> bit) & 1) == 0);

	return result;
}

#if defined(_M_ARM_64)
uint64_t wfe_wait_for_bit_set_i64(uint64_t *ptr, uint8_t bit, bool low_power) {
	uint64_t tmp;
	uint64_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (((result >> bit) & 1) == 1) return result;

	do {
		__asm volatile(SPINLOOP_WFE_LDX_64BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");
		if (((result >> bit) & 1) == 1) return result;

		__asm volatile(SPINLOOP_WFE_64BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			:: "memory");
	} while (((result >> bit) & 1) == 0);

	return result;
}
#endif

uint8_t wfe_wait_for_bit_not_set_i8 (uint8_t *ptr,  uint8_t bit, bool low_power) {
	uint8_t tmp;
	uint8_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (((result >> bit) & 1) == 0) return result;

	do {
		__asm volatile(SPINLOOP_WFE_LDX_8BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");
		if (((result >> bit) & 1) == 0) return result;

		__asm volatile(SPINLOOP_WFE_8BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			:: "memory");
	} while (((result >> bit) & 1) == 1);

	return result;
}

uint16_t wfe_wait_for_bit_not_set_i16(uint16_t *ptr, uint8_t bit, bool low_power) {
	uint16_t tmp;
	uint16_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (((result >> bit) & 1) == 0) return result;

	do {
		__asm volatile(SPINLOOP_WFE_LDX_16BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");
		if (((result >> bit) & 1) == 0) return result;

		__asm volatile(SPINLOOP_WFE_16BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			:: "memory");
	} while (((result >> bit) & 1) == 1);

	return result;
}

uint32_t wfe_wait_for_bit_not_set_i32(uint32_t *ptr, uint8_t bit, bool low_power) {
	uint32_t tmp;
	uint32_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (((result >> bit) & 1) == 0) return result;

	do {
		__asm volatile(SPINLOOP_WFE_LDX_32BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");
		if (((result >> bit) & 1) == 0) return result;

		__asm volatile(SPINLOOP_WFE_32BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			:: "memory");
	} while (((result >> bit) & 1) == 1);

	return result;
}

#if defined(_M_ARM_64)
uint64_t wfe_wait_for_bit_not_set_i64(uint64_t *ptr, uint8_t bit, bool low_power) {
	uint64_t tmp;
	uint64_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (((result >> bit) & 1) == 0) return result;

	do {
		__asm volatile(SPINLOOP_WFE_LDX_64BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");
		if (((result >> bit) & 1) == 0) return result;

		__asm volatile(SPINLOOP_WFE_64BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			:: "memory");
	} while (((result >> bit) & 1) == 1);

	return result;
}
#endif

bool wfe_wait_for_value_timeout_i8 (uint8_t *ptr,  uint8_t value, uint64_t nanoseconds, bool low_power) {
	uint8_t tmp;
	uint8_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	do {
		__asm volatile(SPINLOOP_WFE_LDX_8BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");
		if (result == value) return true;

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
		__asm volatile(SPINLOOP_WFE_LDX_16BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");
		if (result == value) return true;

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
		__asm volatile(SPINLOOP_WFE_LDX_32BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");
		if (result == value) return true;

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

#if defined(_M_ARM_64)
bool wfe_wait_for_value_timeout_i64(uint64_t *ptr, uint64_t value, uint64_t nanoseconds, bool low_power) {
	uint64_t tmp;
	uint64_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return true;

	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t cycles_end = begin_cycles + total_cycles;

	do {
		__asm volatile(SPINLOOP_WFE_LDX_64BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");
		if (result == value) return true;

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
	register const uint64_t cycles_end asm("r2") = begin_cycles + total_cycles;

	do {
		__asm volatile(SPINLOOP_WFE_LDX_8BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");
		if (result == value) return true;

		__asm volatile(SPINLOOP_WFET_8BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			: [WaitCycles] "r" (cycles_end)
			: "memory");

		if (read_cycle_counter() >= cycles_end) {
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
	register const uint64_t cycles_end asm("r2") = begin_cycles + total_cycles;

	do {
		__asm volatile(SPINLOOP_WFE_LDX_16BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");
		if (result == value) return true;

		__asm volatile(SPINLOOP_WFET_16BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			: [WaitCycles] "r" (cycles_end)
			: "memory");

		if (read_cycle_counter() >= cycles_end) {
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

	const uint64_t begin_cycles = read_cycle_counter();
	const uint64_t total_cycles = wfe_mutex_detect_calculate_cycles_for_nanoseconds(nanoseconds);
	register const uint64_t cycles_end asm("r2") = begin_cycles + total_cycles;

	do {
		__asm volatile(SPINLOOP_WFE_LDX_32BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");

		if (result == value) return true;

		__asm volatile(SPINLOOP_WFET_32BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			: [WaitCycles] "r" (cycles_end)
			: "memory");

		if (read_cycle_counter() >= cycles_end) {
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
	register const uint64_t cycles_end asm("r2") = begin_cycles + total_cycles;

	do {
		__asm volatile(SPINLOOP_WFE_LDX_64BIT
			: [Result] "=r" (result)
			, [Futex] "+r" (ptr)
			:: "memory");
		if (result == value) return true;

		__asm volatile(SPINLOOP_WFET_64BIT
			: [Result] "=r" (result)
			, [Tmp] "=r" (tmp)
			, [Futex] "+r" (ptr)
			: [WaitCycles] "r" (cycles_end)
			: "memory");

		if (read_cycle_counter() >= cycles_end) {
			return false;
		}
	} while (result != value);

	return true;
}
#endif

bool wfe_wait_for_value_spurious_oneshot_i8 (uint8_t *ptr,  uint8_t value, bool low_power) {
	uint8_t tmp;
	uint8_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return true;

	__asm volatile(SPINLOOP_WFE_LDX_8BIT
		: [Result] "=r" (result)
		, [Futex] "+r" (ptr)
		:: "memory");
	if (result == value) return true;

	__asm volatile(SPINLOOP_WFE_8BIT
		: [Result] "=r" (result)
		, [Tmp] "=r" (tmp)
		, [Futex] "+r" (ptr)
		:: "memory");

	return result == value;
}

bool wfe_wait_for_value_spurious_oneshot_i16(uint16_t *ptr, uint16_t value, bool low_power) {
	uint16_t tmp;
	uint16_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return true;

	__asm volatile(SPINLOOP_WFE_LDX_16BIT
		: [Result] "=r" (result)
		, [Futex] "+r" (ptr)
		:: "memory");
	if (result == value) return true;

	__asm volatile(SPINLOOP_WFE_16BIT
		: [Result] "=r" (result)
		, [Tmp] "=r" (tmp)
		, [Futex] "+r" (ptr)
		:: "memory");

	return result == value;
}

bool wfe_wait_for_value_spurious_oneshot_i32(uint32_t *ptr, uint32_t value, bool low_power) {
	uint32_t tmp;
	uint32_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return true;

	__asm volatile(SPINLOOP_WFE_LDX_32BIT
		: [Result] "=r" (result)
		, [Futex] "+r" (ptr)
		:: "memory");
	if (result == value) return true;

	__asm volatile(SPINLOOP_WFE_32BIT
		: [Result] "=r" (result)
		, [Tmp] "=r" (tmp)
		, [Futex] "+r" (ptr)
		:: "memory");

	return result == value;
}

#if defined(_M_ARM_64)
bool wfe_wait_for_value_spurious_oneshot_i64(uint64_t *ptr, uint64_t value, bool low_power) {
	uint64_t tmp;
	uint64_t result = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);

	// Early return if the value is already set.
	if (result == value) return true;

	__asm volatile(SPINLOOP_WFE_LDX_64BIT
		: [Result] "=r" (result)
		, [Futex] "+r" (ptr)
		:: "memory");
	if (result == value) return true;

	__asm volatile(SPINLOOP_WFE_64BIT
		: [Result] "=r" (result)
		, [Tmp] "=r" (tmp)
		, [Futex] "+r" (ptr)
		:: "memory");

	return result == value;
}
#endif
#endif
