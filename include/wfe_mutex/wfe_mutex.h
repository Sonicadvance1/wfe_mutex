#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef void (*wait_for_value_i8_ptr)(uint8_t *ptr,  uint8_t value, bool low_power);
typedef void (*wait_for_value_i16_ptr)(uint16_t *ptr, uint16_t value, bool low_power);
typedef void (*wait_for_value_i32_ptr)(uint32_t *ptr, uint32_t value, bool low_power);
typedef void (*wait_for_value_i64_ptr)(uint64_t *ptr, uint64_t value, bool low_power);

typedef uint8_t (*wait_for_bit_set_i8_ptr)(uint8_t *ptr,  uint8_t bit, bool low_power);
typedef uint16_t (*wait_for_bit_set_i16_ptr)(uint16_t *ptr, uint8_t bit, bool low_power);
typedef uint32_t (*wait_for_bit_set_i32_ptr)(uint32_t *ptr, uint8_t bit, bool low_power);
typedef uint64_t (*wait_for_bit_set_i64_ptr)(uint64_t *ptr, uint8_t bit, bool low_power);

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

	wait_for_bit_set_i8_ptr  wait_for_bit_set_i8;
	wait_for_bit_set_i16_ptr wait_for_bit_set_i16;
	wait_for_bit_set_i32_ptr wait_for_bit_set_i32;
	wait_for_bit_set_i64_ptr wait_for_bit_set_i64;

	wait_for_bit_set_i8_ptr  wait_for_bit_not_set_i8;
	wait_for_bit_set_i16_ptr wait_for_bit_not_set_i16;
	wait_for_bit_set_i32_ptr wait_for_bit_not_set_i32;
	wait_for_bit_set_i64_ptr wait_for_bit_not_set_i64;

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

static inline uint8_t wfe_mutex_wait_for_bit_set_i8(uint8_t *ptr, uint8_t bit, bool low_power) {
	return wfe_mutex_get_features()->wait_for_bit_set_i8(ptr, bit, low_power);
}

static inline uint16_t wfe_mutex_wait_for_bit_set_i16(uint16_t *ptr, uint8_t bit, bool low_power) {
	return wfe_mutex_get_features()->wait_for_bit_set_i16(ptr, bit, low_power);
}

static inline uint32_t wfe_mutex_wait_for_bit_set_i32(uint32_t *ptr, uint8_t bit, bool low_power) {
	return wfe_mutex_get_features()->wait_for_bit_set_i32(ptr, bit, low_power);
}

static inline uint64_t wfe_mutex_wait_for_bit_set_i64(uint64_t *ptr, uint8_t bit, bool low_power) {
	return wfe_mutex_get_features()->wait_for_bit_set_i64(ptr, bit, low_power);
}

static inline uint8_t wfe_mutex_wait_for_bit_not_set_i8(uint8_t *ptr, uint8_t bit, bool low_power) {
	return wfe_mutex_get_features()->wait_for_bit_not_set_i8(ptr, bit, low_power);
}

static inline uint16_t wfe_mutex_wait_for_bit_not_set_i16(uint16_t *ptr, uint8_t bit, bool low_power) {
	return wfe_mutex_get_features()->wait_for_bit_not_set_i16(ptr, bit, low_power);
}

static inline uint32_t wfe_mutex_wait_for_bit_not_set_i32(uint32_t *ptr, uint8_t bit, bool low_power) {
	return wfe_mutex_get_features()->wait_for_bit_not_set_i32(ptr, bit, low_power);
}

static inline uint64_t wfe_mutex_wait_for_bit_not_set_i64(uint64_t *ptr, uint8_t bit, bool low_power) {
	return wfe_mutex_get_features()->wait_for_bit_not_set_i64(ptr, bit, low_power);
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

// mutex interface
typedef struct {
	uint32_t mutex;
} wfe_mutex_lock;

typedef struct {
	// Top-bit determines write-lock.
	// Lower 31-bits gives the number of shared locks.
	uint32_t mutex;
} wfe_mutex_rwlock;

#define WFE_MUTEX_LOCK_INITIALIZER \
{ 0 }

#define WFE_MUTEX_RWLOCK_INITIALIZER \
{ 0 }

static inline void wfe_mutex_lock_lock(wfe_mutex_lock *lock, bool low_power) {
	uint32_t expected = 0;
	uint32_t desired = 1;

	// Try to CAS immediately.
	if (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) return;

	do {
		wfe_mutex_wait_for_value_i32(&lock->mutex, 0, low_power);
		expected = 0;
	} while (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
}

static inline bool wfe_mutex_lock_trylock(wfe_mutex_lock *lock) {
	uint32_t expected = 0;
	uint32_t desired = 1;

	// Try to CAS immediately.
	if (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) return true;
	return false;
}

static inline void wfe_mutex_lock_unlock(wfe_mutex_lock *lock) {
	// Unlocking is just storing zero.
	__atomic_store_n(&lock->mutex, 0, __ATOMIC_SEQ_CST);
}

static inline void wfe_mutex_rwlock_rdlock(wfe_mutex_rwlock *lock, bool low_power) {
	// Getting a write-lock is waiting for the top-bit to be zero in the mutex and incrementing the bottom 31-bits.
	const uint32_t TOP_BIT = 1U << 31;
	uint32_t expected = __atomic_load_n(&lock->mutex, __ATOMIC_SEQ_CST) & ~TOP_BIT;
	uint32_t desired = expected + 1;

	if (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) return;

	do {
		expected = wfe_mutex_wait_for_bit_not_set_i32(&lock->mutex, 31, low_power);
	} while (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
}

static inline void wfe_mutex_rwlock_wrlock(wfe_mutex_rwlock *lock, bool low_power) {
	// Getting a write-lock is waiting for a value of zero in the mutex and then setting the top-bit.
	const uint32_t TOP_BIT = 1U << 31;
	uint32_t expected = 0;
	uint32_t desired = TOP_BIT;

	// Try to CAS immediately.
	if (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) return;

	do {
		wfe_mutex_wait_for_value_i32(&lock->mutex, 0, low_power);
		expected = 0;
	} while (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
}

static inline bool wfe_mutex_rwlock_trylock(wfe_mutex_rwlock *lock) {
	// Trying to lock a write lock is trying to set the top bit with the rest being zero.
	const uint32_t TOP_BIT = 1U << 31;
	uint32_t expected = 0;
	uint32_t desired = TOP_BIT;

	if (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) return true;
	return false;
}

static inline bool wfe_mutex_rwlock_trylock_shared(wfe_mutex_rwlock *lock) {
	// Trying to add a read lock is CAS operation to try and increment without top-bit being set.
	// This can spuriously fail if a read-lock is contended.
	// TODO: Should this not spuriously fail if only read lock state is changing?
	const uint32_t TOP_BIT = 1U << 31;
	uint32_t expected = __atomic_load_n(&lock->mutex, __ATOMIC_SEQ_CST) & ~TOP_BIT;
	uint32_t desired = expected + 1;

	if (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) return true;
	return false;
}

static inline void wfe_mutex_rwlock_unlock(wfe_mutex_rwlock *lock) {
	// Unlocking is just storing zero.
	__atomic_store_n(&lock->mutex, 0, __ATOMIC_SEQ_CST);
}

static inline void wfe_mutex_rwlock_read_unlock(wfe_mutex_rwlock *lock) {
	// Unlocked shared is just decrementing 1.
	__atomic_fetch_sub(&lock->mutex, 1, __ATOMIC_SEQ_CST);
}
