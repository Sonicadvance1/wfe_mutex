#pragma once
#include <wfe_mutex/wfe_mutex.h>

#ifdef __cplusplus
namespace wfe_mutex {
	template<bool low_power>
	class mutex final {
		public:
			constexpr mutex() noexcept {}
			mutex (const mutex&) = delete;

			using native_handle_type = wfe_mutex_lock;

			void lock() {
				wfe_mutex_lock_lock(&mut, low_power);
			}

			void unlock() {
				wfe_mutex_lock_unlock(&mut);
			}

			bool try_lock() {
				return wfe_mutex_lock_trylock(&mut);
			}

			native_handle_type& native_handle() {
				return mut;
			}

		private:
			native_handle_type mut = WFE_MUTEX_LOCK_INITIALIZER;
	};

	template<bool low_power>
	class shared_mutex final {
		public:
			using native_handle_type = wfe_mutex_rwlock;
			void lock() {
				wfe_mutex_rwlock_wrlock(&mut, low_power);
			}

			bool try_lock() {
				return wfe_mutex_rwlock_trylock(&mut);
			}

			void unlock() {
				wfe_mutex_rwlock_unlock(&mut);
			}

			void lock_shared() {
				wfe_mutex_rwlock_rdlock(&mut, low_power);
			}

			bool try_lock_shared() {
				return wfe_mutex_rwlock_trylock_shared(&mut);
			}

			void unlock_shared() {
				wfe_mutex_rwlock_read_unlock(&mut);
			}

			native_handle_type& native_handle() {
				return mut;
			}

		private:
			native_handle_type mut = WFE_MUTEX_RWLOCK_INITIALIZER;
	};
}

#endif
