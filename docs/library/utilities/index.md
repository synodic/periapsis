---
title: Utilities
description: Miscellaneous utilities and helpers
---

# Utilities

Miscellaneous utility functions and types.

## nth_element

A compile-time function to extract the Nth element from a parameter pack.

### Synopsis

```cpp
namespace peri {
    template<std::size_t N>
    constexpr decltype(auto) nth_element(auto&&... args)
        requires (N < sizeof...(args));
}
```

### Template Parameters

| Parameter | Description |
|-----------|-------------|
| `N` | Zero-based index of the element to extract |

### Requirements

- `N < sizeof...(args)` — Index must be valid

### Example

```cpp
import synodic.periapsis;

using namespace peri;

// Extract element at index N
auto second = nth_element<1>(10, 20, 30, 40);  // 20
auto fourth = nth_element<3>(10, 20, 30, 40);  // 40

// Works with any types
auto str = nth_element<0>("hello", 42, 3.14);  // "hello"

// Compile-time evaluation
constexpr auto val = nth_element<2>(1, 2, 3, 4, 5);
static_assert(val == 3);
```

### Use Cases

- Template metaprogramming with variadic templates
- Selecting specific arguments in generic code
- Index-based access to parameter packs

### Implementation

Uses `std::forward_as_tuple` and `std::get` for zero-overhead extraction:

```cpp
template<std::size_t N>
constexpr decltype(auto) nth_element(auto&&... args)
    requires (N < sizeof...(args))
{
    return std::get<N>(std::forward_as_tuple(
        std::forward<decltype(args)>(args)...));
}
```

---

## NotImplementedException

An exception type for marking unimplemented functionality during development.

### Synopsis

```cpp
namespace peri {
    class NotImplementedException : public std::logic_error;
}
```

### Construction

```cpp
// Default message
throw NotImplementedException();
// what() = "Function is not yet implemented"

// Custom message
throw NotImplementedException("Feature X not implemented");
```

### Example

```cpp
import synodic.periapsis;

using namespace peri;

void future_feature() {
    throw NotImplementedException();
}

try {
    future_feature();
} catch (const NotImplementedException& e) {
    std::cerr << e.what() << std::endl;
}
```

### Use Cases

- Mark stub implementations during development
- Indicate unfinished code paths
- Placeholder for future features

### Inheritance

```
std::exception
└── std::logic_error
    └── NotImplementedException
```

---

## Hash Utilities

### hash_combine

Combine multiple hash values into one:

```cpp
namespace peri {
    template<typename T>
    constexpr void hash_combine(std::size_t& seed, const T& value);
}
```

### Example

```cpp
import synodic.periapsis;

using namespace peri;

struct Point {
    int x, y;
};

// Custom hash function
struct PointHash {
    std::size_t operator()(const Point& p) const {
        std::size_t seed = 0;
        hash_combine(seed, p.x);
        hash_combine(seed, p.y);
        return seed;
    }
};

// Use with unordered containers
std::unordered_set<Point, PointHash> points;
```

### Algorithm

Uses the boost::hash_combine formula:

```cpp
template<typename T>
constexpr void hash_combine(std::size_t& seed, const T& value) {
    seed ^= std::hash<T>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
```

The magic constant `0x9e3779b9` is derived from the golden ratio and provides good bit mixing.
