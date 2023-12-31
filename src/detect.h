#pragma once
#include <wfe_mutex/wfe_mutex.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

void wfe_mutex_detect_features();

static inline uint64_t wfe_mutex_detect_calculate_cycles_for_nanoseconds(uint64_t nanoseconds) {
	const wfe_mutex_features *features = wfe_mutex_get_features();
	return nanoseconds * features->cycles_per_nanosecond_multiplier / features->cycles_per_nanosecond_divisor;
}
