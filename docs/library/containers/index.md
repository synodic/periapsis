---
title: Containers
description: Fixed-capacity, sparse, dynamic, and lock-free containers
---

# Containers

Periapsis provides specialized container types organized by memory allocation strategy and thread-safety guarantees.

## Overview

<div class="grid cards" markdown>

- :material-package-variant-closed:{ .lg .middle } __[Fixed-Capacity](fixed.md)__

    ---

    Constexpr-capable containers with compile-time capacity.

  - `FixedRingBuffer` — Circular buffer
  - `FixedQueue` / `FixedStack` — Adapter aliases
  - `fixed::Bitset` — Fixed-size bitset

- :material-dots-grid:{ .lg .middle } __[Sparse](sparse.md)__

    ---

    Efficient storage for sparse data patterns.

  - `BitVector` — Sparse bit vector with rank/select
  - `IndexVector` — Sorted sparse index set

- :material-resize:{ .lg .middle } __[Dynamic](dynamic.md)__

    ---

    Runtime-sized containers.

  - `DynamicBitset` — Resizable bitset

- :material-lightning-bolt:{ .lg .middle } __[Lock-Free](lockfree.md)__

    ---

    Thread-safe containers without locks.

  - `SPSCQueue` — Single-producer, single-consumer queue

</div>

## Quick Comparison

| Container | Capacity | Constexpr | Thread-Safe | Use Case |
|-----------|----------|-----------|-------------|----------|
| `FixedRingBuffer<T, N>` | Fixed | :material-check: | :material-close: | Compile-time buffers |
| `fixed::Bitset<N>` | Fixed | :material-check: | :material-close: | Bit manipulation |
| `DynamicBitset` | Dynamic | :material-close: | :material-close: | Large bit sets |
| `sparse::BitVector` | Mixed | :material-check: | :material-close: | Rank/select queries |
| `sparse::IndexVector` | Dynamic | :material-check: | :material-close: | Sparse index sets |
| `lockfree::SPSCQueue` | Fixed | :material-close: | :material-check:{ title="Single producer, single consumer only" } | Producer-consumer |

## Common Patterns

### Constexpr Container Usage

All fixed-capacity containers support compile-time evaluation:

```cpp
import synodic.periapsis;

constexpr auto compute() {
    peri::FixedRingBuffer<int, 8> buf;
    buf.push_back(1);
    buf.push_back(2);
    buf.push_back(3);
    return buf.size();
}

static_assert(compute() == 3);
```

### Standard Library Integration

Fixed containers integrate with standard adapters:

```cpp
import synodic.periapsis;

using namespace peri;

// Use as std::queue backend
FixedQueue<int, 64> queue;  // std::queue<int, FixedRingBuffer<int, 64>>
queue.push(42);

// Use as std::stack backend
FixedStack<int, 64> stack;  // std::stack<int, FixedRingBuffer<int, 64>>
stack.push(42);
```

## Namespace Reference

| Namespace | Description |
|-----------|-------------|
| `peri` | Core containers (`FixedRingBuffer`, `DynamicBitset`) |
| `peri::fixed` | Fixed-size containers (`Bitset`) |
| `peri::sparse` | Sparse containers (`BitVector`, `IndexVector`) |
| `peri::lockfree` | Lock-free containers (`SPSCQueue`) |
