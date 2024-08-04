#pragma once
#include <stdbool.h>
#include <stdint.h>

// Enable debugging if NDEBUG is not defined and WFE_MUTEX_DEBUG also isn't already defined.
#ifndef NDEBUG
#ifndef WFE_MUTEX_DEBUG
#define WFE_MUTEX_DEBUG 1
#endif
#endif

#if defined(WFE_MUTEX_DEBUG) && WFE_MUTEX_DEBUG == 1
#include <string.h>
#include <unistd.h>
#endif

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

typedef enum {
	WAIT_TYPE_SPIN,
	WAIT_TYPE_WFE,
	WAIT_TYPE_WFET,
	WAIT_TYPE_WAITPKG,
	WAIT_TYPE_MONITORX,
} wfe_mutex_wait_types;

typedef struct {
	// Frequency of cycle counter.
	uint64_t cycle_hz;

	// Cycle count handler
	uint64_t cycles_per_nanosecond_multiplier;
	uint64_t cycles_per_nanosecond_divisor;

	///< The size in which the monitor granule size is.
	/// For optimal monitor usage, mutexes should not be in overlapping granules.
	/// minimum and maximum are usually identical and also usually are a cacheline size.
	uint32_t monitor_granule_size_bytes_min;
	uint32_t monitor_granule_size_bytes_max;

	///< What is used for waiting.
	wfe_mutex_wait_types wait_type;

	///< What is used for waiting when a timeout is provided.
	wfe_mutex_wait_types wait_type_timeout;

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

#if defined(WFE_MUTEX_DEBUG) && WFE_MUTEX_DEBUG == 1
static inline void print_error(const char* msg) {
	write(STDERR_FILENO, msg, strlen(msg));
	__builtin_unreachable();
	__builtin_trap();
}

static inline void sanity_check_rdwrlock_value(uint32_t value) {
	const uint32_t TOP_BIT = 1U << 31;

	if (value & TOP_BIT) {
		// If the top bit is set then we are expecting all the lower bits to be zero.
		if (value & ~TOP_BIT) {
			// Programming error.
			print_error("rdwrlock state inconsistent! Has write lock set and also shared mutex bits!\n");
		}
	}
}

static inline void sanity_check_rdwrlock_read_ready_value(uint32_t value) {
	const uint32_t TOP_BIT = 1U << 31;

	if (value & TOP_BIT) {
		// Programming error.
		print_error("rdwrlock state inconsistent! Expected no writer ownership!!\n");
	}
}

static inline void sanity_check_rdwrlock_mutex(uint32_t *mutex) {
	uint32_t value = __atomic_load_n(mutex, __ATOMIC_SEQ_CST);
	sanity_check_rdwrlock_value(value);
}

static inline void sanity_check_rdwrlock_unlock_mutex(uint32_t *mutex) {
	// On mutex unlock the top bit must be set and the rest of the bits zero
	const uint32_t TOP_BIT = 1U << 31;

	uint32_t value = __atomic_load_n(mutex, __ATOMIC_SEQ_CST);

	if (value & TOP_BIT) {
		// If the top bit is set then we are expecting all the lower bits to be zero.
		if (value & ~TOP_BIT) {
			// Programming error.
			print_error("rdwrlock state inconsistent! Has write lock set and also shared mutex bits!\n");
		}
	}
	else {
		// If top bit is unset with unlock then this is a programming error.
		if (value & ~TOP_BIT) {
			print_error("rdwrlock trying to write unlock. Wasn't unique locked and has shared readers!\n");
		}
		else {
			print_error("rdwrlock trying to write unlock. Wasn't unique locked!\n");
		}
	}
}

static inline void sanity_check_rdwrlock_unlock_shared_mutex(uint32_t *mutex) {
	// On shared unlock the top-bit must not be set and the lower bits must not be zero.
	const uint32_t TOP_BIT = 1U << 31;

	uint32_t value = __atomic_load_n(mutex, __ATOMIC_SEQ_CST);

	if (value & TOP_BIT) {
		print_error("rdwrlock trying to read unlock. Was unique locked!\n");
	}
	else {
		if ((value & ~TOP_BIT) == 0) {
			print_error("rdwrlock trying to read unlock. Wasn't read locked!\n");
		}
	}
}

static inline void sanity_check_wrlock_value(uint32_t value) {
	// Write lock values can only be 0 and 1
	if (value & ~1U) {
		// Programming error.
		print_error("wrlock state inconsistent! Has invalid upper bits set!\n");
	}
}

static inline void sanity_check_wrlock_mutex(uint32_t *mutex) {
	uint32_t value = __atomic_load_n(mutex, __ATOMIC_SEQ_CST);
	sanity_check_wrlock_value(value);
}

static inline void sanity_check_wrlock_unlock_mutex(uint32_t *mutex) {
	// On mutex unlock, only the bottom bit must be set
	uint32_t value = __atomic_load_n(mutex, __ATOMIC_SEQ_CST);


	// Write lock values can only be 0 and 1
	if (value & ~1U) {
		// Programming error.
		print_error("wrlock state inconsistent! Has invalid upper bits set!\n");
	}
	if ((value & 1) == 0) {
		// Tried to unlock a mutex that isn't locked.
		print_error("rdwrlock trying to write unlock. Wasn't unique locked!\n");
	}
}

#else
// readwrite lock mutex checks
static inline void sanity_check_rdwrlock_value(uint32_t value) {}
static inline void sanity_check_rdwrlock_mutex(uint32_t *mutex) {}
static inline void sanity_check_rdwrlock_unlock_mutex(uint32_t *mutex) {}
static inline void sanity_check_rdwrlock_unlock_shared_mutex(uint32_t *mutex) {}
static inline void sanity_check_rdwrlock_read_ready_value(uint32_t value) {}

// write lock mutex checks
static inline void sanity_check_wrlock_value(uint32_t value) {}
static inline void sanity_check_wrlock_mutex(uint32_t *mutex) {}
static inline void sanity_check_wrlock_unlock_mutex(uint32_t *mutex) {}
#endif

static inline void wfe_mutex_lock_lock(wfe_mutex_lock *lock, bool low_power) {
	uint32_t expected = 0;
	uint32_t desired = 1;

	sanity_check_wrlock_mutex(&lock->mutex);

	// Try to CAS immediately.
	if (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE)) return;

	do {
		wfe_mutex_wait_for_value_i32(&lock->mutex, 0, low_power);
		expected = 0;
		sanity_check_wrlock_mutex(&lock->mutex);
	} while (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE) == false);
}

