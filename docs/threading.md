# Threading

Thread-related utilities (work in progress).

## ThreadLocal

A managed object that exists statically for each thread. Provides a wrapper for thread-local storage with proper lifecycle management.

```cpp
#include <synodic/library>

using namespace peri;

// Thread-local data (API under development)
ThreadLocal<MyData> data;
```

**Status:** Implementation in progress.

## Executor Concepts

Concepts for `std::execution`-compatible schedulers.

```cpp
#include <synodic/library>

using namespace peri::executor;

template<typename T>
concept scheduler = /* ... */;  // Currently placeholder
```

**Status:** Placeholder for future `std::execution` integration.

## Future Plans

- Complete `ThreadLocal` implementation
- `std::execution` scheduler integration
- Work-stealing thread pool
- Task graph execution with graph library integration
