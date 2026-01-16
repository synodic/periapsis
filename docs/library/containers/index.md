---
title: Containers
description: Fixed-capacity, sparse, dynamic, and lock-free containers
---

# Containers

Periapsis provides specialized container types organized by memory allocation strategy and thread-safety guarantees.

## Fixed-Capacity

## Sparse

## Dynamic

## Lock-Free

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
