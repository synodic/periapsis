---
title: Trees
description: Specialized tree data structures
---

# Tree Structures

Periapsis provides specialized tree data structures optimized for specific query patterns.

## Fenwick Tree

A Fenwick tree (Binary Indexed Tree) for efficient prefix sum queries and point updates.

### Synopsis

```cpp
namespace peri::structure {
    template<typename T, typename Container>
        requires semigroup<T> && decayed<Container> && std::ranges::contiguous_range<Container>
    class Fenwick;
}
```

### Template Parameters

| Parameter | Constraints |
|-----------|-------------|
| `T` | Must satisfy `semigroup` (has associative `+` operation) |
| `Container` | Must be a `contiguous_range` (e.g., `std::vector`, `std::array`, `std::span`) |

### Complexity

| Operation | Complexity |
|-----------|------------|
| Construction | O(n) |
| Point update | O(log n) |
| Prefix sum | O(log n) |
| Range sum | O(log n) |

### Member Types

| Type | Definition |
|------|------------|
| `value_type` | `T` |
| `container_type` | `Container` |
| `size_type` | `std::size_t` |

### Construction

```cpp
using namespace peri::structure;

// Default construction (for fixed-size containers)
Fenwick<int, std::array<int, 100>> tree1;

// With capacity (for dynamic containers)
Fenwick<int, std::vector<int>> tree2(100);

// From existing container
std::vector<int> data(100, 0);
Fenwick<int, std::vector<int>> tree3(std::move(data));

// From span (non-owning)
std::vector<int> buffer(100);
Fenwick<int, std::span<int>> tree4(std::span{buffer});
```

### Operations

| Method | Returns | Description |
|--------|---------|-------------|
| `size()` | `size_type` | Number of elements |
| `empty()` | `bool` | Test if empty |
| `update(i, delta)` | `void` | Add `delta` to element at index `i` |
| `prefix_sum(i)` | `T` | Sum of elements [0, i) |
| `range_sum(i, j)` | `T` | Sum of elements [i, j) |
| `get(i)` | `T` | Get single element value |
| `set(i, value)` | `void` | Set element to value |
| `build(range)` | `void` | Build tree from range |

### Example: Basic Usage

```cpp
import synodic.periapsis;

using namespace peri::structure;

// Create Fenwick tree with 10 elements
Fenwick<int, std::vector<int>> tree(10);

// Point updates
tree.update(0, 5);   // Element 0 += 5
tree.update(3, 10);  // Element 3 += 10
tree.update(7, 3);   // Element 7 += 3

// Prefix queries
auto sum_0_4 = tree.prefix_sum(4);   // Sum of [0, 4) = 15
auto sum_0_8 = tree.prefix_sum(8);   // Sum of [0, 8) = 18

// Range query
auto sum_3_7 = tree.range_sum(3, 7); // Sum of [3, 7) = 10
```

### Example: Constexpr Usage

```cpp
import synodic.periapsis;

using namespace peri::structure;

constexpr auto compute_prefix_sum() {
    Fenwick<int, std::array<int, 8>> tree;

    tree.update(0, 1);
    tree.update(1, 2);
    tree.update(2, 3);
    tree.update(3, 4);

    return tree.prefix_sum(4);  // 1 + 2 + 3 + 4 = 10
}

static_assert(compute_prefix_sum() == 10);
```

### Example: Non-Owning with Span

For cases where you want the Fenwick tree to operate on external memory:

```cpp
import synodic.periapsis;

using namespace peri::structure;

void process_data(std::span<int> data) {
    // Create Fenwick tree over existing data
    Fenwick<int, std::span<int>> tree(data);

    // Build tree from current values
    tree.build(data);

    // Queries and updates modify the original data
    auto sum = tree.prefix_sum(data.size());
}
```

### Use Cases

| Use Case | Description |
|----------|-------------|
| Running totals | Efficient cumulative sums with updates |
| Range frequency | Count elements in a range |
| Inversion counting | Count inversions in an array |
| 2D prefix sums | Extend to 2D with nested trees |
| Order statistics | Combine with binary search |

### Algorithm Details

The Fenwick tree uses a clever indexing scheme based on the least significant bit:

- Each index `i` stores the sum of a range determined by `i & -i`
- Updates and queries follow paths determined by bit manipulation
- Space efficient: uses same memory as the original array

```
Index:  1   2   3   4   5   6   7   8
        │   │   │   │   │   │   │   │
        │   ├───┤   │   ├───┤   │   │
        │   └───────┤   └───────┤   │
        └───────────────────────────┤
```

### Requirements

The element type `T` must satisfy the `semigroup` concept:

```cpp
template<typename T>
concept semigroup = requires(T a, T b) {
    { a + b } -> std::same_as<T>;  // Associative operation
};
```

This works with numeric types, strings (concatenation), and custom types with `operator+`.