static inline bool wfe_mutex_lock_trylock(wfe_mutex_lock *lock) {
	uint32_t expected = 0;
	uint32_t desired = 1;

	sanity_check_wrlock_mutex(&lock->mutex);

	// Try to CAS immediately.
	if (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE)) return true;
	return false;
}

static inline void wfe_mutex_lock_unlock(wfe_mutex_lock *lock) {
	sanity_check_wrlock_mutex(&lock->mutex);

	sanity_check_wrlock_unlock_mutex(&lock->mutex);

	// Unlocking is just storing zero.
	__atomic_store_n(&lock->mutex, 0, __ATOMIC_RELEASE);
}

static inline bool wfe_mutex_lock_timedlock(wfe_mutex_lock *lock, uint64_t nanoseconds, bool low_power) {
	uint32_t expected = 0;
	uint32_t desired = 1;

	sanity_check_wrlock_mutex(&lock->mutex);

	// Try to CAS immediately.
	if (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE)) return true;

	do {
		// If timed-out then early exit
		if (!wfe_mutex_wait_for_value_timeout_i32(&lock->mutex, 0, nanoseconds, low_power)) return false;
		expected = 0;
		sanity_check_wrlock_mutex(&lock->mutex);
	} while (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE) == false);

	return true;
}

static inline void wfe_mutex_rwlock_rdlock(wfe_mutex_rwlock *lock, bool low_power) {
	sanity_check_rdwrlock_mutex(&lock->mutex);

	// Getting a write-lock is waiting for the top-bit to be zero in the mutex and incrementing the bottom 31-bits.
	const uint32_t TOP_BIT = 1U << 31;
	uint32_t expected = 0;
	uint32_t desired = expected + 1;

	// Uncontended mutex check.
	if (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE)) return;

	// Read-only mutex check
	expected &= ~TOP_BIT;
	desired = expected + 1;
	if (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE)) return;

	do {
		expected = wfe_mutex_wait_for_bit_not_set_i32(&lock->mutex, 31, low_power);
		sanity_check_rdwrlock_value(expected);
		sanity_check_rdwrlock_read_ready_value(expected);
		// write-lock bit no longer set, increment by one to obtain read-lock.
		desired += 1;
	} while (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE) == false);
}

