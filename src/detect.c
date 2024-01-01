#include "detect.h"
#include "implementations.h"

#include <wfe_mutex/wfe_mutex.h>

#include <stdint.h>
#include <time.h>

static wfe_mutex_features Features = {
	.cycle_hz = 0,
	.cycles_per_nanosecond_multiplier = 1,
	.cycles_per_nanosecond_divisor = 1,

	.wait_for_value_i8  = spinloop_wait_for_value_i8,
	.wait_for_value_i16 = spinloop_wait_for_value_i16,
	.wait_for_value_i32 = spinloop_wait_for_value_i32,
	.wait_for_value_i64 = spinloop_wait_for_value_i64,

	.wait_for_value_timeout_i8  = spinloop_wait_for_value_timeout_i8,
	.wait_for_value_timeout_i16 = spinloop_wait_for_value_timeout_i16,
	.wait_for_value_timeout_i32 = spinloop_wait_for_value_timeout_i32,
	.wait_for_value_timeout_i64 = spinloop_wait_for_value_timeout_i64,

	.wait_for_bit_set_i8 = spinloop_wait_for_bit_set_i8,
	.wait_for_bit_set_i16 = spinloop_wait_for_bit_set_i16,
	.wait_for_bit_set_i32 = spinloop_wait_for_bit_set_i32,
	.wait_for_bit_set_i64 = spinloop_wait_for_bit_set_i64,

	.wait_for_bit_not_set_i8 = spinloop_wait_for_bit_not_set_i8,
	.wait_for_bit_not_set_i16 = spinloop_wait_for_bit_not_set_i16,
	.wait_for_bit_not_set_i32 = spinloop_wait_for_bit_not_set_i32,
	.wait_for_bit_not_set_i64 = spinloop_wait_for_bit_not_set_i64,

	.supports_wfe_mutex = false,
	.supports_timed_wfe_mutex = false,
	.supports_low_power_cstate_toggle = false,
};

#ifdef _M_ARM_64
static void detect() {
	Features.wait_for_value_i8  = wfe_wait_for_value_i8;
	Features.wait_for_value_i16 = wfe_wait_for_value_i16;
	Features.wait_for_value_i32 = wfe_wait_for_value_i32;
	Features.wait_for_value_i64 = wfe_wait_for_value_i64;

	Features.wait_for_value_timeout_i8  = wfe_wait_for_value_timeout_i8;
	Features.wait_for_value_timeout_i16 = wfe_wait_for_value_timeout_i16;
	Features.wait_for_value_timeout_i32 = wfe_wait_for_value_timeout_i32;
	Features.wait_for_value_timeout_i64 = wfe_wait_for_value_timeout_i64;

	Features.wait_for_bit_set_i8 = wfe_wait_for_bit_set_i8;
	Features.wait_for_bit_set_i16 = wfe_wait_for_bit_set_i16;
	Features.wait_for_bit_set_i32 = wfe_wait_for_bit_set_i32;
	Features.wait_for_bit_set_i64 = wfe_wait_for_bit_set_i64;

	Features.wait_for_bit_not_set_i8 = wfe_wait_for_bit_not_set_i8;
	Features.wait_for_bit_not_set_i16 = wfe_wait_for_bit_not_set_i16;
	Features.wait_for_bit_not_set_i32 = wfe_wait_for_bit_not_set_i32;
	Features.wait_for_bit_not_set_i64 = wfe_wait_for_bit_not_set_i64;

	// ARMv8 always supports wfe_mutex
	Features.supports_wfe_mutex = true;

	// Need to read AA64ISAR2 to see if WFXT is supported.
	// Linux cpuid emulation allows userspace to read this register directly.
	uint64_t isar2;
	__asm ("mrs %[Res], ID_AA64ISAR2_EL1"
		: [Res] "=r" (isar2));
#define WFXT_OFFSET 0
	if ((isar2 >> WFXT_OFFSET) & 0xF) {
		Features.supports_timed_wfe_mutex = true;

		Features.wait_for_value_timeout_i8  = wfet_wait_for_value_timeout_i8;
		Features.wait_for_value_timeout_i16 = wfet_wait_for_value_timeout_i16;
		Features.wait_for_value_timeout_i32 = wfet_wait_for_value_timeout_i32;
		Features.wait_for_value_timeout_i64 = wfet_wait_for_value_timeout_i64;
	}

	// ARMv8 doesn't support a lower power cstate toggle.
}

