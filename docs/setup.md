---
title: Setup
description: How to install and use Periapsis in your project
---

# Setup

This guide covers how to integrate Periapsis into your C++ project.

## Requirements

| Requirement | Version |
|-------------|---------|
| C++ Standard | C++23 |
| CMake | 4.0+ |

!!! note "Module Support"
    Periapsis uses C++23 modules. Ensure your compiler has full module support enabled.

## Installation

## Usage

### Importing the Library

Import the entire library with a single statement:

```cpp
import synodic.periapsis;
```

All library components are available under the `peri` namespace:

```cpp
import synodic.periapsis;

int main() {
    peri::FixedRingBuffer<int, 64> buffer;
    buffer.push_back(42);
    return buffer.front();
}
```

### Namespace Organization

| Namespace | Contents |
|-----------|----------|
| `peri` | Core containers and utilities |
| `peri::graph` | Graph data structures and algorithms |
| `peri::automata` | Finite state machines |
| `peri::sparse` | Sparse containers |
| `peri::fixed` | Fixed-size containers |
| `peri::lockfree` | Lock-free containers |
| `peri::statistics` | Statistical accumulators |
| `peri::structure` | Tree structures |
