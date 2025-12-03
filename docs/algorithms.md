# Algorithms

Graph and other algorithms provided by the Synodic Template Library.

## Graph Algorithms

### Max Flow (Edmonds-Karp)

A constexpr-compatible maximum flow algorithm using the Edmonds-Karp method (BFS-based Ford-Fulkerson).

```cpp
#include <synodic/library>

using namespace synodic::graph;
using namespace synodic::graph::storage;
using namespace synodic::algorithm::graph;

// Create a flow network
Graph<StaticStorage<void, FlowEdge<int>, 6, 10>> network;

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

**Compile-Time Usage:**

```cpp
constexpr auto compute_flow() {
    Graph<StaticStorage<void, FlowEdge<int>, 4, 4>> g;
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

**Time Complexity:** O(V × E²)

**Types:**

```cpp
// Edge data for flow networks
template<typename T = int>
struct FlowEdge {
    T capacity{};   // Maximum flow allowed
    T flow{};       // Current flow

    constexpr T residual() const noexcept {
        return capacity - flow;
    }
};

// Result of max_flow computation
template<typename T>
struct MaxFlowResult {
    T max_flow{};       // Total flow from source to sink
    bool found_path{};  // Whether any augmenting path existed
};
```

**Requirements:**

- Graph must satisfy `incidence_storage`, `vertex_list_storage`, and `edge_list_storage`
- Edge properties must have `capacity` and `flow` members (use `FlowEdge<T>`)

## Future Algorithms

Planned additions:

- Shortest path (Dijkstra, Bellman-Ford)
- Minimum spanning tree (Prim, Kruskal)
- Topological sort
- Strongly connected components
- Graph coloring
