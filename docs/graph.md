# Graph Library

A high-performance, concept-based graph library supporting compile-time construction, runtime modification, and visitor-based traversal.

## Overview

The graph library is designed around three key principles:

1. **Unified Type** - `Graph<V,E,MaxV,MaxE>` auto-selects storage (fixed vs dynamic)
2. **Compile-Time Construction** - Supports `constexpr` graphs for render graphs, task scheduling
3. **Structure/Execution Separation** - Graph holds structure, Visitor handles traversal

## Quick Start

### Dynamic Graph (Default)

```cpp
import synodic.periapsis;

using namespace peri::graph;

// Dynamic graph with properties - growable at runtime
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

struct TaskData { std::string_view name; };

// Fixed capacity - constexpr capable
constexpr auto build_tasks() {
    Graph<TaskData, void, 10, 20> g;  // Max 10 vertices, 20 edges
    auto t1 = g.add_vertex(TaskData{"load"});
    auto t2 = g.add_vertex(TaskData{"process"});
    auto t3 = g.add_vertex(TaskData{"save"});
    g.add_edge(t1, t2);
    g.add_edge(t2, t3);
    return g;
}

constexpr auto tasks = build_tasks();  // Evaluated at compile-time
```

### Seeding: Constexpr to Dynamic

Extend a compile-time graph at runtime:

```cpp
// Start with compile-time structure
constexpr auto seed = []() {
    Graph<int, void, 4, 4> g;
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

## Visitor Concept

The library defines a unified `visitor` concept for graph traversal. This concept provides a common interface that both `Visitor` (graph traversal) and `Runner` (state machine execution) satisfy:

```cpp
template<typename V>
concept visitor = requires(V& v, const V& cv, typename V::vertex_descriptor vtx) {
    { cv.position() } -> std::same_as<std::optional<typename V::vertex_descriptor>>;
    { cv.is_positioned() } -> std::same_as<bool>;
    { v.move_to(vtx) };
    { v.reset() };
};

// Validates visitor works with specific graph
template<typename V, typename G>
concept visitor_for = visitor<V> && /* type compatibility checks */;
```

This unified interface enables:

- **Algorithm Reuse** - Write algorithms once, use with graphs and FSMs
- **Type Safety** - Compile-time verification of visitor/graph compatibility
- **Consistent API** - Same traversal patterns across different structures

## Visitor Class

The `Visitor` class separates traversal state from graph structure and satisfies the `visitor` concept:

```cpp
using namespace peri::graph;

Graph<NodeData, EdgeWeight> graph;
// ... build graph ...

// Create a visitor at vertex 0
Visitor visitor{graph, graph.vertices().front()};

// Visitor interface
auto pos = visitor.position();        // Current vertex (optional)
bool active = visitor.is_positioned(); // Has current position
visitor.move_to(some_vertex);          // Direct navigation
visitor.reset();                       // Clear position

// Advance using edge selectors
auto result = visitor.advance(FirstEdgeSelector{});

if (result == AdvanceResult::Moved) {
    auto current = visitor.position();
    // Process current vertex
}
```

### Edge Selectors

Edge selectors determine which edge to follow during traversal:

```cpp
template<typename S, typename G>
concept edge_selector = requires(S& s, const G& g, typename G::vertex_descriptor v) {
    { s(g, v) } -> /* optional edge descriptor */;
};
```

Built-in selectors:

```cpp
// Take first available edge
FirstEdgeSelector{}

// Select edge matching predicate
PredicateSelector{[](auto& graph, auto edge) {
    return graph[edge].weight > 0.5;
}}

// Select edge by matching event value (for FSM-style traversal)
EventSelector{my_event, [](auto& e) { return e.event; }}
```

### Const-Compatible Traversal

Visitors work with const graphs for thread-safe traversal:

```cpp
const Graph<Data, void> immutable_graph = build_graph();

// Multiple visitors can traverse concurrently
Visitor v1{immutable_graph, start1};
Visitor v2{immutable_graph, start2};

// Visitors satisfy visitor concept
static_assert(visitor<decltype(v1)>);
```

### Visitor-Based Algorithms

The visitor concept enables generic graph algorithms:

```cpp
// Algorithm works with any visitor
template<visitor V>
auto bfs_visit(V& vis, auto&& visit_fn) {
    while (vis.is_positioned()) {
        visit_fn(vis.position().value());
        vis.advance(FirstEdgeSelector{});
    }
}

// Use with Visitor
Graph<Data, void> g;
Visitor vis{g, start};
bfs_visit(vis, [](auto v) { /* process */ });

// FSM Runner also satisfies visitor - same algorithm works!
// StateMachine<Event, State, Trans> sm;
// Runner runner{sm, initial_state};
// bfs_visit(runner, [](auto s) { /* process */ });
```

## Storage Selection

The unified `Graph` type automatically selects storage:

| Template Args | Storage Type | Capabilities |
|--------------|--------------|--------------|
| `Graph<V,E>` | AdjacencyList | Dynamic, growable |
| `Graph<V,E,MaxV,MaxE>` | StaticStorage | Constexpr, fixed |

Internally available storages (in `storage` namespace):

- `StaticStorage` - Fixed-size arrays, fully constexpr
- `AdjacencyListStorage` - Dynamic vectors
- `CSRStorage` - Compressed sparse row (immutable, cache-friendly)

## Type Aliases

```cpp
// Dynamic graph (default) - equivalent to Graph<V,E>
DynamicGraph<VertexProperty, EdgeProperty>

// Fixed-size constexpr graph
FixedGraph<VertexProperty, EdgeProperty, MaxV, MaxE>

// Topology-only (no properties)
DynamicTopology
```

## Concepts Hierarchy

```
graph_storage (base)
├── vertex_list_storage      - vertices(), num_vertices()
├── edge_list_storage        - edges(), num_edges()
├── incidence_storage        - out_edges(), out_degree(), source(), target()
├── bidirectional_storage    - in_edges(), in_degree()
├── adjacency_storage        - adjacent_vertices()
├── appendable_vertex_storage - add_vertex()
├── appendable_edge_storage  - add_edge()
├── mutable_vertex_storage   - remove_vertex(), clear_vertices()
├── mutable_edge_storage     - remove_edge(), clear_edges()
├── vertex_property_storage  - operator[](vertex)
├── edge_property_storage    - operator[](edge)
├── constexpr_storage        - is_constexpr_capable
└── composable_storage       - boundary_vertices()
```

## Algorithms

### Max Flow (Edmonds-Karp)

Constexpr-compatible maximum flow algorithm:

```cpp
using namespace peri::algorithm::graph;

Graph<void, FlowEdge<int>, 6, 10> network;
// ... build flow network ...

auto result = max_flow(network, source, sink);
// result.max_flow - the maximum flow value
// result.found_path - whether any augmenting path existed
```

Works at compile-time:

```cpp
constexpr auto flow = []() {
    Graph<void, FlowEdge<int>, 4, 4> g;
    auto s = g.add_vertex();
    auto t = g.add_vertex();
    auto a = g.add_vertex();
    g.add_edge(s, a, FlowEdge<int>{10, 0});
    g.add_edge(a, t, FlowEdge<int>{10, 0});
    return max_flow(g, s, t);
}();

static_assert(flow.max_flow == 10);
```

## Constants

```cpp
// Sentinel for dynamic storage selection
inline constexpr std::size_t dynamic_extent;
```
