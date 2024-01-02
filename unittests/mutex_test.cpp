#define WFE_MUTEX_DEBUG 1
#include <catch2/catch_all.hpp>
#include <wfe_mutex/wfe_mutex.h>
#include <sys/wait.h>

TEST_CASE("Basic Test") {
	wfe_mutex_rwlock lock = WFE_MUTEX_RWLOCK_INITIALIZER;

	// write lock
	wfe_mutex_rwlock_wrlock(&lock, false);
	wfe_mutex_rwlock_unlock(&lock);

	// read lock
	wfe_mutex_rwlock_rdlock(&lock, false);
	wfe_mutex_rwlock_read_unlock(&lock);
}

template<typename F>
int CheckIfExitsWithSignal(F&& func) {
	if (fork() == 0) {
		std::forward<F>(func)();
		exit(1);
	}
	else {
		int status{};
		wait(&status);
		return status;
	}
}

TEST_CASE("Invalid unlock") {
	auto status = CheckIfExitsWithSignal([]() {
		wfe_mutex_rwlock lock = WFE_MUTEX_RWLOCK_INITIALIZER;

		// Invalid unlock.
		// Lock as read, unlock as write.
		wfe_mutex_rwlock_rdlock(&lock, false);
		wfe_mutex_rwlock_unlock(&lock);
	});

	CHECK(WIFSIGNALED(status) == true);
	CHECK(WTERMSIG(status) == SIGSEGV);

	status = CheckIfExitsWithSignal([]() {
		wfe_mutex_rwlock lock = WFE_MUTEX_RWLOCK_INITIALIZER;

		// Invalid unlock.
		// Lock as write, unlock as read.
		wfe_mutex_rwlock_wrlock(&lock, false);
		wfe_mutex_rwlock_read_unlock(&lock);
	});

	CHECK(WIFSIGNALED(status) == true);
	CHECK(WTERMSIG(status) == SIGSEGV);

	status = CheckIfExitsWithSignal([]() {
		wfe_mutex_rwlock lock = WFE_MUTEX_RWLOCK_INITIALIZER;

		// Invalid unlock.
		// Lock as write, unlock twice as write.
		wfe_mutex_rwlock_wrlock(&lock, false);
		wfe_mutex_rwlock_unlock(&lock);
		wfe_mutex_rwlock_unlock(&lock);
	});

	CHECK(WIFSIGNALED(status) == true);
	CHECK(WTERMSIG(status) == SIGSEGV);

	status = CheckIfExitsWithSignal([]() {
		wfe_mutex_rwlock lock = WFE_MUTEX_RWLOCK_INITIALIZER;

		// Invalid unlock.
		// Lock as read, unlock twice as read.
		wfe_mutex_rwlock_wrlock(&lock, false);
		wfe_mutex_rwlock_read_unlock(&lock);
		wfe_mutex_rwlock_read_unlock(&lock);
	});

	CHECK(WIFSIGNALED(status) == true);
	CHECK(WTERMSIG(status) == SIGSEGV);


	status = CheckIfExitsWithSignal([]() {
		wfe_mutex_lock lock = WFE_MUTEX_RWLOCK_INITIALIZER;

		// Invalid unlock.
		// Lock, unlock twice.
		wfe_mutex_lock_lock(&lock, false);
		wfe_mutex_lock_unlock(&lock);
		wfe_mutex_lock_unlock(&lock);
	});

	CHECK(WIFSIGNALED(status) == true);
	CHECK(WTERMSIG(status) == SIGSEGV);
}
