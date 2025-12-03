# Containers

The Synodic Template Library provides several specialized container types organized by category.

## Fixed-Capacity Containers

### FixedRingBuffer

A constexpr-capable, fixed-capacity ring buffer (circular buffer) that can be used as the underlying container for `std::queue` and `std::stack`.

```cpp
#include <synodic/library>

using namespace synodic;

// Direct usage
FixedRingBuffer<int, 64> buffer;
buffer.push_back(1);
buffer.push_back(2);
buffer.push_front(0);  // Deque-like operations

int front = buffer.front();  // 0
int back = buffer.back();    // 2
buffer.pop_front();

// As std::queue backend (constexpr-capable!)
FixedQueue<int, 64> queue;
queue.push(42);
auto val = queue.front();
queue.pop();

// As std::stack backend
FixedStack<int, 64> stack;
stack.push(42);
auto top = stack.top();
stack.pop();

// Compile-time usage
constexpr auto sum = []() constexpr {
    FixedRingBuffer<int, 8> buf;
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    return buf.front() + buf.back();
}();
static_assert(sum == 4);
```

**Key Features:**

- Fully `constexpr` - usable at compile-time
- O(1) push/pop at both ends
- Zero heap allocation (uses `std::array` internally)
- Compatible with `std::queue<T, FixedRingBuffer<T, N>>`
- Compatible with `std::stack<T, FixedRingBuffer<T, N>>`

**Type Aliases:**

```cpp
template<typename T, std::size_t Capacity>
using FixedQueue = std::queue<T, FixedRingBuffer<T, Capacity>>;

template<typename T, std::size_t Capacity>
using FixedStack = std::stack<T, FixedRingBuffer<T, Capacity>>;
```

## Sparse Containers

Sparse containers are optimized for scenarios with sparse data access patterns.

### BitMap

A fixed-capacity associative container using a bitset for presence tracking. Faster creation than `std::unordered_set` but with different performance characteristics for intersection operations.

```cpp
#include <synodic/library>

using namespace synodic::sparse;

BitMap<MyData, 64> map;  // 64 slots

auto [iter, inserted] = map.Insert(5, MyData{...});
if (map.Test(5)) {
    MyData& data = map[5];
}
map.Clear();
```

**Template Parameters:**

- `T` - The type to store
- `N` - The capacity (must be byte-aligned, i.e., divisible by 4)

### HashMap

A sparse hash map implementation using open addressing with quadratic probing.

```cpp
#include <synodic/library>

using namespace synodic::sparse;

HashMap<std::string, int> map;
map["key"] = 42;
int value = map.At("key");
map.Emplace("another", 100);
map.Clear();
```

### HashSet

A sparse hash set implementation (work in progress).

```cpp
#include <synodic/library>

using namespace synodic::sparse;

HashSet<int> set;
// API under development
```

### Vector

A sparse vector using block-based storage with bitmap tracking.

```cpp
#include <synodic/library>

using namespace synodic::sparse;

Vector<MyData, 8> vec;  // Block size of 8
vec.Insert(100, MyData{...});  // Insert at index 100
auto iter = vec.Find(100);
```

## Lock-Free Containers

### SPSCQueue

A single-producer, single-consumer lock-free queue adapter. Provides atomic access guarantees for concurrent scenarios with exactly one reader and one writer thread.

```cpp
#include <synodic/library>

using namespace synodic::lockfree;

// Wrap a container (e.g., std::vector with fixed capacity)
std::vector<int> storage(1024);
SPSCQueue queue(std::move(storage));

// Producer thread
queue.push(42);
queue.try_push(43);

// Consumer thread
if (auto value = queue.pop()) {
    process(*value);
}

// Extract the underlying container (single-threaded context only)
auto extracted = queue.extract();
```

**Key Features:**

- Cache-line aligned to avoid false sharing
- Non-blocking `pop()` returns `std::optional`
- Supports container extraction for single-threaded post-processing

**Requirements:**

- Value type must be nothrow constructible and destructible
- Container must satisfy `inplace_constructing_sequence_container`

## Container Concepts

The library provides comprehensive concepts for container requirements. See [Concepts](concepts.md) for details.
