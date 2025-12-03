# Synodic Template Library

A modern C++23 template library with modules support.

## Overview

The Synodic Template Library provides high-performance, concept-constrained data structures and algorithms with first-class support for compile-time evaluation.

**Key Features:**

- C++23 modules (`import synodic.library;`)
- Extensive use of concepts for clear constraints
- Constexpr-capable containers and algorithms
- Storage abstraction patterns (like `std::allocator_traits`)

## Documentation

### Data Structures

- [Containers](containers.md) - Fixed-capacity, sparse, and lock-free containers
  - `FixedRingBuffer`, `FixedQueue`, `FixedStack`
  - `sparse::BitMap`, `sparse::HashMap`, `sparse::Vector`
  - `lockfree::SPSCQueue`

- [Graph Library](graph.md) - High-performance graph data structures
  - Storage backends: `StaticStorage`, `AdjacencyListStorage`, `CSRStorage`
  - Compile-time graph construction
  - Subgraph composition

### Algorithms

- [Algorithms](algorithms.md) - Graph and general algorithms
  - Max Flow (Edmonds-Karp, constexpr-capable)

### Type System

- [Concepts](concepts.md) - Container and utility concepts
  - `container`, `sequence_container`, `associative_container`
  - `random_access_container`, `contiguous_container`
  - Full concept hierarchy

- [Type Traits](traits.md) - Metaprogramming utilities
  - `ConstructorParameters<T>`

### Utilities

- [Strings](strings.md) - String utilities
  - `HashString` - Compile-time FNV-1a hashing

- [Utilities](utilities.md) - Miscellaneous utilities
  - `nth_element` - Parameter pack indexing
  - `NotImplementedException`

- [Threading](threading.md) - Thread utilities (WIP)
  - `ThreadLocal`
  - Executor concepts

## Quick Start

```cpp
import synodic.library;

using namespace synodic;

// Constexpr queue
constexpr auto sum = []() constexpr {
    FixedRingBuffer<int, 8> buf;
    buf.push_back(1);
    buf.push_back(2);
    return buf.front() + buf.back();
}();
static_assert(sum == 3);

// Compile-time graph
using namespace synodic::graph;
using namespace synodic::graph::storage;

constexpr auto build_graph() {
    Graph<StaticStorage<void, void, 4, 4>> g;
    auto v0 = g.add_vertex();
    auto v1 = g.add_vertex();
    g.add_edge(v0, v1);
    return g.num_edges();
}
static_assert(build_graph() == 1);

// Container concepts
template<random_access_container C>
auto process(const C& c) { /* ... */ }
```

## Module Structure

```
synodic.library
├── :container.fixed.ring_buffer      - FixedRingBuffer, FixedQueue, FixedStack
├── :container.sparse.*               - BitMap, HashMap, HashSet, Vector
├── :container.lockfree.spsc_queue    - SPSCQueue
├── :structure.graph.*                - Graph, storage backends, concepts
├── :algorithm.graph.max_flow         - Max flow algorithm
├── :type.concepts.*                  - Container concepts
├── :type.traits                      - ConstructorParameters
├── :string.hash_string               - HashString
├── :thread.local                     - ThreadLocal
├── :function.executor                - Executor concepts
├── :utility                          - nth_element
└── :exception.not_implemented        - NotImplementedException
```

## Building

Requires:

- C++23 compiler with modules support (MSVC 19.34+, Clang 16+, GCC 14+)
- CMake 3.28+
- Conan 2.0+ (for dependencies)

```bash
# Configure
conan install . --build=missing
cmake --preset <your-preset>

# Build
cmake --build --preset <your-preset>
```

## License

See [LICENSE.md](../LICENSE.md)
