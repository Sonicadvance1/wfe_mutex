## wfe_mutex

This is a simple library that implements spin-loops based on using slightly more efficient means.

### What's the use case?
If your code is already using a spin-loop for more than a few dozen cycles or better yet a few thousand cycles. Then your code can easily eat the
small amount of overhead for an indirect branch that can be more efficient. The primary purpose for this library is to implement spin-loops in a way
that is more efficient on both the memory bus and the power consumption of the CPU. This is handled by using AMD's, Intel's, and ARM's instructions to
make this more efficient.

When the CPU supports the pre-requisite extensions, the spin-loop implementation turns in to an exclusive monitor implementation. This allows the CPU
to not hit the memory bus as much and also lets the CPU enter a lower power state. The CPU can wake up and return from the implementation roughly as
quickly as a spin-loop, when benchmarking the two implementations the spin-loop implementation is still usually faster by a handful of cycles.

If this overhead can be accounted for in your spin-loop implementation then the reduction in power consumption and memory bus is likely worth it.

### Mutexes?
The mutex implementations might be interesting if a mutex where its implementation is based on spin-loops is appealing. In particular the
read-lock/shared mutex is incredibly quick as it is a load+mask+cmpxchg in a loop. The unlock is just a decrement without sanity checking.

This can be incredibly useful with shared_mutex that has a high amount of uncontended  shared locks. Where pthread rdwr_lock read locks can appear in
profiles, this likely just vanishes instead. Additionally since they are POD (since they are just a uint32_t) they can be embedded in data structures.

The unique mutex locking and unlocking is just a CAS.

## Support table

ARMv8: WFE and WFET

AArch32: WFE only
- 64-bit variables falls back to spinloop
- Only supports AArch32 not ARMv7

x86-64:
- AMD's Monitorx extension (Available on Zen and newer)
- Intel's waitpkg extension (Available on Alder Lake/Golden Cove/12th gen and newer)

Fallback: Spin-loop fallback implementation. Check `wfe_mutex_get_features()` results to see if on fallback path
- Necessary because monitorx and waitpkg is relatively new
- low_power flag lets the spinloop execute yield/pause interleaved in the loop.

## Unsupported
ARMv7: Subtle differences to AArch32
- Need to wire up spin-loop implementation

## TODO
- Handle higher precision cycle counters like the 1Ghz+ ones on new ARM CPUs

## Benchmarks
### AMD Zen 3 - AMD Threadripper Pro 5995WX
AMD CPUs are known to have slow wake-up latency on mwaitx.

***Same CPU core, different SMT thread***
- Wake-up latency - spin-lock
   - Around 140 cycles - ~50ns

- Wake-up latency - monitor
   - Around 950 cycles - ~350ns

- Wake-up latency - pthread or raw futex
   - From 14,985 cycles to 70,000 cycles. 5,550ns to 26,000 cycles.
      - These are a mess

***Different Cores on the same die, same L3 cache***
- Wake-up latency - spin-lock
   - Around 200 cycles - ~80ns

- Wake-up latency - monitor
   - Around 1800 cycles - ~666ns

- Wake-up latency - pthread or raw futex
   - From 90,000 cycles to 470,000 cycles. 33,333ns to 174,000ns
      - These are a mess

***Different Cores on the across dies, No shared L3 cache***
- Wake-up latency - spin-lock
   - Around 800 cycles - ~296ns

- Wake-up latency - monitor
   - Around 2300 cycles - ~850ns

- Wake-up latency - pthread or raw futex
   - From 68,823 cycles to 754,731 cycles. 25,490ns to 280,000ns
      - These are a mess

#### Uncontended mutexes per second
***Monitor implementation effectively does nothing here, so should be close to spin-loop implementation***
- rwlock - shared lock - spinloop
   - 268,937,561.56 per second
   - ~51.2% more than pthreads
- rwlock - shared lock - monitor
   - 272,881,076.60 per second
	 - ~53.4% more than pthreads
- rwlock - unique lock - spinloop
   - 274,663,334.72 per second
- rwlock - unique lock - monitor
   - 273,871,268.99 per second
- mutex - unique lock - spinloop
   - 279,664,732.79 per second
	 - ~69.2% more than pthreads
- mutex - unique lock - monitor
   - 259,194,625.92 per second
	 - ~56.8% more than pthreads
- pthread rwlock - shared lock
   - 177,835,064.71 per second
- pthread mutex - unique lock
   - 165,202,522.09 per second

### Cortex-A78AE - Nvidia Orin
These numbers have very tight clustering. Which is probably because ARM's WFE instruction spuriously wakes up after 1-4 cycles, effectively turning
the implementation in to a spin-loop.

***Same CPU cluster***
- Wake-up latency - spin-lock
   - Around 5.3 cycles - ~168 ns

- Wake-up latency - monitor
   - Around 5.0 cycles - ~158 ns

- Wake-up latency - pthread or raw futex
   - Around 400 cycles - ~13,000 ns

***Different CPU cluster***
- Wake-up latency - spin-lock
   - Around 12 cycles - ~380 ns

- Wake-up latency - monitor
   - Around 12.4 cycles - ~390 ns

- Wake-up latency - pthread or raw futex
   - Around 400 cycles - ~13,000 ns

#### Uncontended mutexes per second
***Monitor implementation effectively does nothing here, so should be close to spin-loop implementation***
***Compiled with -mcpu=cortex-a78 to inline atomics***
- rwlock - shared lock - spinloop
   - 71,361,969.97 per second
   - ~60% more than pthreads
- rwlock - shared lock - monitor
   - 71,508,180.30 per second
   - ~60.3% more than pthreads
- rwlock - unique lock - spinloop
   - 89,922,815.13 per second
- rwlock - unique lock - monitor
   - 90,075,452.36 per second
- mutex - unique lock - spinloop
   - 90,023,980.00 per second
	 - ~138.6% more than pthreads
- mutex - unique lock - monitor
   - 90,034,430.93 per second
	 - ~138.6% more than pthreads
- pthread rwlock - shared lock
   - 44,596,095.36 per second
- pthread mutex - unique lock
   - 37,731,536.61 per second