void detect_cycle_counter_frequency() {
	__asm ("mrs %[Res], CNTFRQ_EL0"
		: [Res] "=r" (Features.cycle_hz));

	// Calculate cycles per nanosecond
	const uint64_t NanosecondsInSecond = 1000000000ULL;
	if (Features.cycle_hz > NanosecondsInSecond) {
		// Cycle counter frequency is greater than 1Ghz. Claim 1:1
		// Assume that a 10,000 cycle scale is close enough to not matter.
		// TODO: Make this more accurate?
		const uint64_t NanosecondsInSecond = 1000000000ULL;
		Features.cycles_per_nanosecond_multiplier = 10000;
		Features.cycles_per_nanosecond_divisor = (NanosecondsInSecond * 10000) / Features.cycle_hz;
	}
	else {
		// Cycle counter frequency is less than 1Ghz. Just divide nanoseconds by frequency.
		// Snapdragon devices historically use a 19.2Mhz cycle counter, which gives around 52.08 cycles per nanosecond.
		// Apple M1 uses a 24Mhz cycle counter which gives around 41.6666... cycles per nanosecond.
		// NVIDIA Tegra goes up to 31.25Mhz which gives 32 cycles per nanosecond, a clean divide.
		Features.cycles_per_nanosecond_multiplier = 1;
		Features.cycles_per_nanosecond_divisor = NanosecondsInSecond / Features.cycle_hz;
	}
}

#elif defined(_M_X86_64) || defined(_M_X86_32)
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

#include <cpuid.h>

static void detect() {
	// Try to detect AMD monitorx first.
	uint32_t eax, ebx, ecx, edx;
	__cpuid_count(0x80000000U, 0, eax, ebx, ecx, edx);

	// if CPUID limit is >= 0x8000_0001.
	if (eax >= 0x80000001U) {
		__cpuid_count(0x80000001U, 0, eax, ebx, ecx, edx);
#define MONITORX_BIT 29
		if ((ecx >> MONITORX_BIT) & 1) {
			// Monitorx supported.
			Features.supports_wfe_mutex = true;

			// Monitorx always supports waiting with a maximum wait time.
			Features.supports_timed_wfe_mutex = true;

			// Monitorx always supports low power cstate toggle
			Features.supports_low_power_cstate_toggle = true;

			Features.wait_for_value_i8  = mwaitx_wait_for_value_i8;
			Features.wait_for_value_i16 = mwaitx_wait_for_value_i16;
			Features.wait_for_value_i32 = mwaitx_wait_for_value_i32;
			Features.wait_for_value_i64 = mwaitx_wait_for_value_i64;

			Features.wait_for_value_timeout_i8  = mwaitx_wait_for_value_timeout_i8;
			Features.wait_for_value_timeout_i16 = mwaitx_wait_for_value_timeout_i16;
			Features.wait_for_value_timeout_i32 = mwaitx_wait_for_value_timeout_i32;
			Features.wait_for_value_timeout_i64 = mwaitx_wait_for_value_timeout_i64;

			Features.wait_for_bit_set_i8 = mwaitx_wait_for_bit_set_i8;
			Features.wait_for_bit_set_i16 = mwaitx_wait_for_bit_set_i16;
			Features.wait_for_bit_set_i32 = mwaitx_wait_for_bit_set_i32;
			Features.wait_for_bit_set_i64 = mwaitx_wait_for_bit_set_i64;

			Features.wait_for_bit_not_set_i8 = mwaitx_wait_for_bit_not_set_i8;
			Features.wait_for_bit_not_set_i16 = mwaitx_wait_for_bit_not_set_i16;
			Features.wait_for_bit_not_set_i32 = mwaitx_wait_for_bit_not_set_i32;
			Features.wait_for_bit_not_set_i64 = mwaitx_wait_for_bit_not_set_i64;
			return;
		}
	}

	// Try and detect Intel umonitor through waitpkg extension.
	__cpuid_count(0, 0, eax, ebx, ecx, edx);

	// if CPUID limit is >= 7
	if (eax >= 7) {
		__cpuid_count(7, 0, eax, ebx, ecx, edx);
#define WAITPKG_BIT 5
		if ((ecx >> WAITPKG_BIT) & 1) {
			// waitpkg supported.
			Features.supports_wfe_mutex = true;

			// waitpkg always supports waiting with a maximum wait time.
			Features.supports_timed_wfe_mutex = true;

			// waitpkg always supports low power cstate toggle
			Features.supports_low_power_cstate_toggle = true;

			Features.wait_for_value_i8  = waitpkg_wait_for_value_i8;
			Features.wait_for_value_i16 = waitpkg_wait_for_value_i16;
			Features.wait_for_value_i32 = waitpkg_wait_for_value_i32;
			Features.wait_for_value_i64 = waitpkg_wait_for_value_i64;

			Features.wait_for_value_timeout_i8  = waitpkg_wait_for_value_timeout_i8;
			Features.wait_for_value_timeout_i16 = waitpkg_wait_for_value_timeout_i16;
			Features.wait_for_value_timeout_i32 = waitpkg_wait_for_value_timeout_i32;
			Features.wait_for_value_timeout_i64 = waitpkg_wait_for_value_timeout_i64;

			Features.wait_for_bit_set_i8 = waitpkg_wait_for_bit_set_i8;
			Features.wait_for_bit_set_i16 = waitpkg_wait_for_bit_set_i16;
			Features.wait_for_bit_set_i32 = waitpkg_wait_for_bit_set_i32;
			Features.wait_for_bit_set_i64 = waitpkg_wait_for_bit_set_i64;

			Features.wait_for_bit_not_set_i8 = waitpkg_wait_for_bit_not_set_i8;
			Features.wait_for_bit_not_set_i16 = waitpkg_wait_for_bit_not_set_i16;
			Features.wait_for_bit_not_set_i32 = waitpkg_wait_for_bit_not_set_i32;
			Features.wait_for_bit_not_set_i64 = waitpkg_wait_for_bit_not_set_i64;
		}
	}
}

