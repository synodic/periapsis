---
title: Graph Library
description: High-performance graph data structures with unified API
---

# Graph Library

A high-performance, concept-based graph library supporting compile-time construction, runtime modification, and visitor-based traversal.

## Overview

The graph library is designed around three key principles:

1. **Unified Type** — `Graph<V, E, Policy>` auto-selects storage based on policy
2. **Compile-Time Construction** — Supports constexpr graphs for render graphs, task scheduling
3. **Structure/Execution Separation** — Graph holds structure, Visitor handles traversal

## Graph Type

### Synopsis

```cpp
namespace peri::graph {
    template<
        typename VertexProperty,
        typename EdgeProperty,
        typename Policy = Dynamic<>
    >
    class Graph;
}
```

### Template Parameters

| Parameter | Description |
|-----------|-------------|
| `VertexProperty` | Data stored per vertex (`void` for none) |
| `EdgeProperty` | Data stored per edge (`void` for none) |
| `Policy` | Storage policy (`Dynamic<>` or `Fixed<MaxV, MaxE>`) |

### Storage Policies

=== "Dynamic"

    ```cpp
    template<Directionality Dir = Directionality::DIRECTED>
    struct Dynamic;
    ```

    - Growable at runtime
    - Uses `std::vector`-based storage
    - Default policy

=== "Fixed"

    ```cpp
    template<
        std::size_t MaxV,
        std::size_t MaxE,
        Directionality Dir = Directionality::DIRECTED
    >
    struct Fixed;
    ```

    - Fixed capacity at compile-time
    - Fully constexpr capable
    - Zero heap allocation

### Directionality

```cpp
enum class Directionality {
    DIRECTED,      // Forward-only edge traversal
    BIDIRECTIONAL, // Both forward and backward edge traversal
    UNDIRECTED     // Edges are symmetric
};
```

## Quick Start

### Dynamic Graph

```cpp
import synodic.periapsis;

using namespace peri::graph;

// Default: dynamic, directed
Graph<NodeData, EdgeWeight> graph;

auto v0 = graph.add_vertex(NodeData{...});
auto v1 = graph.add_vertex(NodeData{...});
graph.add_edge(v0, v1, EdgeWeight{1.5f});

// Iterate neighbors
for (auto e : graph.out_edges(v0)) {
    auto target = graph.target(e);
    auto weight = graph[e];
}
```

### Fixed-Size Constexpr Graph

```cpp
using namespace peri::graph;

constexpr auto build_tasks() {
    Graph<void, void, Fixed<10, 20>> g;  // Max 10 vertices, 20 edges

    auto t1 = g.add_vertex();
    auto t2 = g.add_vertex();
    auto t3 = g.add_vertex();

    g.add_edge(t1, t2);
    g.add_edge(t2, t3);

    return g;
}

constexpr auto tasks = build_tasks();  // Evaluated at compile-time
static_assert(tasks.num_vertices() == 3);
static_assert(tasks.num_edges() == 2);
```

### Seeding: Constexpr to Dynamic

Extend a compile-time graph at runtime:

```cpp
// Start with compile-time structure
constexpr auto seed = [] constexpr {
    Graph<int, void, Fixed<4, 4>> g;
    auto a = g.add_vertex(1);
    auto b = g.add_vertex(2);
    g.add_edge(a, b);
    return g;
}();

// Seed a dynamic graph from constexpr
Graph<int, void> dynamic{seed};  // Copies structure

// Now add more at runtime
dynamic.add_vertex(3);
dynamic.add_edge(1, 2);
```

## Graph Operations

### Vertex Operations

| Method | Returns | Description |
|--------|---------|-------------|
| `add_vertex()` | `vertex_descriptor` | Add vertex with default property |
| `add_vertex(prop)` | `vertex_descriptor` | Add vertex with property |
| `remove_vertex(v)` | `void` | Remove vertex (invalidates descriptors) |
| `operator[](v)` | `VertexProperty&` | Access vertex property |
| `num_vertices()` | `size_type` | Number of vertices |
| `vertices()` | Range | Range of vertex descriptors |

### Edge Operations

| Method | Returns | Description |
|--------|---------|-------------|
| `add_edge(u, v)` | `edge_descriptor` | Add edge with default property |
| `add_edge(u, v, prop)` | `edge_descriptor` | Add edge with property |
| `remove_edge(e)` | `void` | Remove edge |
| `operator[](e)` | `EdgeProperty&` | Access edge property |
| `num_edges()` | `size_type` | Number of edges |
| `edges()` | Range | Range of edge descriptors |
| `out_edges(v)` | Range | Outgoing edges from vertex |
| `in_edges(v)` | Range | Incoming edges to vertex (bidirectional only) |
| `source(e)` | `vertex_descriptor` | Source vertex of edge |
| `target(e)` | `vertex_descriptor` | Target vertex of edge |

