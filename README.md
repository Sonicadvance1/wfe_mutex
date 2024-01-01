## wfe_mutex

This is a simple library that implements spin-loops based on using slightly more efficient means.

## Support table

ARMv8: WFE and WFET

AArch32: WFE only
- 64-bit variables falls back to spinloop
- Only supports AArch32 not ARMv7

x86-64:
- AMD's Monitorx extension
- Intel's waitpkg extension

Fallback: Spin-loop fallback implementation. Check `wfe_mutex_get_features()` results to see if on fallback path
- Necessary because monitorx and waitpkg is relatively new
- Monitorx supported on Zen and above
- waitpkg supported on Alder Lake(12th gen Golden Code) and above.

## Unsupported
ARMv7: Subtle differences to AArch32
- Need to wire up spin-loop implementation

## TODO
- Handle higher precision cycle counters like the 1Ghz+ ones on new ARM CPUs
