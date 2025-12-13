# Utilities

Miscellaneous utility functions and types.

## nth_element

A compile-time function to extract the Nth element from a parameter pack.

```cpp
#include <synodic/library>

using namespace peri;

// Extract element at index N from variadic arguments
auto second = nth_element<1>(10, 20, 30, 40);  // 20
auto fourth = nth_element<3>(10, 20, 30, 40);  // 40

// Works with any types
auto str = nth_element<0>("hello", 42, 3.14);  // "hello"

// Compile-time evaluation
constexpr auto val = nth_element<2>(1, 2, 3, 4, 5);
static_assert(val == 3);
```

**Implementation:**
Uses `std::index_sequence` and a helper struct to select the Nth argument at compile time with zero runtime overhead.

## NotImplementedException

An exception type for marking unimplemented functionality during development.

```cpp
#include <synodic/library>

using namespace peri;

void future_feature() {
    throw NotImplementedException();
    // "Function is not yet implemented"
}

try {
    future_feature();
} catch (const NotImplementedException& e) {
    std::cerr << e.what() << std::endl;
}
```

**Inheritance:** Derives from `std::logic_error`
