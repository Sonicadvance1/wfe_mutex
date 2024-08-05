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

## Spurious wake-up benchmark

Microbenchmark waits for a value to change using the waitx, pkgwait, or wfe instructions and times how long it waits before spuriously waking up even
with the value not changing.

Having the average closer to the maximum is better in this instance, and having a higher maximum means less spurious wake-ups.

| Device | Cycle-Counter frequency | Min | Max | Average |
| - | - | - | - | - |
| AMD Threadripper Pro 5995WX | 2.7Ghz | 29646 | 2786076 | 2694717.002 |
| Cortex-X1C | 19.2Mhz | 0 | 2048 | 595.3532 |
| Cortex-X4 | 19.2Mhz | 0 | 2121 | 1680.7252 |
| Apple M1 (Parallels VM) | 24Mhz | 0 | 8460 | 36.331 |
| Oryon-1 | 19.2Mhz | 0 | 3 | 0.3712 |
