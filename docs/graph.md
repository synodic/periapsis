# Graph Library

A high-performance, concept-based graph library supporting compile-time construction, runtime modification, and storage abstraction.

## Overview

The graph library is designed around three key principles:

1. **Storage Abstraction** - Like `std::allocator_traits`, the graph is templated on a storage strategy
2. **Compile-Time Construction** - Supports `constexpr` graphs for render graphs, task scheduling
3. **Zero-Overhead Abstraction** - Concepts enable compile-time dispatch without virtual calls

## Quick Start

### Compile-Time Task Subgraph

```cpp
#include <synodic/library>

using namespace synodic::graph;
using namespace synodic::graph::storage;

struct TaskData { std::string_view name; };

constexpr auto build_tasks() {
    Graph<StaticStorage<TaskData, void, 10, 20>> g;
    auto t1 = g.add_vertex(TaskData{"load"});
    auto t2 = g.add_vertex(TaskData{"process"});
    auto t3 = g.add_vertex(TaskData{"save"});
    g.add_edge(t1, t2);
    g.add_edge(t2, t3);
    return g;
}

constexpr auto tasks = build_tasks();
```

### Runtime Dynamic Graph

```cpp
using namespace synodic::graph;

// Dynamic graph with vertex and edge properties
DynamicGraph<NodeData, EdgeWeight> graph;

auto v0 = graph.add_vertex(NodeData{...});
auto v1 = graph.add_vertex(NodeData{...});
graph.add_edge(v0, v1, EdgeWeight{1.5f});

// Iterate neighbors
for (auto e : graph.out_edges(v0)) {
    auto target = graph.target(e);
    auto weight = graph[e];
}
```

### Subgraph Composition

Modules can define subgraphs independently, then compose them into a scheduler:

```cpp
// Module A defines its task subgraph
constexpr auto module_a_tasks = []() {
    Graph<StaticStorage<TaskData, void, 4, 4>> g;
    auto entry = g.add_vertex(TaskData{"a_start"});
    auto work = g.add_vertex(TaskData{"a_work"});
    auto exit = g.add_vertex(TaskData{"a_end"});
    g.add_edge(entry, work);
    g.add_edge(work, exit);
    g.mark_boundary(entry);  // Entry point
    g.mark_boundary(exit);   // Exit point
    return g;
}();

// Scheduler composes subgraphs at runtime
DynamicGraph<TaskData, void> scheduler;
auto mapping = scheduler.compose(module_a_tasks);
// mapping[old_vertex] -> new_vertex in scheduler
```

## Storage Types

### StaticStorage

Fixed-capacity storage using `std::array`. Fully `constexpr`-capable.

```cpp
StaticStorage<VertexProp, EdgeProp, MaxVertices, MaxEdges, IndexType>
```

- **Use case**: Compile-time graphs, embedded systems, known-size graphs
- **Complexity**: O(1) vertex/edge access, O(V) edge insertion
- **Memory**: Stack-allocated, no heap

### AdjacencyListStorage

Dynamic storage using `std::vector`.

```cpp
AdjacencyListStorage<VertexProp, EdgeProp, VertexContainer, EdgeContainer, IndexType, Bidirectional>
```

- **Use case**: Runtime-modifiable graphs, general purpose
- **Complexity**: O(1) amortized insertion, O(E) edge removal
- **Memory**: Heap-allocated, grows dynamically

### CSRStorage

Compressed Sparse Row format for immutable graphs.

```cpp
CSRStorage<VertexProp, EdgeProp, IndexType>
```

- **Use case**: Read-heavy workloads, cache-friendly traversal
- **Complexity**: O(1) access, immutable after construction
- **Memory**: Compact, cache-friendly layout

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
├── mutable_vertex_storage   - add_vertex(), remove_vertex(), clear_vertices()
├── mutable_edge_storage     - add_edge(), remove_edge(), clear_edges()
├── vertex_property_storage  - operator[](vertex)
├── edge_property_storage    - operator[](edge)
├── constexpr_storage        - is_constexpr_capable
└── composable_storage       - boundary_vertices()
```

## Algorithms

### Max Flow (Edmonds-Karp)

Constexpr-compatible maximum flow algorithm:

```cpp
using namespace synodic::algorithm::graph;

Graph<StaticStorage<void, FlowEdge<int>, 6, 10>> network;
// ... build flow network ...

auto result = max_flow(network, source, sink);
// result.max_flow - the maximum flow value
// result.found_path - whether any augmenting path existed
```

Works at compile-time:

```cpp
constexpr auto flow = []() {
    Graph<StaticStorage<void, FlowEdge<int>, 4, 4>> g;
    auto s = g.add_vertex();
    auto t = g.add_vertex();
    auto a = g.add_vertex();
    g.add_edge(s, a, FlowEdge<int>{10, 0});
    g.add_edge(a, t, FlowEdge<int>{10, 0});
    return max_flow(g, s, t);
}();

static_assert(flow.max_flow == 10);
```

## Type Aliases

```cpp
// Dynamic graph with properties
DynamicGraph<VertexProperty, EdgeProperty>

// Dynamic bidirectional graph
BidirectionalGraph<VertexProperty, EdgeProperty>

// Topology-only dynamic graph
DynamicTopology

// Static topology (no properties)
StaticTopology<MaxVertices, MaxEdges>

// Static with vertex properties only
StaticVertexStorage<VertexProp, MaxVertices, MaxEdges>

// Static with edge properties only
StaticEdgeStorage<EdgeProp, MaxVertices, MaxEdges>
```

## Storage Traits

Query storage capabilities uniformly via `storage_traits<Storage>`:

```cpp
using traits = storage_traits<MyStorage>;

static_assert(traits::is_directed);
static_assert(traits::is_constexpr_capable);
static_assert(traits::supports_vertex_properties);

using vertex_t = traits::vertex_descriptor;
using edge_t = traits::edge_descriptor;
```
