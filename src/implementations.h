#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
#define SYMBOL_EXPORT extern "C"
#else
#define SYMBOL_EXPORT
#endif

// spin-loop implementations
void spinloop_wait_for_value_i8 (uint8_t *ptr,  uint8_t value, bool low_power);
void spinloop_wait_for_value_i16(uint16_t *ptr, uint16_t value, bool low_power);
void spinloop_wait_for_value_i32(uint32_t *ptr, uint32_t value, bool low_power);
void spinloop_wait_for_value_i64(uint64_t *ptr, uint64_t value, bool low_power);

uint8_t spinloop_wait_for_bit_set_i8 (uint8_t *ptr,  uint8_t bit, bool low_power);
uint16_t spinloop_wait_for_bit_set_i16(uint16_t *ptr, uint8_t bit, bool low_power);
uint32_t spinloop_wait_for_bit_set_i32(uint32_t *ptr, uint8_t bit, bool low_power);
uint64_t spinloop_wait_for_bit_set_i64(uint64_t *ptr, uint8_t bit, bool low_power);

uint8_t spinloop_wait_for_bit_not_set_i8 (uint8_t *ptr,  uint8_t bit, bool low_power);
uint16_t spinloop_wait_for_bit_not_set_i16(uint16_t *ptr, uint8_t bit, bool low_power);
uint32_t spinloop_wait_for_bit_not_set_i32(uint32_t *ptr, uint8_t bit, bool low_power);
uint64_t spinloop_wait_for_bit_not_set_i64(uint64_t *ptr, uint8_t bit, bool low_power);

bool spinloop_wait_for_value_timeout_i8 (uint8_t *ptr,  uint8_t value, uint64_t nanoseconds, bool low_power);
bool spinloop_wait_for_value_timeout_i16(uint16_t *ptr, uint16_t value, uint64_t nanoseconds, bool low_power);
bool spinloop_wait_for_value_timeout_i32(uint32_t *ptr, uint32_t value, uint64_t nanoseconds, bool low_power);
bool spinloop_wait_for_value_timeout_i64(uint64_t *ptr, uint64_t value, uint64_t nanoseconds, bool low_power);

bool spinloop_wait_for_value_spurious_oneshot_i8 (uint8_t *ptr,  uint8_t value, bool low_power);
bool spinloop_wait_for_value_spurious_oneshot_i16(uint16_t *ptr, uint16_t value, bool low_power);
bool spinloop_wait_for_value_spurious_oneshot_i32(uint32_t *ptr, uint32_t value, bool low_power);
bool spinloop_wait_for_value_spurious_oneshot_i64(uint64_t *ptr, uint64_t value, bool low_power);

#if defined(_M_ARM_64) || defined(_M_ARM_32)
// wfe implementation
void wfe_wait_for_value_i8 (uint8_t *ptr,  uint8_t value, bool low_power);
void wfe_wait_for_value_i16(uint16_t *ptr, uint16_t value, bool low_power);
void wfe_wait_for_value_i32(uint32_t *ptr, uint32_t value, bool low_power);

#if defined(_M_ARM_64)
void wfe_wait_for_value_i64(uint64_t *ptr, uint64_t value, bool low_power);
#endif

uint8_t wfe_wait_for_bit_set_i8 (uint8_t *ptr,  uint8_t bit, bool low_power);
uint16_t wfe_wait_for_bit_set_i16(uint16_t *ptr, uint8_t bit, bool low_power);
uint32_t wfe_wait_for_bit_set_i32(uint32_t *ptr, uint8_t bit, bool low_power);
#if defined(_M_ARM_64)
uint64_t wfe_wait_for_bit_set_i64(uint64_t *ptr, uint8_t bit, bool low_power);
#endif

uint8_t wfe_wait_for_bit_not_set_i8 (uint8_t *ptr,  uint8_t bit, bool low_power);
uint16_t wfe_wait_for_bit_not_set_i16(uint16_t *ptr, uint8_t bit, bool low_power);
uint32_t wfe_wait_for_bit_not_set_i32(uint32_t *ptr, uint8_t bit, bool low_power);
#if defined(_M_ARM_64)
uint64_t wfe_wait_for_bit_not_set_i64(uint64_t *ptr, uint8_t bit, bool low_power);
#endif

