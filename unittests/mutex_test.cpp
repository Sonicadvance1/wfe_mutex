#define WFE_MUTEX_DEBUG 1
#include <catch2/catch_all.hpp>
#include <wfe_mutex/wfe_mutex.h>
#include <sys/wait.h>

TEST_CASE("Basic Test") {
	wfe_mutex_init();
	wfe_mutex_rwlock lock = WFE_MUTEX_RWLOCK_INITIALIZER;

	// write lock
	wfe_mutex_rwlock_wrlock(&lock, false);
	wfe_mutex_rwlock_unlock(&lock);

	// read lock
	wfe_mutex_rwlock_rdlock(&lock, false);
	wfe_mutex_rwlock_read_unlock(&lock);
}


TEST_CASE("Basic Test - wfe_mutex_lock") {
	wfe_mutex_init();
	wfe_mutex_lock lock = WFE_MUTEX_LOCK_INITIALIZER;

	// lock + unlock
	wfe_mutex_lock_lock(&lock, false);
	wfe_mutex_lock_unlock(&lock);

	// Lock + try_lock + unlock
	wfe_mutex_lock_lock(&lock, false);
	REQUIRE(wfe_mutex_lock_trylock(&lock) == false);
	wfe_mutex_lock_unlock(&lock);

	// timed_lock + unlock
	const uint64_t MAX_NANOSECONDS = 1000000000ULL;
	REQUIRE(wfe_mutex_lock_timedlock(&lock, MAX_NANOSECONDS, false) == true);
	wfe_mutex_lock_unlock(&lock);

	// lock + timed_lock + unlock
	wfe_mutex_lock_lock(&lock, false);
	REQUIRE(wfe_mutex_lock_timedlock(&lock, MAX_NANOSECONDS, false) == false);
	wfe_mutex_lock_unlock(&lock);
}

template<typename F>
int CheckIfExitsWithSignal(F&& func) {
	if (fork() == 0) {
		// Mask signals so fork can't catch the fault.
		sigset_t set;
		sigset_t oldset;
		sigfillset(&set);
		sigprocmask(SIG_SETMASK, &set, &oldset);
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
	wfe_mutex_init();

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