void detect_cycle_counter_frequency() {
	// Intel reports the TSC frequency in CPUID which is nice.
	bool has_cycle_counter = false;
	uint32_t eax, ebx, ecx, edx;
	__cpuid_count(0, 0, eax, ebx, ecx, edx);
	if (eax >= 0x15) {
		// CPUID function 0x15 returns a frequency if eax, ebx, and ecx are all not zero.
		__cpuid_count(0x15, 0, eax, ebx, ecx, edx);

		if (eax && ebx && ecx) {
			// EAX = Denominator of TSC / "core crystal clock" ratio
			// EBX = Numerator of TSC / "core crystal clock" ratio
			// ECX = "core crystal clock frequency"
			double Frequency = (double)ecx * ((double)ebx / (double)eax);
			Features.cycle_hz = Frequency;
			has_cycle_counter = true;
		}
	}

	if (!has_cycle_counter) {
		// If TSC frequency isn't reported through cpuid like on AMD CPUs and older Intel CPUs then it must be calculated.
		// Calculate the cycle counter frequency by spin-looping five times and getting the minimum cycle counter times.

		uint64_t rdtsc_min = ~0ULL;
		for (size_t i = 0; i < 5; ++i) {
#define NANOSECOND_PER_MILLISECOND 1000000
			struct timespec ts_start;
			struct timespec ts_end;

			uint64_t rdtsc_start = read_cycle_counter();
			clock_gettime(CLOCK_MONOTONIC, &ts_start);

			// Spin-loop until one millisecond has elapsed.
			do {
				if (clock_gettime(CLOCK_MONOTONIC, &ts_end) == -1) continue;
			} while (ts_end.tv_nsec < (ts_start.tv_nsec + NANOSECOND_PER_MILLISECOND));

			uint64_t rdtsc_end = read_cycle_counter();
			uint64_t rdtsc_diff = rdtsc_end - rdtsc_start;
			rdtsc_min = rdtsc_diff <= rdtsc_min ? rdtsc_diff : rdtsc_min;
		}

		Features.cycle_hz = rdtsc_min * 1000ULL;
	}

	// Modern x86 CPUs have a very high cycle counter frequency.
	// AMD Zen 3 5995WX = 2.7Ghz.

	// Assume that a 10,000 cycle scale is close enough to not matter.
	// TODO: Make this more accurate?
	const uint64_t NanosecondsInSecond = 1000000000ULL;
	Features.cycles_per_nanosecond_multiplier = 10000;
	Features.cycles_per_nanosecond_divisor = (NanosecondsInSecond * 10000) / Features.cycle_hz;
}

#else
#error Unknown arch and no fallback
#endif

void wfe_mutex_detect_features() {
	detect();
	detect_cycle_counter_frequency();
}

const wfe_mutex_features *wfe_mutex_get_features() {
	return &Features;
}
