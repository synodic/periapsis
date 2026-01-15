---
title: Algorithms
description: Graph and other algorithms
---

# Algorithms

Periapsis provides constexpr-capable algorithms for graphs and other data structures.

## Graph Algorithms

### Max Flow (Edmonds-Karp)

A constexpr-compatible maximum flow algorithm using the Edmonds-Karp method (BFS-based Ford-Fulkerson).

#### Synopsis

```cpp
namespace peri::algorithm::graph {
    template<typename Graph>
    constexpr auto max_flow(
        Graph& graph,
        typename Graph::vertex_descriptor source,
        typename Graph::vertex_descriptor sink
    ) -> MaxFlowResult<typename Graph::edge_property_type::value_type>;
}
```

#### FlowEdge Type

Edge properties for flow networks must provide capacity and flow:

```cpp
namespace peri::algorithm::graph {
    template<typename T = int>
    struct FlowEdge {
        T capacity{};   // Maximum flow allowed
        T flow{};       // Current flow

        constexpr T residual() const noexcept {
            return capacity - flow;
        }
    };
}
```

#### MaxFlowResult Type

```cpp
namespace peri::algorithm::graph {
    template<typename T>
    struct MaxFlowResult {
        T max_flow{};       // Total flow from source to sink
        bool found_path{};  // Whether any augmenting path existed
    };
}
```

#### Example

```cpp
import synodic.periapsis;

using namespace peri::graph;
using namespace peri::algorithm::graph;

// Create a flow network
Graph<void, FlowEdge<int>, Fixed<6, 10>> network;

auto s = network.add_vertex();  // Source
auto a = network.add_vertex();
auto b = network.add_vertex();
auto t = network.add_vertex();  // Sink

// Add edges with capacity
network.add_edge(s, a, FlowEdge<int>{16, 0});
network.add_edge(s, b, FlowEdge<int>{13, 0});
network.add_edge(a, b, FlowEdge<int>{10, 0});
network.add_edge(a, t, FlowEdge<int>{12, 0});
network.add_edge(b, t, FlowEdge<int>{14, 0});

// Compute max flow
auto result = max_flow(network, s, t);

std::cout << "Max flow: " << result.max_flow << "\n";
std::cout << "Path found: " << result.found_path << "\n";
```

#### Constexpr Usage

The algorithm is fully constexpr-capable:

```cpp
constexpr auto compute_flow() {
    Graph<void, FlowEdge<int>, Fixed<4, 4>> g;

    auto s = g.add_vertex();
    auto a = g.add_vertex();
    auto t = g.add_vertex();

    g.add_edge(s, a, FlowEdge<int>{10, 0});
    g.add_edge(a, t, FlowEdge<int>{10, 0});

    return max_flow(g, s, t);
}

constexpr auto result = compute_flow();
static_assert(result.max_flow == 10);
static_assert(result.found_path == true);
```

#### Complexity

| Aspect | Complexity |
|--------|------------|
| Time | O(V × E²) |
| Space | O(V + E) |

Where V is the number of vertices and E is the number of edges.

#### Requirements

The graph must satisfy:

- `incidence_storage` — Access to outgoing edges
- `vertex_list_storage` — Enumeration of vertices
- `edge_list_storage` — Enumeration of edges

Edge properties must have:

- `capacity` — Maximum flow allowed
- `flow` — Current flow (mutable)

---

## Future Algorithms

!!! note "Planned Additions"
    The following algorithms are planned for future releases:

### Shortest Path

- **Dijkstra** — Single-source shortest path (non-negative weights)
- **Bellman-Ford** — Single-source shortest path (negative weights allowed)
- **Floyd-Warshall** — All-pairs shortest path

### Minimum Spanning Tree

- **Prim** — MST using priority queue
- **Kruskal** — MST using union-find

### Graph Structure

- **Topological Sort** — Linear ordering of DAG vertices
- **Strongly Connected Components** — Tarjan's or Kosaraju's algorithm
- **Biconnected Components** — Cut vertices and bridges

### Matching and Coloring

- **Graph Coloring** — Greedy and optimal algorithms
- **Bipartite Matching** — Maximum matching in bipartite graphs
