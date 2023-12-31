#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef void (*wait_for_value_i8_ptr)(uint8_t *ptr,  uint8_t value, bool low_power);
typedef void (*wait_for_value_i16_ptr)(uint16_t *ptr, uint16_t value, bool low_power);
typedef void (*wait_for_value_i32_ptr)(uint32_t *ptr, uint32_t value, bool low_power);
typedef void (*wait_for_value_i64_ptr)(uint64_t *ptr, uint64_t value, bool low_power);

typedef bool (*wait_for_value_timeout_i8_ptr)(uint8_t *ptr,  uint8_t value, uint64_t nanoseconds, bool low_power);
typedef bool (*wait_for_value_timeout_i16_ptr)(uint16_t *ptr, uint16_t value, uint64_t nanoseconds, bool low_power);
typedef bool (*wait_for_value_timeout_i32_ptr)(uint32_t *ptr, uint32_t value, uint64_t nanoseconds, bool low_power);
typedef bool (*wait_for_value_timeout_i64_ptr)(uint64_t *ptr, uint64_t value, uint64_t nanoseconds, bool low_power);

typedef struct {
	// Frequency of cycle counter.
	uint64_t cycle_hz;

	// Cycle count handler
	uint64_t cycles_per_nanosecond_multiplier;
	uint64_t cycles_per_nanosecond_divisor;

	// Mutex functions
	wait_for_value_i8_ptr  wait_for_value_i8;
	wait_for_value_i16_ptr wait_for_value_i16;
	wait_for_value_i32_ptr wait_for_value_i32;
	wait_for_value_i64_ptr wait_for_value_i64;

	wait_for_value_timeout_i8_ptr  wait_for_value_timeout_i8;
	wait_for_value_timeout_i16_ptr wait_for_value_timeout_i16;
	wait_for_value_timeout_i32_ptr wait_for_value_timeout_i32;
	wait_for_value_timeout_i64_ptr wait_for_value_timeout_i64;

	bool supports_wfe_mutex : 1;
	bool supports_timed_wfe_mutex : 1;
	bool supports_low_power_cstate_toggle : 1;
} wfe_mutex_features;

#ifdef __cplusplus
#define SYMBOL_EXPORT extern "C"
#else
#define SYMBOL_EXPORT
#endif

SYMBOL_EXPORT
void wfe_mutex_init();

// Features
SYMBOL_EXPORT
const wfe_mutex_features *wfe_mutex_get_features();

static inline void wfe_mutex_wait_for_value_i8(uint8_t *ptr, uint8_t value, bool low_power) {
	wfe_mutex_get_features()->wait_for_value_i8(ptr, value, low_power);
}

static inline void wfe_mutex_wait_for_value_i16(uint16_t *ptr, uint16_t value, bool low_power) {
	wfe_mutex_get_features()->wait_for_value_i16(ptr, value, low_power);
}

static inline void wfe_mutex_wait_for_value_i32(uint32_t *ptr, uint32_t value, bool low_power) {
	wfe_mutex_get_features()->wait_for_value_i32(ptr, value, low_power);
}

static inline void wfe_mutex_wait_for_value_i64(uint64_t *ptr, uint64_t value, bool low_power) {
	wfe_mutex_get_features()->wait_for_value_i64(ptr, value, low_power);
}

static inline bool wfe_mutex_wait_for_value_timeout_i8(uint8_t *ptr, uint8_t value, uint64_t nanoseconds, bool low_power) {
	return wfe_mutex_get_features()->wait_for_value_timeout_i8(ptr, value, nanoseconds, low_power);
}

static inline bool wfe_mutex_wait_for_value_timeout_i16(uint16_t *ptr, uint16_t value, uint64_t nanoseconds, bool low_power) {
	return wfe_mutex_get_features()->wait_for_value_timeout_i16(ptr, value, nanoseconds, low_power);
}

static inline bool wfe_mutex_wait_for_value_timeout_i32(uint32_t *ptr, uint32_t value, uint64_t nanoseconds, bool low_power) {
	return wfe_mutex_get_features()->wait_for_value_timeout_i32(ptr, value, nanoseconds, low_power);
}

static inline bool wfe_mutex_wait_for_value_timeout_i64(uint64_t *ptr, uint64_t value, uint64_t nanoseconds, bool low_power) {
	return wfe_mutex_get_features()->wait_for_value_timeout_i64(ptr, value, nanoseconds, low_power);
}