bool wfe_wait_for_value_timeout_i8 (uint8_t *ptr,  uint8_t value, uint64_t nanoseconds, bool low_power);
bool wfe_wait_for_value_timeout_i16(uint16_t *ptr, uint16_t value, uint64_t nanoseconds, bool low_power);
bool wfe_wait_for_value_timeout_i32(uint32_t *ptr, uint32_t value, uint64_t nanoseconds, bool low_power);
#if defined(_M_ARM_64)
bool wfe_wait_for_value_timeout_i64(uint64_t *ptr, uint64_t value, uint64_t nanoseconds, bool low_power);
#endif

bool wfe_wait_for_value_spurious_oneshot_i8 (uint8_t *ptr,  uint8_t value, bool low_power);
bool wfe_wait_for_value_spurious_oneshot_i16(uint16_t *ptr, uint16_t value, bool low_power);
bool wfe_wait_for_value_spurious_oneshot_i32(uint32_t *ptr, uint32_t value, bool low_power);
#if defined(_M_ARM_64)
bool wfe_wait_for_value_spurious_oneshot_i64(uint64_t *ptr, uint64_t value, bool low_power);
#endif

#if defined(_M_ARM_64)
bool wfet_wait_for_value_timeout_i8 (uint8_t *ptr,  uint8_t value, uint64_t nanoseconds, bool low_power);
bool wfet_wait_for_value_timeout_i16(uint16_t *ptr, uint16_t value, uint64_t nanoseconds, bool low_power);
bool wfet_wait_for_value_timeout_i32(uint32_t *ptr, uint32_t value, uint64_t nanoseconds, bool low_power);
bool wfet_wait_for_value_timeout_i64(uint64_t *ptr, uint64_t value, uint64_t nanoseconds, bool low_power);
#endif
#elif defined(_M_X86_64) || defined(_M_X86_32)

// monitorx implementation
SYMBOL_EXPORT void mwaitx_wait_for_value_i8 (uint8_t *ptr,  uint8_t value, bool low_power);
SYMBOL_EXPORT void mwaitx_wait_for_value_i16(uint16_t *ptr, uint16_t value, bool low_power);
SYMBOL_EXPORT void mwaitx_wait_for_value_i32(uint32_t *ptr, uint32_t value, bool low_power);
SYMBOL_EXPORT void mwaitx_wait_for_value_i64(uint64_t *ptr, uint64_t value, bool low_power);

SYMBOL_EXPORT uint8_t mwaitx_wait_for_bit_set_i8 (uint8_t *ptr,  uint8_t bit, bool low_power);
SYMBOL_EXPORT uint16_t mwaitx_wait_for_bit_set_i16(uint16_t *ptr, uint8_t bit, bool low_power);
SYMBOL_EXPORT uint32_t mwaitx_wait_for_bit_set_i32(uint32_t *ptr, uint8_t bit, bool low_power);
SYMBOL_EXPORT uint64_t mwaitx_wait_for_bit_set_i64(uint64_t *ptr, uint8_t bit, bool low_power);

SYMBOL_EXPORT uint8_t mwaitx_wait_for_bit_not_set_i8 (uint8_t *ptr,  uint8_t bit, bool low_power);
SYMBOL_EXPORT uint16_t mwaitx_wait_for_bit_not_set_i16(uint16_t *ptr, uint8_t bit, bool low_power);
SYMBOL_EXPORT uint32_t mwaitx_wait_for_bit_not_set_i32(uint32_t *ptr, uint8_t bit, bool low_power);
SYMBOL_EXPORT uint64_t mwaitx_wait_for_bit_not_set_i64(uint64_t *ptr, uint8_t bit, bool low_power);

SYMBOL_EXPORT bool mwaitx_wait_for_value_timeout_i8 (uint8_t *ptr,  uint8_t value, uint64_t nanoseconds, bool low_power);
SYMBOL_EXPORT bool mwaitx_wait_for_value_timeout_i16(uint16_t *ptr, uint16_t value, uint64_t nanoseconds, bool low_power);
SYMBOL_EXPORT bool mwaitx_wait_for_value_timeout_i32(uint32_t *ptr, uint32_t value, uint64_t nanoseconds, bool low_power);
SYMBOL_EXPORT bool mwaitx_wait_for_value_timeout_i64(uint64_t *ptr, uint64_t value, uint64_t nanoseconds, bool low_power);

