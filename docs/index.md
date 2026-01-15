---
title: Home
description: Periapsis - A modern C++23 template library with modules support
---

# Periapsis

<p class="subtitle">A modern C++23 template library with modules support.</p>

---

## Overview

Periapsis is a high-performance, concept-constrained template library designed for modern C++ development. It provides data structures and algorithms with first-class support for compile-time evaluation.

<div class="grid cards" markdown>

- :material-lightning-bolt:{ .lg .middle } __C++23 Modules__

    ---

    Import the entire library with a single statement:

    ```cpp
    import synodic.periapsis;
    ```

- :material-clock-fast:{ .lg .middle } __Constexpr Everything__

    ---

    Containers and algorithms designed for compile-time evaluation with `static_assert` verification.

- :material-shield-check:{ .lg .middle } __Concept-Constrained__

    ---

    Clear, enforceable type constraints throughout using C++20 concepts.

- :material-memory:{ .lg .middle } __Zero Overhead__

    ---

    Fixed-capacity containers with no heap allocation and SIMD-ready alignment.

</div>

## Quick Start

```cpp
import synodic.periapsis;

using namespace peri;

// Constexpr ring buffer
constexpr auto sum = [] constexpr {
    FixedRingBuffer<int, 8> buf;
    buf.push_back(1);
    buf.push_back(2);
    return buf.front() + buf.back();
}();
static_assert(sum == 3);

// Compile-time graph
constexpr auto build_graph() {
    graph::Graph<void, void, graph::Fixed<4, 4>> g;
    auto v0 = g.add_vertex();
    auto v1 = g.add_vertex();
    g.add_edge(v0, v1);
    return g.num_edges();
}
static_assert(build_graph() == 1);
```

## Library Components

<div class="grid cards" markdown>

- :material-database:{ .lg .middle } __[Containers](library/containers/index.md)__

    ---

    Fixed-capacity, sparse, dynamic, and lock-free containers.

    [:octicons-arrow-right-24: Explore containers](library/containers/index.md)

- :material-graph:{ .lg .middle } __[Structures](library/structures/index.md)__

    ---

    Graphs, automata (FSMs), and tree data structures.

    [:octicons-arrow-right-24: Explore structures](library/structures/index.md)

- :material-math-compass:{ .lg .middle } __[Math](library/math/index.md)__

    ---

    SIMD-ready vectors, matrices, quaternions, and geometry.

    [:octicons-arrow-right-24: Explore math](library/math/index.md)

- :material-chart-line:{ .lg .middle } __[Statistics](library/statistics/index.md)__

    ---

    Online statistical accumulators with composable features.

    [:octicons-arrow-right-24: Explore statistics](library/statistics/index.md)

- :material-function:{ .lg .middle } __[Algorithms](library/algorithms/index.md)__

    ---

    Graph algorithms including max flow.

    [:octicons-arrow-right-24: Explore algorithms](library/algorithms/index.md)

- :material-text:{ .lg .middle } __[Strings](library/strings/index.md)__

    ---

    Compile-time string hashing and fixed-capacity strings.

    [:octicons-arrow-right-24: Explore strings](library/strings/index.md)

- :material-puzzle:{ .lg .middle } __[Type System](library/type/index.md)__

    ---

    Concepts and type traits for metaprogramming.

    [:octicons-arrow-right-24: Explore types](library/type/index.md)

- :material-wrench:{ .lg .middle } __[Utilities](library/utilities/index.md)__

    ---

    Miscellaneous utilities and helpers.

    [:octicons-arrow-right-24: Explore utilities](library/utilities/index.md)

</div>

## Getting Started

<div class="grid cards" markdown>

- :material-download:{ .lg .middle } __[Setup](setup.md)__

    ---

    Install and integrate Periapsis into your project.

- :material-code-braces:{ .lg .middle } __[Development](development.md)__

    ---

    Build, test, and contribute to the library.

</div>

## License

Periapsis is licensed under the [MIT License](https://github.com/synodic/periapsis/blob/development/LICENSE.md).
