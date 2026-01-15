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
| CMake | 3.28+ |
| Compiler | MSVC 19.40+, GCC 14+, or Clang 18+ |

!!! note "Module Support"
    Periapsis uses C++23 modules. Ensure your compiler has full module support enabled.

## Installation

### Using CMake FetchContent

The recommended way to use Periapsis is via CMake's FetchContent:

```cmake
include(FetchContent)

FetchContent_Declare(
    periapsis
    GIT_REPOSITORY https://github.com/synodic/periapsis.git
    GIT_TAG        development
)

FetchContent_MakeAvailable(periapsis)

target_link_libraries(your_target PRIVATE synodic::periapsis)
```

### Using find_package

If Periapsis is installed on your system:

```cmake
find_package(periapsis REQUIRED)
target_link_libraries(your_target PRIVATE synodic::periapsis)
```

### Using Conan

Periapsis can be consumed via Conan package manager:

```ini title="conanfile.txt"
[requires]
periapsis/1.0.0

[generators]
CMakeDeps
CMakeToolchain
```

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
| `peri::fixed` | Fixed-size bitset |
| `peri::lockfree` | Lock-free containers |
| `peri::statistics` | Statistical accumulators |
| `peri::structure` | Tree structures (Fenwick) |

## Compiler Configuration

### MSVC

```cmake
target_compile_options(your_target PRIVATE
    /std:c++latest
    /experimental:module
)
```

### GCC

```cmake
target_compile_options(your_target PRIVATE
    -std=c++23
    -fmodules-ts
)
```

### Clang

```cmake
target_compile_options(your_target PRIVATE
    -std=c++23
    -fmodules
)
```

## Next Steps

- [Containers](library/containers/index.md) — Start with fixed-capacity containers
- [Structures](library/structures/index.md) — Explore graphs and automata
- [Development](development.md) — Contribute to the library
