---
title: Dynamic Containers
description: Runtime-sized containers
---

# Dynamic Containers

Dynamic containers provide runtime-sized storage with growable capacity.

## DynamicBitset

A dynamically-sized bitset with configurable block storage.

### Synopsis

```cpp
namespace peri {
    template<
        std::unsigned_integral Block = std::size_t,
        typename Allocator = std::allocator<Block>
    >
        requires (!std::same_as<Block, bool>)
    class DynamicBitset;
}
```

### Template Parameters

| Parameter | Description |
|-----------|-------------|
| `Block` | Unsigned integral type for internal storage (default: `std::size_t`) |
| `Allocator` | Allocator type for block storage |

### Member Constants

| Constant | Description |
|----------|-------------|
| `BITS_PER_BLOCK` | Number of bits per block |
| `NOT_VALID` | Sentinel value for invalid positions |

### Member Types

| Type | Definition |
|------|------------|
| `block_type` | `Block` |
| `allocator_type` | `Allocator` |
| `size_type` | `std::size_t` |

### Construction

```cpp
using namespace peri;

// Default (empty)
DynamicBitset<> bits;

// With initial size (all bits zero)
DynamicBitset<> bits(1000);

// With initial size and value
DynamicBitset<> bits(64, 0xFF00FF00ULL);

// From string
DynamicBitset<> bits("10101010");
```

### Capacity Operations

| Method | Description |
|--------|-------------|
| `size()` | Number of bits |
| `num_blocks()` | Number of blocks used |
| `empty()` | Returns `true` if size is 0 |
| `resize(n)` | Resize to `n` bits |
| `resize(n, value)` | Resize to `n` bits, new bits set to `value` |
| `clear()` | Remove all bits |
| `reserve(n)` | Reserve capacity for `n` bits |

### Element Access

| Method | Description |
|--------|-------------|
| `test(pos)` | Test bit at position (throws on out of range) |
| `operator[](pos)` | Proxy reference to bit at position |
| `block(i)` | Access raw block at index `i` |

### Modifiers

| Method | Description |
|--------|-------------|
| `set()` | Set all bits |
| `set(pos)` | Set bit at position |
| `set(pos, value)` | Set bit at position to `value` |
| `reset()` | Clear all bits |
| `reset(pos)` | Clear bit at position |
| `flip()` | Flip all bits |
| `flip(pos)` | Flip bit at position |
| `push_back(value)` | Append a bit |
| `pop_back()` | Remove last bit |

### Search Operations

| Method | Description |
|--------|-------------|
| `find_first()` | Find first set bit |
| `find_next(pos)` | Find next set bit after position |
| `count()` | Count set bits (popcount) |
| `all()` | Test if all bits set |
| `any()` | Test if any bit set |
| `none()` | Test if no bits set |

### Bitwise Operators

| Operator | Description |
|----------|-------------|
| `&`, `&=` | Bitwise AND |
| `\|`, `\|=` | Bitwise OR |
| `^`, `^=` | Bitwise XOR |
| `~` | Bitwise NOT |
| `<<`, `<<=` | Left shift |
| `>>`, `>>=` | Right shift |
| `-`, `-=` | Set difference (AND NOT) |

### Example

```cpp
import synodic.periapsis;

using namespace peri;

// Create a bitset for tracking many flags
DynamicBitset<> flags(10000);

// Set some flags
flags.set(42);
flags.set(1000);
flags.set(9999);

// Test flags
if (flags.test(42)) {
    // Flag 42 is set
}

// Count set flags
auto active = flags.count();  // 3

// Iterate over set flags
for (auto pos = flags.find_first();
     pos != DynamicBitset<>::NOT_VALID;
     pos = flags.find_next(pos)) {
    process_flag(pos);
}

// Bitwise operations
DynamicBitset<> mask(10000);
mask.set(42);
mask.set(100);

auto intersection = flags & mask;   // Only position 42
auto union_set = flags | mask;      // 42, 100, 1000, 9999
auto difference = flags - mask;     // 1000, 9999
```

### Proxy Reference

The `operator[]` returns a proxy object that allows assignment:

```cpp
DynamicBitset<> bits(64);

bits[10] = true;   // Set bit 10
bits[10] = false;  // Clear bit 10
bits[10].flip();   // Flip bit 10

bool val = bits[10];  // Read bit 10
```

### Block-Level Access

For performance-critical code, access the underlying blocks directly:

```cpp
DynamicBitset<> bits(1024);
bits.set();  // Set all bits

// Process blocks directly
for (std::size_t i = 0; i < bits.num_blocks(); ++i) {
    auto block = bits.block(i);
    auto popcount = std::popcount(block);
    // ...
}
```

### Comparison with std::vector<bool>

| Feature | `DynamicBitset` | `std::vector<bool>` |
|---------|-----------------|---------------------|
| Bitwise operators | :material-check: | :material-close: |
| Find first/next | :material-check: | :material-close: |
| Block access | :material-check: | :material-close: |
| Popcount | :material-check: | :material-close: |
| Set difference | :material-check: | :material-close: |
| Configurable block | :material-check: | :material-close: |
| Iterator | :material-close: | :material-check: |

`DynamicBitset` is designed for bitset operations, while `std::vector<bool>` is a general-purpose container with unfortunate specialization.