SYMBOL_EXPORT bool mwaitx_wait_for_value_spurious_oneshot_i8 (uint8_t *ptr,  uint8_t value, bool low_power);
SYMBOL_EXPORT bool mwaitx_wait_for_value_spurious_oneshot_i16(uint16_t *ptr, uint16_t value, bool low_power);
SYMBOL_EXPORT bool mwaitx_wait_for_value_spurious_oneshot_i32(uint32_t *ptr, uint32_t value, bool low_power);
SYMBOL_EXPORT bool mwaitx_wait_for_value_spurious_oneshot_i64(uint64_t *ptr, uint64_t value, bool low_power);

// waitpkg implementation
SYMBOL_EXPORT void waitpkg_wait_for_value_i8 (uint8_t *ptr,  uint8_t value, bool low_power);
SYMBOL_EXPORT void waitpkg_wait_for_value_i16(uint16_t *ptr, uint16_t value, bool low_power);
SYMBOL_EXPORT void waitpkg_wait_for_value_i32(uint32_t *ptr, uint32_t value, bool low_power);
SYMBOL_EXPORT void waitpkg_wait_for_value_i64(uint64_t *ptr, uint64_t value, bool low_power);

SYMBOL_EXPORT uint8_t waitpkg_wait_for_bit_set_i8 (uint8_t *ptr,  uint8_t bit, bool low_power);
SYMBOL_EXPORT uint16_t waitpkg_wait_for_bit_set_i16(uint16_t *ptr, uint8_t bit, bool low_power);
SYMBOL_EXPORT uint32_t waitpkg_wait_for_bit_set_i32(uint32_t *ptr, uint8_t bit, bool low_power);
SYMBOL_EXPORT uint64_t waitpkg_wait_for_bit_set_i64(uint64_t *ptr, uint8_t bit, bool low_power);

SYMBOL_EXPORT uint8_t waitpkg_wait_for_bit_not_set_i8 (uint8_t *ptr,  uint8_t bit, bool low_power);
SYMBOL_EXPORT uint16_t waitpkg_wait_for_bit_not_set_i16(uint16_t *ptr, uint8_t bit, bool low_power);
SYMBOL_EXPORT uint32_t waitpkg_wait_for_bit_not_set_i32(uint32_t *ptr, uint8_t bit, bool low_power);
SYMBOL_EXPORT uint64_t waitpkg_wait_for_bit_not_set_i64(uint64_t *ptr, uint8_t bit, bool low_power);

SYMBOL_EXPORT bool waitpkg_wait_for_value_timeout_i8 (uint8_t *ptr,  uint8_t value, uint64_t nanoseconds, bool low_power);
SYMBOL_EXPORT bool waitpkg_wait_for_value_timeout_i16(uint16_t *ptr, uint16_t value, uint64_t nanoseconds, bool low_power);
SYMBOL_EXPORT bool waitpkg_wait_for_value_timeout_i32(uint32_t *ptr, uint32_t value, uint64_t nanoseconds, bool low_power);
SYMBOL_EXPORT bool waitpkg_wait_for_value_timeout_i64(uint64_t *ptr, uint64_t value, uint64_t nanoseconds, bool low_power);

SYMBOL_EXPORT bool waitpkg_wait_for_value_spurious_oneshot_i8 (uint8_t *ptr,  uint8_t value, bool low_power);
SYMBOL_EXPORT bool waitpkg_wait_for_value_spurious_oneshot_i16(uint16_t *ptr, uint16_t value, bool low_power);
SYMBOL_EXPORT bool waitpkg_wait_for_value_spurious_oneshot_i32(uint32_t *ptr, uint32_t value, bool low_power);
SYMBOL_EXPORT bool waitpkg_wait_for_value_spurious_oneshot_i64(uint64_t *ptr, uint64_t value, bool low_power);

#endif

