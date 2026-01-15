---
title: Lock-Free Containers
description: Thread-safe containers without locks
---

# Lock-Free Containers

Lock-free containers provide thread-safe operations without using mutexes, enabling high-performance concurrent programming.

## SPSCQueue

A single-producer, single-consumer (SPSC) lock-free queue adapter. Provides atomic access guarantees for concurrent scenarios with exactly one reader and one writer thread.

### Synopsis

```cpp
namespace peri::lockfree {
    template<typename Container>
        requires inplace_constructing_sequence_container<Container>
    class SPSCQueue;
}
```

### Template Parameters

| Parameter | Constraints |
|-----------|-------------|
| `Container` | Must satisfy `inplace_constructing_sequence_container` |

The underlying container must support:

- `emplace(iterator, args...)` — In-place construction
- `size()` — Current element count
- Random access iteration

### Member Types

| Type | Definition |
|------|------------|
| `value_type` | `Container::value_type` |
| `size_type` | `std::size_t` |
| `container_type` | `Container` |

### Construction

```cpp
using namespace peri::lockfree;

// From container with pre-allocated capacity
std::vector<int> storage;
storage.resize(1024);
SPSCQueue queue(std::move(storage));

// The container's size determines queue capacity
```

!!! warning "Capacity"
    The queue capacity is fixed at construction time based on the container's size. The container must be pre-sized.

### Producer Operations

These operations are safe to call from the producer thread:

| Method | Returns | Description |
|--------|---------|-------------|
| `push(value)` | `void` | Push element (blocks if full) |
| `try_push(value)` | `bool` | Try to push, returns `false` if full |
| `emplace(args...)` | `void` | Construct element in-place |
| `try_emplace(args...)` | `bool` | Try to construct in-place |

### Consumer Operations

These operations are safe to call from the consumer thread:

| Method | Returns | Description |
|--------|---------|-------------|
| `pop()` | `std::optional<T>` | Remove and return front element |
| `try_pop()` | `std::optional<T>` | Non-blocking pop attempt |
| `front()` | `const T&` | Access front element (UB if empty) |

### Shared Operations

Safe to call from either thread:

| Method | Returns | Description |
|--------|---------|-------------|
| `empty()` | `bool` | Test if queue is empty |
| `size()` | `size_type` | Approximate current size |
| `capacity()` | `size_type` | Maximum capacity |

### Container Extraction

| Method | Description |
|--------|-------------|
| `extract()` | Move out the underlying container |

!!! danger "Single-Threaded Only"
    `extract()` must only be called when no concurrent access is occurring. It invalidates the queue.

### Example

```cpp
import synodic.periapsis;

#include <thread>
#include <vector>

using namespace peri::lockfree;

int main() {
    // Create queue with capacity for 1024 elements
    std::vector<int> storage(1024);
    SPSCQueue queue(std::move(storage));

    // Producer thread
    std::thread producer([&queue] {
        for (int i = 0; i < 1000; ++i) {
            queue.push(i);
        }
    });

    // Consumer thread
    std::thread consumer([&queue] {
        int sum = 0;
        int count = 0;
        while (count < 1000) {
            if (auto value = queue.pop()) {
                sum += *value;
                ++count;
            }
        }
        // sum == 499500
    });

    producer.join();
    consumer.join();

    return 0;
}
```

### Non-Blocking Pattern

For real-time or latency-sensitive code, use the non-blocking variants:

```cpp
// Producer side
while (!queue.try_push(value)) {
    // Queue is full, handle backpressure
    handle_full_queue();
}

// Consumer side
while (true) {
    if (auto value = queue.try_pop()) {
        process(*value);
    } else {
        // Queue is empty
        do_other_work();
    }
}
```

### Implementation Details

The implementation uses:

- **Cache-line alignment**: Head and tail indices are on separate cache lines to avoid false sharing
- **Memory ordering**: Uses `std::memory_order_acquire` and `std::memory_order_release` for synchronization
- **Circular buffer**: Elements are stored in a circular pattern within the container

### Performance Characteristics

| Operation | Complexity | Lock-Free |
|-----------|------------|-----------|
| `push` | O(1) | :material-check: |
| `try_push` | O(1) | :material-check: |
| `pop` | O(1) | :material-check: |
| `try_pop` | O(1) | :material-check: |
| `empty` | O(1) | :material-check: |
| `size` | O(1) | :material-check: (approximate) |

!!! note "Size Approximation"
    The `size()` method may return an approximate value due to concurrent modifications. It's safe to call but shouldn't be used for precise synchronization.

### Requirements

The value type must satisfy:

- Nothrow move constructible
- Nothrow destructible

```cpp
// Valid value types
SPSCQueue<std::vector<int>> queue1;           // ✓
SPSCQueue<std::vector<std::string>> queue2;   // ✓

// The container must satisfy inplace_constructing_sequence_container
```

### When to Use

| Scenario | Recommendation |
|----------|----------------|
| Single producer, single consumer | :material-check: Use `SPSCQueue` |
| Multiple producers | :material-close: Use mutex-based queue |
| Multiple consumers | :material-close: Use mutex-based queue |
| Need blocking wait | :material-close: Use condition variable with mutex |
| Real-time / low-latency | :material-check: Use `try_push`/`try_pop` |
