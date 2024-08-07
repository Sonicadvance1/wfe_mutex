#include "detect.h"
#include "implementations.h"
#include "implementation_details_arm.h"
#include "implementation_details_x86.h"

#include <wfe_mutex/wfe_mutex.h>

#include <stdint.h>
#include <time.h>

wfe_mutex_features Features = {
	.cycle_hz = 0,
	.cycles_per_nanosecond_multiplier = 1,
	.cycles_per_nanosecond_divisor = 1,

	.wait_type = WAIT_TYPE_SPIN,
	.wait_type_timeout = WAIT_TYPE_SPIN,

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

	.wait_for_value_spurious_oneshot_i8  = spinloop_wait_for_value_spurious_oneshot_i8,
	.wait_for_value_spurious_oneshot_i16 = spinloop_wait_for_value_spurious_oneshot_i16,
	.wait_for_value_spurious_oneshot_i32 = spinloop_wait_for_value_spurious_oneshot_i32,
	.wait_for_value_spurious_oneshot_i64 = spinloop_wait_for_value_spurious_oneshot_i64,

	.supports_wfe_mutex = false,
	.supports_timed_wfe_mutex = false,
	.supports_low_power_cstate_toggle = false,
};

#if defined(_M_ARM_64) || defined(_M_ARM_32)

#if defined(_M_ARM_64)
uint64_t get_cycle_counter_frequency() {
	uint64_t result;
	__asm ("mrs %[Res], CNTFRQ_EL0;\n"
		: [Res] "=r" (result));

	return result;
}

uint32_t get_exclusive_monitor_granule_size() {
	uint64_t result;
	__asm ("mrs %[Res], CTR_EL0;\n"
		: [Res] "=r" (result));

#define ERG_OFFSET 20
	// Exclusive reservation grnule is described as log2 of words of the maximum reservation size.
	// 0 indicates that this register doesn't provide granule information and worst case 2KB must be assumed.
	uint32_t ERG = (result >> ERG_OFFSET) & 0xF;

	if (ERG == 0) {
		return 2048;
	}

	return (1U << ERG) * sizeof(uint32_t);
}
#else

uint64_t get_cycle_counter_frequency() {
	uint32_t result;

	// Read cntfrq
	__asm volatile(
		"mrc p15, 0, %[Res], c14, c0, 0;\n"
		: [Res] "=r" (result));
	return result;
}

uint32_t get_exclusive_monitor_granule_size() {
	// TODO: Assume 64-bytes, CTR can't be read from userspace in all situations.
	return 64;
}
#endif

