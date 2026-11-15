#include <wfe_mutex/wfe_mutex.h>
#include <catch2/catch_all.hpp>

#include <sys/mman.h>

namespace {
	const uint64_t NanosecondsInSecond = 1000000000ULL;
	const wfe_mutex_features *Features;
}

static inline uint64_t wfe_mutex_detect_calculate_cycles_for_nanoseconds(uint64_t nanoseconds) {
	return nanoseconds * Features->cycles_per_nanosecond_multiplier / Features->cycles_per_nanosecond_divisor;
}

// Convert nanoseconds to cycles.
uint64_t float_calculate_cycles_for_nanoseconds(uint64_t nanoseconds) {
	long double ns_f = nanoseconds;
	long double hz_f = Features->cycle_hz;
	constexpr long double nanosecond_in_second = 1000000000ULL;
	return ns_f * (hz_f / nanosecond_in_second);
}

static inline uint64_t wfe_mutex_detect_calculate_nanoseconds_for_cycles(uint64_t cycles) {
	return cycles * Features->nanoseconds_per_cycle_multiplier / Features->nanoseconds_per_cycle_divisor;
}

// Float convert cycles to nanoseconds
uint64_t float_calculate_nanoseconds_for_cycles(uint64_t cycles) {
	long double cycles_f = cycles;
	long double hz_f = Features->cycle_hz;
	constexpr long double nanosecond_in_second = 1000000000ULL;
	return cycles_f * (nanosecond_in_second / hz_f);
}


TEST_CASE("Check accuracy: nanoseconds->cycles") {
	wfe_mutex_init();
	Features = wfe_mutex_get_features();

	struct TestData {
		uint64_t nanoseconds;
		int64_t cycle_ns_tolerance;
	};

	TestData TestData [] = {
		// 1 ns: zero tolerance
		{1ULL, 0},
		// 1 us: zero tolerance
		{1000ULL, 0},
		// 1 ms: 1us tolerance
		{1000000ULL, 1ULL * 1000ULL},
		// 1 Second: 1us tolerance
		{1000000000ULL, 1ULL * 1000ULL},
		// 1 Minute: 1ms tolerance
		{NanosecondsInSecond * 60, 1ULL * 1000ULL * 1000ULL},
		// 1 Hour: 10ms tolerance
		{NanosecondsInSecond * 60 * 60, 10ULL * 1000ULL * 1000ULL},
	};

	for (const auto& Data : TestData) {
		uint64_t Converted_WFE = wfe_mutex_detect_calculate_cycles_for_nanoseconds(Data.nanoseconds);
		uint64_t Converted_Float = float_calculate_cycles_for_nanoseconds(Data.nanoseconds);
		const int64_t tolerance = float_calculate_cycles_for_nanoseconds(Data.cycle_ns_tolerance);

		// Only allow a certain amount of tolerance.
		// Allow only one cycle for one second and below.
		// Another above that it starts skewing.
		// By default wfe_mutex tolerates 1 hour of busy waiting. 240 cycle skew over 1 hour is tolerable.
		// If the cycle counter is significantly faster than 1Ghz then skews get larger.
		//   - Even a 4Ghz cycle counter only skews ~2ms over the course of an hour.
		CHECK(std::abs(static_cast<int64_t>(Converted_WFE - Converted_Float)) <= tolerance);
	}
}

TEST_CASE("Check accuracy: cycles->nanoseconds") {
	wfe_mutex_init();
	Features = wfe_mutex_get_features();

	struct TestData {
		uint64_t cycles;
		int64_t cycle_tolerance;
	};

	// Allow a small tolerance.
	TestData TestData [] = {
		// 1 cycle
		{1ULL, 0},
		// 1 microsecond.
		{Features->cycle_hz / 1000ULL, 10}, // 1%
		// 1 millisecond.
		{Features->cycle_hz / 1000000ULL, 10000ULL}, // 1% = 0.01ms
		// 1 second
		{Features->cycle_hz, 100000ULL}, // 0.01% = 0.1ms
		// 1 Minute.
		{Features->cycle_hz * 60, 6000000ULL}, // 0.01% = 6ms
		// 1 Hour.
		{Features->cycle_hz * 60 * 60, 360000000ULL}, // 0.01% = 360ms
	};

	for (const auto& Data : TestData) {
		uint64_t Converted_WFE = wfe_mutex_detect_calculate_nanoseconds_for_cycles(Data.cycles);
		uint64_t Converted_Float = float_calculate_nanoseconds_for_cycles(Data.cycles);
		CHECK(std::abs(static_cast<int64_t>(Converted_WFE - Converted_Float)) <= Data.cycle_tolerance);
	}
}
