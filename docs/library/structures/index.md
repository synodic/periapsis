---
title: Structures
description: Graphs, automata (FSMs), and tree data structures
---

# Structures

Periapsis provides high-performance data structures for graphs, finite state machines, and trees.

## Overview

<div class="grid cards" markdown>

- :material-graph:{ .lg .middle } __[Graph Library](graph.md)__

    ---

    High-performance graph data structures with unified API.

  - Automatic storage selection (fixed vs dynamic)
  - Constexpr graph construction
  - Visitor-based traversal
  - Directed, bidirectional, and undirected graphs

- :material-state-machine:{ .lg .middle } __[Automata](automata.md)__

    ---

    Constexpr-capable finite state machines.

  - Unified `StateMachine` type
  - Compile-time FSM definition
  - `Runner` class for execution
  - Event-driven transitions

- :material-file-tree:{ .lg .middle } __[Trees](tree.md)__

    ---

    Specialized tree structures.

  - `Fenwick` — Binary indexed tree for prefix sums

</div>

## Design Principles

### 1. Unified Types

Both `Graph` and `StateMachine` use a single template with automatic storage selection:

```cpp
// Graph: storage selected by policy
graph::Graph<Data, void, graph::Fixed<10, 20>> fixed_graph;   // Constexpr
graph::Graph<Data, void, graph::Dynamic<>> dynamic_graph;     // Runtime

// StateMachine: storage selected by capacity
automata::StateMachine<Event, void, void, 4, 8> fixed_fsm;    // Constexpr
automata::StateMachine<Event> dynamic_fsm;                    // Runtime
```

### 2. Compile-Time Construction

Fixed-capacity structures support full constexpr evaluation:

```cpp
constexpr auto build_graph() {
    graph::Graph<void, void, graph::Fixed<4, 4>> g;
    auto v0 = g.add_vertex();
    auto v1 = g.add_vertex();
    g.add_edge(v0, v1);
    return g;
}

constexpr auto g = build_graph();
static_assert(g.num_vertices() == 2);
static_assert(g.num_edges() == 1);
```

### 3. Visitor Pattern

The `visitor` concept unifies traversal across graphs and automata:

```cpp
template<typename V>
concept visitor = requires(V& v, const V& cv, typename V::vertex_descriptor vtx) {
    { cv.position() } -> std::same_as<std::optional<typename V::vertex_descriptor>>;
    { cv.is_positioned() } -> std::same_as<bool>;
    { v.move_to(vtx) };
    { v.reset() };
};
```

Both `graph::Visitor` and `automata::Runner` satisfy this concept, enabling generic algorithms:

```cpp
// Works with both graphs and FSMs
template<visitor V>
void traverse_all(V& vis, auto&& on_visit) {
    while (vis.is_positioned()) {
        on_visit(vis.position().value());
        vis.advance(FirstEdgeSelector{});
    }
}
```

## Namespace Reference

| Namespace | Contents |
|-----------|----------|
| `peri::graph` | Graph types, storage, and concepts |
| `peri::automata` | State machine and runner |
| `peri::structure` | Tree structures (Fenwick) |