static void detect() {
	Features.wait_type = WAIT_TYPE_WFE,
	Features.wait_type_timeout = WAIT_TYPE_WFE,

	Features.monitor_granule_size_bytes_min = Features.monitor_granule_size_bytes_max = get_exclusive_monitor_granule_size();

	Features.wait_for_value_i8  = wfe_wait_for_value_i8;
	Features.wait_for_value_i16 = wfe_wait_for_value_i16;
	Features.wait_for_value_i32 = wfe_wait_for_value_i32;
#if defined(_M_ARM_64)
	Features.wait_for_value_i64 = wfe_wait_for_value_i64;
#endif

	Features.wait_for_value_timeout_i8  = wfe_wait_for_value_timeout_i8;
	Features.wait_for_value_timeout_i16 = wfe_wait_for_value_timeout_i16;
	Features.wait_for_value_timeout_i32 = wfe_wait_for_value_timeout_i32;
#if defined(_M_ARM_64)
	Features.wait_for_value_timeout_i64 = wfe_wait_for_value_timeout_i64;
#endif

	Features.wait_for_value_spurious_oneshot_i8  = wfe_wait_for_value_spurious_oneshot_i8;
	Features.wait_for_value_spurious_oneshot_i16 = wfe_wait_for_value_spurious_oneshot_i16;
	Features.wait_for_value_spurious_oneshot_i32 = wfe_wait_for_value_spurious_oneshot_i32;
#if defined(_M_ARM_64)
	Features.wait_for_value_spurious_oneshot_i64 = wfe_wait_for_value_spurious_oneshot_i64;
#endif

	Features.wait_for_bit_set_i8 = wfe_wait_for_bit_set_i8;
	Features.wait_for_bit_set_i16 = wfe_wait_for_bit_set_i16;
	Features.wait_for_bit_set_i32 = wfe_wait_for_bit_set_i32;
#if defined(_M_ARM_64)
	Features.wait_for_bit_set_i64 = wfe_wait_for_bit_set_i64;
#endif

	Features.wait_for_bit_not_set_i8 = wfe_wait_for_bit_not_set_i8;
	Features.wait_for_bit_not_set_i16 = wfe_wait_for_bit_not_set_i16;
	Features.wait_for_bit_not_set_i32 = wfe_wait_for_bit_not_set_i32;
#if defined(_M_ARM_64)
	Features.wait_for_bit_not_set_i64 = wfe_wait_for_bit_not_set_i64;
#endif

	// ARMv8 always supports wfe_mutex
	Features.supports_wfe_mutex = true;

#if defined(_M_ARM_64)
	// Need to read AA64ISAR2 to see if WFXT is supported.
	// Linux cpuid emulation allows userspace to read this register directly.
	uint64_t isar2;
	__asm ("mrs %[Res], ID_AA64ISAR2_EL1;\n"
		: [Res] "=r" (isar2));
#define WFXT_OFFSET 0
	if ((isar2 >> WFXT_OFFSET) & 0xF) {
		Features.wait_type_timeout = WAIT_TYPE_WFET;
		Features.supports_timed_wfe_mutex = true;

		Features.wait_for_value_timeout_i8  = wfet_wait_for_value_timeout_i8;
		Features.wait_for_value_timeout_i16 = wfet_wait_for_value_timeout_i16;
		Features.wait_for_value_timeout_i32 = wfet_wait_for_value_timeout_i32;
		Features.wait_for_value_timeout_i64 = wfet_wait_for_value_timeout_i64;
	}
#endif

	// ARMv8 doesn't support a lower power cstate toggle.
}

