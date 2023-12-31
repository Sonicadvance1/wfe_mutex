## wfe_mutex

This is a simple library that implements spin-loops based on using slightly more efficient means.

## Support table

ARMv8: WFE and WFET

x86-64: AMD's Monitorx extension

Fallback: Spin-loop fallback implementation. Check `wfe_mutex_get_features()` results to see if on fallback path
  - Necessary because monitorx and waitpkg is relatively new
	- Monitorx supported on Zen and above
	- waitpkg supported on Alder Lake(12th gen Golden Code) and above.

## TODO
- Implement support for Intel's waitpkg extension
- Handle higher precision cycle counters like the 1Ghz+ ones on new ARM CPUs
- Implement C++ mutexes
  - Both shared mutexes and unique mutexes
