# What is this library for?
This is a useful library for x86, x86-64, AArch32, and AArch64 for a /simple/ interface for a spinloop implementation that uses less power than a
basic atomic memory access spin. This uses the new extensions that these instruction sets offer if possible, or fall back to a spin loop if necessary.

This library is assuming that you are already using spin-locks in your application, and you want to make them slightly nicer to mobile devices, but
you don't want to pay the cost of using OS provided futex/mutex constructs. This can be a building block for additional implementations on top of
this.

**Depending on hardware features this library can take advantage of multiple extensions:**
- waitpkg on latest Intel CPUs
- monitorx on latest AMD Zen CPUs
- WFE and WFET on latest ARM CPUs
- Spinloop based fallback if none of these are available

# Building
When not building unittests, the only requirement is a new enough compiler for C17. The build script will build a static library by default.

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -G Ninja
ninja
sudo ninja install
```

Including in another project is as simple as linking against libwfe_mutex.a.

# Example usage
```cpp
#include <wfe_mutex/wfe_mutex.h> 

int main() {
    wfe_mutex_init();
    wfe_mutex_lock lock = WFE_MUTEX_LOCK_INITIALIZER;

    // Lock the mutex.
    wfe_mutex_lock_lock(&lock, true);

    // Unlock the mutex.
    wfe_mutex_lock_unlock(&lock);
    return 0;
}
```

# What constructs are provided in this library?
This library provides two primary constructs:
- `wfe_mutex_lock` - A mutex object that can only ever have one "unique" or "writer" at a time.
- `wfe_mutex_rwlock` - A mutex object that can have one "writer" or multiple "readers" at a time, never both.

These objects directly correlate to their equivalent pthreads or c++ versions.

Additionally there are two exported symbols, while other implementations all live in the header.
- `wfe_mutex_init()` - Initializes the library. Call before using this library otherwise only spin-locks are used.
- `wfe_mutex_get_features()` returns the internal initialized structure for information purposes.
  - Usually used by inline header functions, but exposes some useful information.

With the two primary mutex objects there are then multiple inline functions for using them. POSIX doesn't require failed mutexes to "synchronize memory" and
neither do any of these implementations. These only synchronize memory on unlock, be aware that the acquiring side might need a memory barrier still
if sharing data between threads that are not the mutex.

## `wfe_mutex_lock`
- `wfe_mutex_lock_lock` - Locks the mutex. Will spin until lock is achieved.
- `wfe_mutex_lock_unlock` - Unlocks the mutex. Doesn't block.
- `wfe_mutex_lock_timedlock` - Tries to lock the mutex, Spins until acquired or timeout, returning the result.

## `wfe_mutex_rwlock`
This entire mutex type has read-lock priority. This matches default pthread semantics. Meaning if multiple readers are active, the implementation will
prioritize those readers getting additional read-locks, even if writers are waiting. If a lock is currently unlocked and two threads race for read and
write locks, there is no guarantees for which one wins.

- `wfe_mutex_rwlock_rdlock` - Locks the mutex with "read" semantics. Spins until read-lock is acquired.
  - Write locks can cause this to spin indefinitely. Multiple readers will eventually succeed.
- `wfe_mutex_rwlock_wrlock` - Locks the mutex with "write" semantics. Spins until write-lock is acquired.
  - Read locks can cause this to spin indefinitely. Once all readers are unlocked, a single waiting blocked write-lock will continue.
  - Multiple write-lock attempts have no guarantee of fairness.
- `wfe_mutex_rwlock_timedwrlock` - Tries to lock the mutex with "write" semantics. Spins until acquired or timeout, returning the result.
- `wfe_mutex_rwlock_trylock` - Tries to lock the mutex with "write" semantics.
  - If already locked, then returns immediately with failure.
  - Otherwise attempts to lock and returns result.
- `wfe_mutex_rwlock_trylock_shared` - Tries to lock the mutex with "read" semantics.
  - If already write locked, returns failure
  - If already read lock, attempts to acquire read lock and returns result
  - Can spuriously fail (Usually due to mid-futex lock updates).
- `wfe_mutex_rwlock_unlock` - Unlocks mutex currently in "write" lock semantics
  - Needs to match original write lock
- `wfe_mutex_rwlock_read_unlock` - Unlocks mutex currently in "read" lock semantics
  - Only removes one reader from the lock.
  - Multiple readers all need to unlock for the mutex to be "unlocked"

# Additional functions
The additional header functions are provided as a means for building more basic things on top of them, as well as getting used by the wfe_mutex
functions.

- `wfe_mutex_wait_for_value_{timeout_,}{i8,i16,i32,i64}(T *ptr, T value, {uint64_t timeout\,} bool low_power)`
  - Atomically waits for the raw memory location to equal the value provided
  - Optionally a version with a timeout in nanoseconds
- `T wfe_mutex_wait_for_bit_{set,not_set}_{i8,i16,i32,i64}`
  - Atomically waits for the bit in the element of memory to either be set or not set depending
  - Returns the full element value
- `bool wfe_mutex_wait_for_value_spurious_oneshot_{i8,i16,i32,i64}`
 - Tries one iteration of the spin-loop iteration before giving up.
 - Useful for implementing a short back-off implementation that is freestanding, since it only tries once.
 - Backend using waitpkg/monitorx/WFE can still sleep for multiple milliseconds in some instances!

# Caveats?
This library has no safety unlike pthreads and C++ mutex objects. If someone uses the API incorrectly then it can break the underlying mutex object.
Although if the header is compiled with `WFE_MUTEX_DEBUG=1` defined then there is at least /some/ sanity checking for fleshing out simple mistakes.

## Things that aren't protected against
- If a mutex is locked one way and then unlocked another, that's not protected against.
  - A write "unlock" will reset the internal object to zero, negative any other owner's locks.
  - Once the other readers come back, they will decrement zero and underflow the internal uint32_t.
- There is no "ownership" unlike pthread objects, a single threadd can read lock a mutex multiple times, or deadlock itself with double write lock.
  - There's no safety here and that would bloat the implementation
  - No deadlock detection as it requires storing thread-specific information inside the mutex and checking that.
- If the library isn't initialized then it uses spin-loops
  - This may be undesirable in some instances
  - AArch64 always supports the WFE instruction
  - x86 the extensions are relatively new so not all hardware supports it
  - The spinloop implementation will at least try to use the `yield` instruction to be a bit nicer
- The `low_power` option on locks only affects x86 and the spin-loop implementation
  - x86 extensions have a flag to try and lower the CPU state to `C1`, which might not always be respected
  - The spin-loop implementation adds five `yield` instructions per loop iteration
  - ARM's WFE/WFET implementation has no concept of "low power", although the implementation should behave almost like idle
    - Although this is idle at max CPU clocks, since the kernel can't tell if the process is doing work or not
    - This still saves power compared to a spin-lock
  - Some situations this may be comparable to a spin loop
    - VMs typically catch the couple of extension instructions and emulate in the hypervisor, providing little or no gain
    - No way to detect this
- Spinloop with timeout may wait longer than expected
  - The implementation of converting nanoseconds to cycles isn't ideal if the cycle counter is significantly faster than 1Ghz
  - Measured delays are within reason (0.20 milliseconds?) but should be known.
  - If faster is required then see issue #3 about fixing the problem

