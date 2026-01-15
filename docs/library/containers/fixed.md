---
title: Fixed-Capacity Containers
description: Constexpr-capable containers with compile-time capacity
---

# Fixed-Capacity Containers

Fixed-capacity containers provide zero-allocation, constexpr-capable storage with compile-time capacity bounds.

## FixedRingBuffer

A circular buffer (ring buffer) with fixed capacity. Provides O(1) operations at both ends.

### Synopsis

```cpp
namespace peri {
    template<typename T, std::size_t Capacity>
        requires (Capacity > 0)
    class FixedRingBuffer;
}
```

### Template Parameters

| Parameter | Description |
|-----------|-------------|
| `T` | Element type |
| `Capacity` | Maximum number of elements (must be > 0) |

### Member Types

| Type | Definition |
|------|------------|
| `value_type` | `T` |
| `size_type` | `std::size_t` |
| `difference_type` | `std::ptrdiff_t` |
| `reference` | `T&` |
| `const_reference` | `const T&` |
| `pointer` | `T*` |
| `const_pointer` | `const T*` |

### Construction

```cpp
// Default construction
constexpr FixedRingBuffer<int, 64> buffer;

// Initializer list
constexpr FixedRingBuffer<int, 8> buffer = {1, 2, 3, 4};
```

### Capacity Operations

| Method | Complexity | Description |
|--------|------------|-------------|
| `empty()` | O(1) | Returns `true` if empty |
| `size()` | O(1) | Current number of elements |
| `max_size()` | O(1) | Returns `Capacity` |
| `capacity()` | O(1) | Returns `Capacity` |
| `full()` | O(1) | Returns `true` if at capacity |

### Element Access

| Method | Complexity | Description |
|--------|------------|-------------|
| `front()` | O(1) | Access first element |
| `back()` | O(1) | Access last element |
| `operator[](n)` | O(1) | Access element at position `n` |

### Modifiers

| Method | Complexity | Description |
|--------|------------|-------------|
| `push_back(value)` | O(1) | Add element to back |
| `push_front(value)` | O(1) | Add element to front |
| `emplace_back(args...)` | O(1) | Construct element at back |
| `emplace_front(args...)` | O(1) | Construct element at front |
| `pop_back()` | O(1) | Remove last element |
| `pop_front()` | O(1) | Remove first element |
| `clear()` | O(n) | Remove all elements |

### Example

```cpp
import synodic.periapsis;

using namespace peri;

// Basic usage
FixedRingBuffer<int, 64> buffer;
buffer.push_back(1);
buffer.push_back(2);
buffer.push_front(0);

int front = buffer.front();  // 0
int back = buffer.back();    // 2

// Compile-time usage
constexpr auto sum = [] constexpr {
    FixedRingBuffer<int, 8> buf;
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);

    int total = 0;
    while (!buf.empty()) {
        total += buf.front();
        buf.pop_front();
    }
    return total;
}();

static_assert(sum == 6);
```

---

## FixedQueue / FixedStack

Type aliases for using `FixedRingBuffer` as a `std::queue` or `std::stack` backend.

### Synopsis

```cpp
namespace peri {
    template<typename T, std::size_t Capacity>
    using FixedQueue = std::queue<T, FixedRingBuffer<T, Capacity>>;

    template<typename T, std::size_t Capacity>
    using FixedStack = std::stack<T, FixedRingBuffer<T, Capacity>>;
}
```

### Example

```cpp
import synodic.periapsis;

using namespace peri;

// Queue with fixed capacity
FixedQueue<int, 64> queue;
queue.push(1);
queue.push(2);
auto front = queue.front();  // 1
queue.pop();

// Stack with fixed capacity
FixedStack<int, 64> stack;
stack.push(1);
stack.push(2);
auto top = stack.top();  // 2
stack.pop();

// Constexpr queue
constexpr auto result = [] constexpr {
    FixedQueue<int, 8> q;
    q.push(10);
    q.push(20);
    return q.front();
}();

static_assert(result == 10);
```

---

## fixed::Bitset

A fixed-size bitset with configurable block type and direct block access.

### Synopsis

```cpp
namespace peri::fixed {
    template<std::size_t N, std::unsigned_integral Block = std::size_t>
    class Bitset;
}
```

### Template Parameters

| Parameter | Description |
|-----------|-------------|
| `N` | Number of bits |
| `Block` | Unsigned integral type for internal storage (default: `std::size_t`) |

### Member Constants

| Constant | Description |
|----------|-------------|
| `BITS_PER_BLOCK` | Number of bits per block |
| `NUM_BLOCKS` | Number of blocks used |
| `NOT_VALID` | Sentinel value for invalid positions |

### Construction

```cpp
// Default (all bits zero)
constexpr peri::fixed::Bitset<64> bits;

// From unsigned long long
constexpr peri::fixed::Bitset<64> bits(0xFF00FF00);
```

### Element Access

| Method | Description |
|--------|-------------|
| `test(pos)` | Test bit at position (throws on out of range) |
| `operator[](pos)` | Test bit at position (no bounds check) |
| `block(i)` | Access raw block at index `i` |

### Bit Operations

| Method | Description |
|--------|-------------|
| `set()` | Set all bits |
| `set(pos)` | Set bit at position |
| `reset()` | Clear all bits |
| `reset(pos)` | Clear bit at position |
| `flip()` | Flip all bits |
| `flip(pos)` | Flip bit at position |

### Search Operations

| Method | Description |
|--------|-------------|
| `find_first()` | Find first set bit |
| `find_next(pos)` | Find next set bit after position |
| `count()` | Count set bits (popcount) |
| `all()` | Test if all bits set |
| `any()` | Test if any bit set |
| `none()` | Test if no bits set |

### Example

```cpp
import synodic.periapsis;

using namespace peri::fixed;

constexpr auto test_bits() {
    Bitset<64> bits;
    bits.set(0);
    bits.set(5);
    bits.set(10);

    auto first = bits.find_first();     // 0
    auto next = bits.find_next(first);  // 5
    auto count = bits.count();          // 3

    return count;
}

static_assert(test_bits() == 3);
```

### Block Access

Direct block access enables efficient bulk operations:

```cpp
peri::fixed::Bitset<256> bits;

// Set entire block at once
for (std::size_t i = 0; i < bits.num_blocks(); ++i) {
    auto block = bits.block(i);
    // Process block...
}
```
