#pragma once
#include <wfe_mutex/wfe_mutex.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

extern wfe_mutex_features Features;

static inline uint64_t wfe_mutex_detect_calculate_cycles_for_nanoseconds(uint64_t nanoseconds) {
	return nanoseconds * Features.cycles_per_nanosecond_multiplier / Features.cycles_per_nanosecond_divisor;
}