---

## Visitor Class

The `Visitor` class separates traversal state from graph structure.

### Synopsis

```cpp
namespace peri::graph {
    template<typename Graph>
    class Visitor;
}
```

### Construction

```cpp
Graph<NodeData, void> graph;
// ... build graph ...

// Create positioned visitor
Visitor visitor{graph, graph.vertices().front()};

// Create unpositioned visitor
Visitor visitor{graph};
```

### Operations

| Method | Returns | Description |
|--------|---------|-------------|
| `position()` | `std::optional<vertex_descriptor>` | Current vertex |
| `is_positioned()` | `bool` | Has current position |
| `move_to(v)` | `void` | Jump to vertex |
| `reset()` | `void` | Clear position |
| `advance(selector)` | `AdvanceResult` | Move along selected edge |

### Advance Results

```cpp
enum class AdvanceResult {
    Moved,      // Successfully moved to new vertex
    Blocked,    // No edge selected
    NoPosition  // Visitor not positioned
};
```

### Edge Selectors

Edge selectors determine which edge to follow:

```cpp
// Take first available edge
auto result = visitor.advance(FirstEdgeSelector{});

// Select edge matching predicate
auto result = visitor.advance(
    PredicateSelector{[](auto& graph, auto edge) {
        return graph[edge].weight > 0.5;
    }}
);
```

### Example

```cpp
import synodic.periapsis;

using namespace peri::graph;

// Build a simple graph
Graph<std::string, void> graph;
auto a = graph.add_vertex("A");
auto b = graph.add_vertex("B");
auto c = graph.add_vertex("C");
graph.add_edge(a, b);
graph.add_edge(b, c);

// Traverse with visitor
Visitor visitor{graph, a};

while (visitor.is_positioned()) {
    auto v = visitor.position().value();
    std::cout << graph[v] << "\n";

    auto result = visitor.advance(FirstEdgeSelector{});
    if (result == AdvanceResult::Blocked) {
        break;  // No more edges
    }
}
// Output: A, B, C
```

### Const-Compatible Traversal

Visitors work with const graphs for thread-safe traversal:

```cpp
const Graph<Data, void> immutable = build_graph();

// Multiple visitors can traverse concurrently
Visitor v1{immutable, start1};
Visitor v2{immutable, start2};
```

---

## Undirected Graphs

For undirected graphs, use the `UNDIRECTED` directionality:

```cpp
using namespace peri::graph;

Graph<void, float, Dynamic<Directionality::UNDIRECTED>> graph;

auto a = graph.add_vertex();
auto b = graph.add_vertex();

// Edge is traversable both directions
graph.add_edge(a, b, 1.0f);

// Both work
for (auto e : graph.out_edges(a)) { ... }  // Includes edge to b
for (auto e : graph.out_edges(b)) { ... }  // Includes edge to a
```

---

## Bidirectional Graphs

For efficient reverse traversal, use `BIDIRECTIONAL`:

```cpp
using namespace peri::graph;

Graph<void, void, Dynamic<Directionality::BIDIRECTIONAL>> graph;

auto a = graph.add_vertex();
auto b = graph.add_vertex();
graph.add_edge(a, b);

// Forward traversal
for (auto e : graph.out_edges(a)) { ... }

// Reverse traversal (efficient)
for (auto e : graph.in_edges(b)) { ... }
```

---

## Graph Concepts

The library defines concepts for generic algorithm development:

```cpp
namespace peri::graph {
    // Basic graph access
    template<typename G>
    concept incidence_graph = /* has out_edges, source, target */;

    // Vertex enumeration
    template<typename G>
    concept vertex_list_graph = /* has vertices(), num_vertices() */;

    // Edge enumeration
    template<typename G>
    concept edge_list_graph = /* has edges(), num_edges() */;

    // Visitor compatibility
    template<typename V>
    concept visitor = /* has position, move_to, reset, advance */;

    template<typename V, typename G>
    concept visitor_for = visitor<V> && /* type compatible with G */;
}
```

---

## Storage Types

Advanced users can access storage types directly:

| Storage | Description |
|---------|-------------|
| `AdjacencyListStorage` | Dynamic directed graph |
| `UndirectedAdjacencyList` | Dynamic undirected graph |
| `StaticStorage` | Fixed-size directed graph |
| `BidirectionalFixed` | Fixed-size bidirectional graph |
| `UndirectedFixed` | Fixed-size undirected graph |
| `CSRStorage` | Compressed sparse row (read-only) |
