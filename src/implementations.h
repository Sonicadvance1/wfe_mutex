#pragma once
#include <stdbool.h>
#include <stdint.h>

// spin-loop implementations
void spinloop_wait_for_value_i8 (uint8_t *ptr,  uint8_t value, bool low_power);
void spinloop_wait_for_value_i16(uint16_t *ptr, uint16_t value, bool low_power);
void spinloop_wait_for_value_i32(uint32_t *ptr, uint32_t value, bool low_power);
void spinloop_wait_for_value_i64(uint64_t *ptr, uint64_t value, bool low_power);

bool spinloop_wait_for_value_timeout_i8 (uint8_t *ptr,  uint8_t value, uint64_t nanoseconds, bool low_power);
bool spinloop_wait_for_value_timeout_i16(uint16_t *ptr, uint16_t value, uint64_t nanoseconds, bool low_power);
bool spinloop_wait_for_value_timeout_i32(uint32_t *ptr, uint32_t value, uint64_t nanoseconds, bool low_power);
bool spinloop_wait_for_value_timeout_i64(uint64_t *ptr, uint64_t value, uint64_t nanoseconds, bool low_power);

#ifdef _M_ARM_64
// wfe implementation
void wfe_wait_for_value_i8 (uint8_t *ptr,  uint8_t value, bool low_power);
void wfe_wait_for_value_i16(uint16_t *ptr, uint16_t value, bool low_power);
void wfe_wait_for_value_i32(uint32_t *ptr, uint32_t value, bool low_power);
void wfe_wait_for_value_i64(uint64_t *ptr, uint64_t value, bool low_power);

bool wfe_wait_for_value_timeout_i8 (uint8_t *ptr,  uint8_t value, uint64_t nanoseconds, bool low_power);
bool wfe_wait_for_value_timeout_i16(uint16_t *ptr, uint16_t value, uint64_t nanoseconds, bool low_power);
bool wfe_wait_for_value_timeout_i32(uint32_t *ptr, uint32_t value, uint64_t nanoseconds, bool low_power);
bool wfe_wait_for_value_timeout_i64(uint64_t *ptr, uint64_t value, uint64_t nanoseconds, bool low_power);

bool wfet_wait_for_value_timeout_i8 (uint8_t *ptr,  uint8_t value, uint64_t nanoseconds, bool low_power);
bool wfet_wait_for_value_timeout_i16(uint16_t *ptr, uint16_t value, uint64_t nanoseconds, bool low_power);
bool wfet_wait_for_value_timeout_i32(uint32_t *ptr, uint32_t value, uint64_t nanoseconds, bool low_power);
bool wfet_wait_for_value_timeout_i64(uint64_t *ptr, uint64_t value, uint64_t nanoseconds, bool low_power);
#elif defined(_M_X86_64)

// monitorx implementation
void mwaitx_wait_for_value_i8 (uint8_t *ptr,  uint8_t value, bool low_power);
void mwaitx_wait_for_value_i16(uint16_t *ptr, uint16_t value, bool low_power);
void mwaitx_wait_for_value_i32(uint32_t *ptr, uint32_t value, bool low_power);
void mwaitx_wait_for_value_i64(uint64_t *ptr, uint64_t value, bool low_power);

bool mwaitx_wait_for_value_timeout_i8 (uint8_t *ptr,  uint8_t value, uint64_t nanoseconds, bool low_power);
bool mwaitx_wait_for_value_timeout_i16(uint16_t *ptr, uint16_t value, uint64_t nanoseconds, bool low_power);
bool mwaitx_wait_for_value_timeout_i32(uint32_t *ptr, uint32_t value, uint64_t nanoseconds, bool low_power);
bool mwaitx_wait_for_value_timeout_i64(uint64_t *ptr, uint64_t value, uint64_t nanoseconds, bool low_power);

#endif