static void detect_cycle_counter_frequency() {
	Features.cycle_hz = get_cycle_counter_frequency();

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
#include <cpuid.h>

static void detect() {
	// Try to detect AMD monitorx first.
	uint32_t eax, ebx, ecx, edx;

	uint32_t feature_limit;
	__cpuid_count(0, 0, eax, ebx, ecx, edx);
	feature_limit = eax;

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

			Features.wait_type = WAIT_TYPE_MONITORX,
			Features.wait_type_timeout = WAIT_TYPE_MONITORX,

			Features.wait_for_value_i8  = mwaitx_wait_for_value_i8;
			Features.wait_for_value_i16 = mwaitx_wait_for_value_i16;
			Features.wait_for_value_i32 = mwaitx_wait_for_value_i32;
			Features.wait_for_value_i64 = mwaitx_wait_for_value_i64;

			Features.wait_for_value_timeout_i8  = mwaitx_wait_for_value_timeout_i8;
			Features.wait_for_value_timeout_i16 = mwaitx_wait_for_value_timeout_i16;
			Features.wait_for_value_timeout_i32 = mwaitx_wait_for_value_timeout_i32;
			Features.wait_for_value_timeout_i64 = mwaitx_wait_for_value_timeout_i64;

			Features.wait_for_value_spurious_oneshot_i8  = mwaitx_wait_for_value_spurious_oneshot_i8;
			Features.wait_for_value_spurious_oneshot_i16 = mwaitx_wait_for_value_spurious_oneshot_i16;
			Features.wait_for_value_spurious_oneshot_i32 = mwaitx_wait_for_value_spurious_oneshot_i32;
			Features.wait_for_value_spurious_oneshot_i64 = mwaitx_wait_for_value_spurious_oneshot_i64;

			Features.wait_for_bit_set_i8 = mwaitx_wait_for_bit_set_i8;
			Features.wait_for_bit_set_i16 = mwaitx_wait_for_bit_set_i16;
			Features.wait_for_bit_set_i32 = mwaitx_wait_for_bit_set_i32;
			Features.wait_for_bit_set_i64 = mwaitx_wait_for_bit_set_i64;

			Features.wait_for_bit_not_set_i8 = mwaitx_wait_for_bit_not_set_i8;
			Features.wait_for_bit_not_set_i16 = mwaitx_wait_for_bit_not_set_i16;
			Features.wait_for_bit_not_set_i32 = mwaitx_wait_for_bit_not_set_i32;
			Features.wait_for_bit_not_set_i64 = mwaitx_wait_for_bit_not_set_i64;
			if (feature_limit >= 5) {
				__cpuid_count(5, 0, eax, ebx, ecx, edx);
				Features.monitor_granule_size_bytes_min = eax & 0xFFFF;
				Features.monitor_granule_size_bytes_max = ebx & 0xFFFF;
			}
			return;
		}
	}

	// Try and detect Intel umonitor through waitpkg extension.

	// if CPUID limit is >= 7
	if (feature_limit >= 7) {
		__cpuid_count(7, 0, eax, ebx, ecx, edx);
#define WAITPKG_BIT 5
		if ((ecx >> WAITPKG_BIT) & 1) {
			// waitpkg supported.
			Features.supports_wfe_mutex = true;

			// waitpkg always supports waiting with a maximum wait time.
			Features.supports_timed_wfe_mutex = true;

			// waitpkg always supports low power cstate toggle
			Features.supports_low_power_cstate_toggle = true;

			Features.wait_type = WAIT_TYPE_WAITPKG,
			Features.wait_type_timeout = WAIT_TYPE_WAITPKG,

			Features.wait_for_value_i8  = waitpkg_wait_for_value_i8;
			Features.wait_for_value_i16 = waitpkg_wait_for_value_i16;
			Features.wait_for_value_i32 = waitpkg_wait_for_value_i32;
			Features.wait_for_value_i64 = waitpkg_wait_for_value_i64;

			Features.wait_for_value_timeout_i8  = waitpkg_wait_for_value_timeout_i8;
			Features.wait_for_value_timeout_i16 = waitpkg_wait_for_value_timeout_i16;
			Features.wait_for_value_timeout_i32 = waitpkg_wait_for_value_timeout_i32;
			Features.wait_for_value_timeout_i64 = waitpkg_wait_for_value_timeout_i64;

			Features.wait_for_value_spurious_oneshot_i8  = waitpkg_wait_for_value_spurious_oneshot_i8;
			Features.wait_for_value_spurious_oneshot_i16 = waitpkg_wait_for_value_spurious_oneshot_i16;
			Features.wait_for_value_spurious_oneshot_i32 = waitpkg_wait_for_value_spurious_oneshot_i32;
			Features.wait_for_value_spurious_oneshot_i64 = waitpkg_wait_for_value_spurious_oneshot_i64;

			Features.wait_for_bit_set_i8 = waitpkg_wait_for_bit_set_i8;
			Features.wait_for_bit_set_i16 = waitpkg_wait_for_bit_set_i16;
			Features.wait_for_bit_set_i32 = waitpkg_wait_for_bit_set_i32;
			Features.wait_for_bit_set_i64 = waitpkg_wait_for_bit_set_i64;

			Features.wait_for_bit_not_set_i8 = waitpkg_wait_for_bit_not_set_i8;
			Features.wait_for_bit_not_set_i16 = waitpkg_wait_for_bit_not_set_i16;
			Features.wait_for_bit_not_set_i32 = waitpkg_wait_for_bit_not_set_i32;
			Features.wait_for_bit_not_set_i64 = waitpkg_wait_for_bit_not_set_i64;
			if (feature_limit >= 5) {
				__cpuid_count(5, 0, eax, ebx, ecx, edx);
				Features.monitor_granule_size_bytes_min = eax & 0xFFFF;
				Features.monitor_granule_size_bytes_max = ebx & 0xFFFF;
			}
		}
	}
}

static void detect_cycle_counter_frequency() {
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
static void detect() {
}

static void detect_cycle_counter_frequency() {
}
#endif

void wfe_mutex_detect_features() {
	detect();
	detect_cycle_counter_frequency();
}

const wfe_mutex_features *wfe_mutex_get_features() {
	return &Features;
}
