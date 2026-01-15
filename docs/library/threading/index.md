---
title: Threading
description: Thread utilities
---

# Threading

Thread-related utilities for concurrent programming.

!!! warning "Work in Progress"
    The threading module is under active development. APIs may change.

## ThreadLocal

A managed object that exists statically for each thread with proper lifecycle management.

### Synopsis

```cpp
namespace peri {
    template<typename T>
    class ThreadLocal;
}
```

### Description

`ThreadLocal<T>` provides a wrapper for thread-local storage with automatic construction and destruction per thread.

### Example

```cpp
import synodic.periapsis;

using namespace peri;

// Thread-local cache
ThreadLocal<std::vector<int>> cache;

void worker() {
    // Each thread gets its own vector
    cache->push_back(42);
}
```

!!! note "Status"
    Implementation in progress.

---

## Future Plans

The following features are planned for the threading module:

### Executors

Integration with `std::execution` (P2300):

```cpp
namespace peri::executor {
    template<typename T>
    concept scheduler = /* ... */;

    template<typename T>
    concept sender = /* ... */;

    template<typename T>
    concept receiver = /* ... */;
}
```

### Thread Pool

A work-stealing thread pool:

```cpp
namespace peri {
    class ThreadPool {
    public:
        explicit ThreadPool(std::size_t num_threads);

        template<typename F>
        auto submit(F&& task) -> std::future<std::invoke_result_t<F>>;

        void wait();
    };
}
```

### Task Graph

Integration with the graph library for task scheduling:

```cpp
using namespace peri;

// Define task dependencies as a graph
graph::Graph<Task, void> task_graph;

auto load = task_graph.add_vertex([] { return load_data(); });
auto process = task_graph.add_vertex([] { return process_data(); });
auto save = task_graph.add_vertex([] { save_data(); });

task_graph.add_edge(load, process);
task_graph.add_edge(process, save);

// Execute in topological order
ThreadPool pool(4);
execute_graph(pool, task_graph);
```

### Parallel Algorithms

Parallel versions of graph algorithms:

```cpp
// Parallel BFS
auto distances = parallel_bfs(graph, source, pool);

// Parallel max flow
auto result = parallel_max_flow(network, source, sink, pool);
```