static inline void wfe_mutex_rwlock_wrlock(wfe_mutex_rwlock *lock, bool low_power) {
	sanity_check_rdwrlock_mutex(&lock->mutex);

	// Getting a write-lock is waiting for a value of zero in the mutex and then setting the top-bit.
	const uint32_t TOP_BIT = 1U << 31;
	uint32_t expected = 0;
	uint32_t desired = TOP_BIT;

	// Try to CAS immediately.
	if (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE)) return;

	do {
		wfe_mutex_wait_for_value_i32(&lock->mutex, 0, low_power);
		expected = 0;
		sanity_check_rdwrlock_mutex(&lock->mutex);
	} while (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE) == false);
}

// TODO: Need to implement.
// static inline bool wfe_mutex_rwlock_timedrdlock(wfe_mutex_rwlock *lock, uint64_t nanoseconds, bool low_power);

static inline bool wfe_mutex_rwlock_timedwrlock(wfe_mutex_rwlock *lock, uint64_t nanoseconds, bool low_power) {
	sanity_check_rdwrlock_mutex(&lock->mutex);

	// Getting a write-lock is waiting for a value of zero in the mutex and then setting the top-bit.
	const uint32_t TOP_BIT = 1U << 31;
	uint32_t expected = 0;
	uint32_t desired = TOP_BIT;

	// Try to CAS immediately.
	if (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE)) return true;

	do {
		if (!wfe_mutex_wait_for_value_timeout_i32(&lock->mutex, 0, nanoseconds, low_power)) return false;
		expected = 0;
		sanity_check_rdwrlock_mutex(&lock->mutex);
	} while (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE) == false);
	return true;
}

static inline bool wfe_mutex_rwlock_trylock(wfe_mutex_rwlock *lock) {
	sanity_check_rdwrlock_mutex(&lock->mutex);

	// Trying to lock a write lock is trying to set the top bit with the rest being zero.
	const uint32_t TOP_BIT = 1U << 31;
	uint32_t expected = 0;
	uint32_t desired = TOP_BIT;

	if (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE)) return true;
	return false;
}

static inline bool wfe_mutex_rwlock_trylock_shared(wfe_mutex_rwlock *lock) {
	sanity_check_rdwrlock_mutex(&lock->mutex);

	// Trying to add a read lock is CAS operation to try and increment without top-bit being set.
	// This can spuriously fail if a read-lock is contended.
	// TODO: Should this not spuriously fail if only read lock state is changing?
	const uint32_t TOP_BIT = 1U << 31;
	uint32_t expected = __atomic_load_n(&lock->mutex, __ATOMIC_ACQUIRE) & ~TOP_BIT;
	uint32_t desired = expected + 1;

	if (__atomic_compare_exchange_n(&lock->mutex, &expected, desired, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE)) return true;
	return false;
}

static inline void wfe_mutex_rwlock_unlock(wfe_mutex_rwlock *lock) {
	sanity_check_rdwrlock_mutex(&lock->mutex);
	sanity_check_rdwrlock_unlock_mutex(&lock->mutex);

	// Unlocking is just storing zero.
	__atomic_store_n(&lock->mutex, 0, __ATOMIC_RELEASE);
}

static inline void wfe_mutex_rwlock_read_unlock(wfe_mutex_rwlock *lock) {
	sanity_check_rdwrlock_mutex(&lock->mutex);
	sanity_check_rdwrlock_unlock_shared_mutex(&lock->mutex);

	// Unlocked shared is just decrementing 1.
	__atomic_fetch_sub(&lock->mutex, 1, __ATOMIC_ACQUIRE);
}
