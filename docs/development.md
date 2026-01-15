---
title: Development
description: Building, testing, and contributing to Periapsis
---

# Development

This guide covers building Periapsis from source and contributing to the project.

## Requirements

| Tool | Version | Purpose |
|------|---------|---------|
| Python | 3.11+ | Build tooling via CPPython |
| CMake | 3.28+ | Build system |
| Ninja | 1.11+ | Build generator (recommended) |
| Conan | 2.0+ | Package management |
| Compiler | See [Setup](setup.md) | C++23 with modules |

## Quick Start

=== "Windows"

    ```powershell
    # Clone the repository
    git clone https://github.com/synodic/periapsis.git
    cd periapsis

    # Install Python dependencies
    pip install -e .

    # Configure and build
    cppython build
    ```

=== "Linux/macOS"

    ```bash
    # Clone the repository
    git clone https://github.com/synodic/periapsis.git
    cd periapsis

    # Install Python dependencies
    pip install -e .

    # Configure and build
    cppython build
    ```

## Building with CMake

### Configure

```bash
cmake --preset windows-ninja-msvc  # or your preferred preset
```

Available presets are defined in `CMakePresets.json`:

| Preset | Description |
|--------|-------------|
| `windows-ninja-msvc` | Windows with Ninja and MSVC |
| `linux-ninja-gcc` | Linux with Ninja and GCC |
| `linux-ninja-clang` | Linux with Ninja and Clang |

### Build

```bash
cmake --build --preset windows-ninja-msvc
```

### Test

```bash
ctest --preset windows-ninja-msvc
```

## CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_TESTING` | `ON` | Build unit tests |
| `BUILD_DOCS` | `OFF` | Build documentation |

## Project Structure

```
periapsis/
├── src/                    # Library source
│   ├── algorithm/          # Graph algorithms
│   ├── container/          # Container implementations
│   │   ├── dynamic/        # Dynamic containers
│   │   ├── fixed/          # Fixed-capacity containers
│   │   ├── lockfree/       # Lock-free containers
│   │   └── sparse/         # Sparse containers
│   ├── math/               # Math types
│   ├── statistics/         # Statistical accumulators
│   ├── string/             # String utilities
│   ├── structure/          # Data structures
│   │   ├── automata/       # Finite state machines
│   │   ├── graph/          # Graph library
│   │   └── tree/           # Tree structures
│   ├── thread/             # Threading utilities
│   ├── type/               # Type system
│   │   ├── concepts/       # C++20 concepts
│   │   └── traits/         # Type traits
│   └── utility/            # Miscellaneous utilities
├── tests/                  # Unit tests
│   └── unit/               # GTest-based tests
├── docs/                   # Documentation
└── tool/                   # Build tooling
```

## Running Tests

All tests use Google Test:

```bash
# Run all tests
ctest --preset windows-ninja-msvc --output-on-failure

# Run specific test
ctest -R "FixedRingBuffer" --output-on-failure
```

## Code Style

The project follows these conventions:

- **Naming**: PascalCase for types, camelCase for functions/variables
- **Concepts**: snake_case following standard library convention
- **Namespaces**: lowercase, nested as appropriate
- **Documentation**: Doxygen-style comments

## Contributing

1. Fork the repository
2. Create a feature branch from `development`
3. Make your changes with tests
4. Submit a pull request

For detailed guidelines, see [CONTRIBUTING.md](https://github.com/synodic/.github/blob/stable/CONTRIBUTING.md).

## Documentation

Documentation is built with [Zensical](https://github.com/synodic/zensical) (Material for MkDocs-based):

```bash
# Serve locally
zensical serve

# Build static site
zensical build
```
