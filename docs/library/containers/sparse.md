---
title: Sparse Containers
description: Efficient storage for sparse data patterns
---

# Sparse Containers

Sparse containers are optimized for scenarios where data is sparsely distributed across a large index space.

## BitVector

A sparse bit vector combining a bitset with a Fenwick tree for O(log n) rank and select operations.

### Synopsis

```cpp
namespace peri::sparse {
    template<typename Bitset, typename FenwickContainer>
    class BitVectorBase;

    // Fixed-size alias
    template<std::size_t N, std::unsigned_integral Block = std::size_t>
    using BitVector = BitVectorBase<
        peri::fixed::Bitset<N, Block>,
        std::array<std::size_t, /* block count */>
    >;

    // Dynamic alias
    template<std::unsigned_integral Block = std::size_t>
    using DynamicBitVector = BitVectorBase<
        peri::DynamicBitset<Block>,
        std::vector<std::size_t>
    >;
}
```

### Key Operations

| Operation | Complexity | Description |
|-----------|------------|-------------|
| `set(pos)` | O(log n) | Set bit and update Fenwick tree |
| `reset(pos)` | O(log n) | Clear bit and update Fenwick tree |
| `test(pos)` | O(1) | Test if bit is set |
| `rank(pos)` | O(log n) | Count set bits in [0, pos) |
| `select(k)` | O(log² n) | Find position of k-th set bit |

### Rank and Select

The rank and select operations are fundamental for succinct data structures:

- **rank(pos)**: How many 1-bits are there before position `pos`?
- **select(k)**: Where is the k-th 1-bit?

```cpp
import synodic.periapsis;

using namespace peri::sparse;

BitVector<1024> bv;
bv.set(10);
bv.set(20);
bv.set(30);

auto r = bv.rank(25);   // 2 (bits 10 and 20 are before 25)
auto s = bv.select(1);  // 10 (first set bit is at position 10)
auto s2 = bv.select(2); // 20 (second set bit is at position 20)
```

### Iteration

Efficiently iterate over set bits:

```cpp
BitVector<1024> bv;
// ... set some bits ...

// Iterate over all set bit positions
for (auto pos : bv) {
    // pos is the index of a set bit
    process(pos);
}

// Manual iteration
for (auto pos = bv.find_first();
     pos != BitVector<1024>::NOT_VALID;
     pos = bv.find_next(pos)) {
    process(pos);
}
```

### Example

```cpp
import synodic.periapsis;

using namespace peri::sparse;

// Fixed-size sparse bit vector
BitVector<4096> bv;

// Set sparse positions
bv.set(100);
bv.set(500);
bv.set(1000);
bv.set(3000);

// Query operations
auto count = bv.count();        // 4
auto rank = bv.rank(600);       // 2 (positions 100 and 500)
auto pos = bv.select(3);        // 1000 (third set bit)

// Use for sparse set membership with rank
if (bv.test(500)) {
    auto dense_index = bv.rank(500);  // Map sparse -> dense
}
```

---

## IndexVector

A dynamic sparse index set backed by a sorted vector. Provides O(log n) lookup and ordered iteration.

### Synopsis

```cpp
namespace peri::sparse {
    template<
        std::integral Index = std::size_t,
        typename Allocator = std::allocator<Index>
    >
    class IndexVector;
}
```

### Template Parameters

| Parameter | Description |
|-----------|-------------|
| `Index` | Integral type for indices |
| `Allocator` | Allocator for the underlying vector |

### Member Types

| Type | Definition |
|------|------------|
| `value_type` | `Index` |
| `size_type` | `std::size_t` |
| `iterator` | Random access iterator |
| `const_iterator` | Const random access iterator |

### Construction

```cpp
using namespace peri::sparse;

// Default construction
IndexVector<> indices;

// From initializer list
IndexVector<> indices = {10, 20, 30, 40};

// From range
std::vector<int> data = {5, 3, 8, 1};
IndexVector<> indices(data.begin(), data.end());  // Sorted: {1, 3, 5, 8}
```

### Operations

| Method | Complexity | Description |
|--------|------------|-------------|
| `insert(idx)` | O(n) | Insert index (maintains sorted order) |
| `erase(idx)` | O(n) | Remove index |
| `contains(idx)` | O(log n) | Test if index exists |
| `find(idx)` | O(log n) | Find iterator to index |
| `rank(idx)` | O(log n) | Count indices less than `idx` |
| `select(k)` | O(1) | Get k-th smallest index |

### Iteration

Indices are always sorted, enabling efficient ordered traversal:

```cpp
IndexVector<> indices = {30, 10, 20, 40};

// Iteration is in sorted order
for (auto idx : indices) {
    // 10, 20, 30, 40
}
```

### Example

```cpp
import synodic.periapsis;

using namespace peri::sparse;

// Track active entity IDs
IndexVector<std::uint32_t> active_entities;

// Add entities
active_entities.insert(1000);
active_entities.insert(500);
active_entities.insert(2000);

// Check membership
if (active_entities.contains(500)) {
    // Entity 500 is active
}

// Get rank (useful for dense array indexing)
auto rank = active_entities.rank(500);  // How many entities have smaller IDs?

// Iterate in ID order
for (auto entity_id : active_entities) {
    update_entity(entity_id);
}

// Remove entity
active_entities.erase(500);
```

### Use Cases

- **Sparse-to-dense mapping**: Use `rank()` to convert sparse IDs to dense array indices
- **Ordered iteration**: Process elements in a specific order
- **Set operations**: Membership testing with O(log n) lookup
- **Entity component systems**: Track which entities have a specific component

---

## When to Use Which

| Container | Best For |
|-----------|----------|
| `BitVector` | Fixed range, need rank/select, memory-efficient |
| `DynamicBitVector` | Dynamic range, need rank/select |
| `IndexVector` | Sparse indices, ordered iteration, dense mapping |
