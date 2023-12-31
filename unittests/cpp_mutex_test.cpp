#include <wfe_mutex/wfe_mutex.hpp>

#include <mutex>
#include <shared_mutex>

extern "C" int cpp_mutex_test() {
	wfe_mutex::mutex<false> mutex_hi;
	wfe_mutex::mutex<true> mutex_lo;
	wfe_mutex::shared_mutex<false> shared_hi;
	wfe_mutex::shared_mutex<true> shared_lo;
	std::scoped_lock lk {mutex_hi};
	std::scoped_lock lk2 {mutex_lo};

	std::shared_lock lk3 {shared_hi};
	std::shared_lock lk4 {shared_lo};
	return 0;
}

