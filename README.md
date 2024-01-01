## wfe_mutex

This is a simple library that implements spin-loops based on using slightly more efficient means.

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
