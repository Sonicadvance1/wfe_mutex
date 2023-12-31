#include <wfe_mutex/wfe_mutex.h>

int mutex_test() {
	wfe_mutex_rwlock lock = WFE_MUTEX_RWLOCK_INITIALIZER;

	// write lock
	wfe_mutex_rwlock_wrlock(&lock, false);
	wfe_mutex_rwlock_unlock(&lock);

	// read lock
	wfe_mutex_rwlock_rdlock(&lock, false);
	wfe_mutex_rwlock_read_unlock(&lock);
	return 0;
}
