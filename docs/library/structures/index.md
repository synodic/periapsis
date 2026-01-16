---
title: Structures
description: Graphs, automata (FSMs), and tree data structures
---

# Structures

Periapsis provides high-performance data structures for graphs, finite state machines, and trees.

## Graph Library

## Automata

## Trees

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
