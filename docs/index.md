# Periapsis

A modern C++23 template library with modules support.

## Overview

The Periapsis provides high-performance, concept-constrained data structures and algorithms with first-class support for compile-time evaluation.

**Key Features:**

- C++23 modules (`import synodic.periapsis;`)
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
  - Unified `Graph<V,E,MaxV,MaxE>` type with automatic storage selection
  - Compile-time graph construction
  - `Visitor` class for traversal (satisfies `visitor` concept)
  - Visitor concept for generic algorithm compatibility

- [Automata](automata.md) - Constexpr-capable finite state machines
  - Unified `StateMachine<Event,StateData,TransData,MaxS,MaxT>` type
  - Compile-time FSM definition and validation
  - `Runner` class for execution (satisfies `visitor` concept)
  - Guards, actions, and event processing

### Algorithms

- [Algorithms](algorithms.md) - Graph and general algorithms
  - Max Flow (Edmonds-Karp, constexpr-capable)

### Math

- [Math](math.md) - Vector and matrix types for graphics
  - `Vec2`, `Vec3`, `Vec4` - SIMD-ready vector types
  - Constexpr operations: `Dot`, `Cross`, `Normalize`, `Lerp`

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
import synodic.periapsis;

using namespace peri;

// Constexpr queue
constexpr auto sum = []() constexpr {
    FixedRingBuffer<int, 8> buf;
    buf.push_back(1);
    buf.push_back(2);
    return buf.front() + buf.back();
}();
static_assert(sum == 3);

// Compile-time graph with unified API
using namespace peri::graph;

constexpr auto build_graph() {
    Graph<void, void, 4, 4> g;  // Fixed-size, constexpr capable
    auto v0 = g.add_vertex();
    auto v1 = g.add_vertex();
    g.add_edge(v0, v1);
    return g.num_edges();
}
static_assert(build_graph() == 1);

// Visitor concept - same interface for graph and FSM traversal
Graph<int, void> graph;
Visitor vis{graph, start_vertex};
vis.position();     // Get current position
vis.move_to(v);     // Navigate directly
vis.advance(...);   // Follow edge

// Container concepts
template<random_access_container C>
auto process(const C& c) { /* ... */ }
```

## Module Structure

```
synodic.periapsis
├── :container.fixed.ring_buffer      - FixedRingBuffer, FixedQueue, FixedStack
├── :container.sparse.*               - BitMap, HashMap, HashSet, Vector
├── :container.lockfree.spsc_queue    - SPSCQueue
├── :structure.graph.*                - Graph, Visitor, walker concept
├── :structure.automata.*             - StateMachine, Runner
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
