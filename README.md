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
#### TODO
### Cortex-A78AE - Nvidia Orin
#### TODO

## Spurious wake-up benchmark - microbench_spuriouswakeup

Microbenchmark waits for a value to change using the waitx, waitpkg, or wfe instructions and times how long it waits before spuriously waking up even
with the value not changing.

How to read these numbers
- Having the average closer to the maximum is better in this instance.
- Having a higher maximum means each wait between spurious wake-ups lasts longer
- Having a higher minimum means less likely to have spurious short waits
- Numbers are in cycles

| Device | Cycle-Counter frequency | Min | Max | Average |
| - | - | - | - | - |
| AMD Threadripper Pro 5995WX | 2.7Ghz | 29646 | 2786076 | 2694717.002 |
| Cortex-A78AE | 31.25Mhz | 0 | 4137 | 3844.195200 |
| Cortex-X1C | 19.2Mhz | 0 | 2048 | 595.3532 |
| Cortex-X4 | 19.2Mhz | 0 | 2121 | 1680.7252 |
| Apple M1 (Parallels VM) | 24Mhz | 0 | 8460 | 36.331 |
| Oryon-1 | 19.2Mhz | 0 | 3 | 0.3712 |

## Wake-up latency benchmark - microbench_latency

Microbenchmark measures how long it takes for one thread to unlock a wfe_mutex, until another thread is able to be notified and gains the lock.
- Cross-cluster atomics can affect performance
- Cacheline-thrashing on the spinloop atomic can affect performance
- SMT can affect the performance on x86 machines

How to read these numbers
- Lower average means the mutex wakes up the other thread in less time
- Higher average means that the thread was slower to reach
- Ideally the spurious wake-up number from the previous bench is high and this number is low, for best performance
- Numbers are in cycles
- ARM WFE doesn't support **Low-Power** tests, this is a feature of x86 monitorx and waitpkg extensions only
- Large excursions on the **Max** number will usually be from context switching overhead
  - Should be a minimal impact to the average due to many samples

### Two X1C cores, same cluster
| Device | Cycle-Counter frequency | Test | Min | Max | Average |
| - | - | - | - | - | - |
| Cortex-X1C | 19.2Mhz | **spinloop_rw_unique** | 4 | 6 | 5.61 |
| Cortex-X1C | 19.2Mhz | **spinloop_rw_shared** | 3 | 7 | 5.68 |
| Cortex-X1C | 19.2Mhz | **spinloop_mutex_unique** | 2 | 8 | 5.55 |
| Cortex-X1C | 19.2Mhz | **monitor_rw_unique** | 2 | 5 | 3.32 |
| Cortex-X1C | 19.2Mhz | **monitor_rw_shared** | 2 | 5 | 3.44 |
| Cortex-X1C | 19.2Mhz | **monitor_mutex_unique** | 3 | 8 | 3.44 |
| Cortex-X1C | 19.2Mhz | **pthread_rw_shared** | 116 | 3980 | 3545.87 |
| Cortex-X1C | 19.2Mhz | **pthread_mutex_unique** | 3466 | 4661 | 3657.90 |
| Cortex-X1C | 19.2Mhz | **futex_wakeup** | 3550 | 4281 | 3761.22 |

### Two Oryon-1 cores, same cluster
**Due to Oryon-1 WFE behaviour, monitor implementation might behave like spinloops!**

| Device | Cycle-Counter frequency | Test | Min | Max | Average |
| - | - | - | - | - | - |
| Oryon-1 | 19.2Mhz | **spinloop_rw_unique** | 1 | 4 | 2.38 |
| Oryon-1 | 19.2Mhz | **spinloop_rw_shared** | 1 | 4 | 2.35 |
| Oryon-1 | 19.2Mhz | **spinloop_mutex_unique** | 1 | 4 | 2.36 |
| Oryon-1 | 19.2Mhz | **monitor_rw_unique** | 1 | 4 | 2.37 |
| Oryon-1 | 19.2Mhz | **monitor_rw_shared** | 1 | 4 | 2.35 |
| Oryon-1 | 19.2Mhz | **monitor_mutex_unique** | 1 | 4 | 2.55 |
| Oryon-1 | 19.2Mhz | **pthread_rw_shared** | 88 | 1773 | 986.66 |
| Oryon-1 | 19.2Mhz | **pthread_mutex_unique** | 130| 1655 | 1131.15 |
| Oryon-1 | 19.2Mhz | **futex_wakeup** | 135 | 7188 | 1004.58 |

