#include <chrono>
#include <wfe_mutex/wfe_mutex.hpp>

#include <atomic>
#include <sys/mman.h>
#include <stdio.h>
#include <thread>

#if defined(_M_ARM_64)
static uint64_t read_cycle_counter() {
	uint64_t result;
	__asm volatile(
		"isb;"
		"mrs %[Res], CNTVCT_EL0;"
		: [Res] "=r" (result));
	return result;
}
#elif defined(_M_ARM_32)
static uint64_t read_cycle_counter() {
	uint32_t result_low, result_high;

	// Read cntvct
	__asm volatile(
		"isb;"
		"mrrc p15, 1, %[Res_Lower], %[Res_Upper], c14;"
		: [Res_Lower] "=r" (result_low)
		, [Res_Upper] "=r" (result_high));
	uint64_t result = result_high;
	result <<= 32;
	result |= result_low;
	return result;
}
#elif defined(_M_X86_64)
uint64_t read_cycle_counter() {
	return __rdtsc();
}
#elif defined(_M_X86_32)
uint64_t read_cycle_counter() {
	uint32_t high, low;

	__asm volatile(
	"rdtsc;"
	: "=a" (low)
	, "=d" (high)
	:: "memory");

	uint64_t result = high;
	result <<= 32;
	result |= low;
	return result;
}
#else
uint64_t read_cycle_counter() {
	return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}
#endif


void Thread_wfe(uint8_t *Data, std::atomic<uint8_t> *Ready, std::atomic<uint8_t> *Done, std::atomic<uint64_t>* Time) {
	while (Ready->load() == 0);

	wfe_mutex_wait_for_value_i8(Data, 1, false);
	*Time = read_cycle_counter();
	*Done = 1;
}

void Thread_spin(uint8_t *Data, std::atomic<uint8_t> *Ready, std::atomic<uint8_t> *Done, std::atomic<uint64_t>* Time) {
	auto DataAtomic = reinterpret_cast<std::atomic<uint8_t>*>(Data);
	while (Ready->load() == 0);
	while (DataAtomic->load() == 0);

	*Time = read_cycle_counter();
	*Done = 1;
}

void DoTest_wfe() {
	void *Ptr = mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	uint8_t *Data = reinterpret_cast<uint8_t*>(Ptr);
	auto DataAtomic = reinterpret_cast<std::atomic<uint8_t>*>(Data + 1024 * 0);
	auto Ready = reinterpret_cast<std::atomic<uint8_t>*>(Data + 1024 * 1);
	auto Done  = reinterpret_cast<std::atomic<uint8_t>*>(Data + 1024 * 2);
	auto Time  = reinterpret_cast<std::atomic<uint64_t>*>(Data + 1024 * 3);

	std::thread t(Thread_wfe, Data, Ready, Done, Time);

	*Ready = 1;
	// Wait an arbitrary amount of time.
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	// Write to the data value
	DataAtomic->store(1);

	// Wait for the thread to say we're done
	while (Done->load() == 0);

	const auto DoneTime = read_cycle_counter();

	size_t Diff = DoneTime - *Time;
	fprintf(stderr, "cycle difference between threads: %zu\n", Diff);

	t.join();

	munmap(Ptr, 4096);
}

void DoTest_spin() {
	void *Ptr = mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	uint8_t *Data = reinterpret_cast<uint8_t*>(Ptr);
	auto DataAtomic = reinterpret_cast<std::atomic<uint8_t>*>(Data + 1024 * 0);
	auto Ready = reinterpret_cast<std::atomic<uint8_t>*>(Data + 1024 * 1);
	auto Done  = reinterpret_cast<std::atomic<uint8_t>*>(Data + 1024 * 2);
	auto Time  = reinterpret_cast<std::atomic<uint64_t>*>(Data + 1024 * 3);

	std::thread t(Thread_spin, Data, Ready, Done, Time);

	*Ready = 1;
	// Wait an arbitrary amount of time.
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	// Write to the data value
	DataAtomic->store(1);

	// Wait for the thread to say we're done
	while (Done->load() == 0);

	const auto DoneTime = read_cycle_counter();

	size_t Diff = DoneTime - *Time;
	fprintf(stderr, "cycle difference between threads: %zu\n", Diff);

	t.join();

	munmap(Ptr, 4096);
}

extern "C" int cpp_test() {
	wfe_mutex_init();
	DoTest_wfe();

	DoTest_spin();
	return 0;
}