### AMD Zen-3, Threadripper 5995WX, Same cluster
- Observation: Low-Power monitor implementation has negligible impact on results
  - Low-power spin-loop is just yield inserted between atomic fetches

| Device | Cycle-Counter frequency | Test | Min | Max | Average |
| - | - | - | - | - | - |
| Zen-3 | 2.7Ghz | **spinloop_rw_unique** | 189 | 324 | 244.08 |
| Zen-3 | 2.7Ghz | **spinloop_rw_shared** | 189 | 3375 | 269.19 |
| Zen-3 | 2.7Ghz | **spinloop_mutex_unique** | 216 | 270 | 242.46 |
| Zen-3 | 2.7Ghz | **monitor_rw_unique** | 1539 | 2214 | 2024 |
| Zen-3 | 2.7Ghz | **monitor_rw_shared** | 1539 | 16929 | 1990.98 |
| Zen-3 | 2.7Ghz | **monitor_mutex_unique** | 1836 | 59130 | 2434.32 |
| Zen-3 | 2.7Ghz | **spinloop_rw_unique_lp** | 216 | 459 | 340.20 |
| Zen-3 | 2.7Ghz | **spinloop_rw_shared_lp** | 216 | 459 | 335.61 |
| Zen-3 | 2.7Ghz | **spinloop_mutex_unique_lp** | 243 | 459 | 341.28 |
| Zen-3 | 2.7Ghz | **monitor_rw_unique_lp** | 1566 | 34479 | 2170 |
| Zen-3 | 2.7Ghz | **monitor_rw_shared_lp** | 1539 | 1890 | 1858.14 |
| Zen-3 | 2.7Ghz | **monitor_mutex_unique_lp** | 1836 | 1917 | 1866.24 |
| Zen-3 | 2.7Ghz | **pthread_rw_shared** | 48843 | 1204983 | 165546.99 |
| Zen-3 | 2.7Ghz | **pthread_mutex_unique** | 39663 | 992493 | 162045.36 |
| Zen-3 | 2.7Ghz | **futex_wakeup** | 30105 | 1097577 | 174408.93 |

## Wake-up timeout tardiness benchmark - microbench_tardiness
Microbenchmark tests that when trying to lock a mutex with a timeout, how late it is to return. The "tardiness" of the timeout before returning to the
application code.

How to read these numbers
- The closer to zero, the better the implementation is at returning in a timely manner
- The number is in **NANOSECONDS**

**There is currently a bug in the nanosecond to cycle conversion routine which has precision loss in the number of cycles to wait.**
- This results in cycle counters that don't evenly divide the number of nanoseconds or further away from 1Ghz having larger tardiness
- 1Ghz cycle counter systems shouldn't have this issue
- Avoid timeout based futexes if this matters. Issue #3 is tracking this.

| Device | Test | Min | Max | Average |
| - | - | - | - | - |
| Cortex-X1C | **spinloop_mutex** | 91 | 195 | 129.70 |
| Cortex-X1C | **monitor_mutex_unique** | 3083874 | 3182725 | 3146788.10 |
| Cortex-X1C | **pthread_mutex** | 266977 | 899910 | 746110.60 |
| Cortex-X1C | **futex_wakeup** | 41951 | 881222 | 531128.00 |

| Device | Test | Min | Max | Average |
| - | - | - | - | - |
| Oryon-1 | **spinloop_mutex** | 2 | 158 | 85.00 |
| Oryon-1 | **monitor_mutex_unique** | 3211830 | 3212664 | 3211929.80 |
| Oryon-1 | **pthread_mutex** | 84379 | 2171521 | 1342300.20 |
| Oryon-1 | **futex_wakeup** | 87246 | 2175950 | 940680.20 |

| Device | Test | Min | Max | Average |
| - | - | - | - | - |
| Zen-3 | **spinloop_mutex** | 57 | 99 | 76.20 |
| Zen-3 | **monitor_mutex_unique** | 431519 | 433076 | 431877.70 |
| Zen-3 | **monitor_mutex_unique_lp** | 431205 | 433554 | 431765.00 |
| Zen-3 | **pthread_mutex** | 64696 | 100524 | 85709.10 |
| Zen-3 | **futex_wakeup** | 87459 | 339066 | 140959.50 |
