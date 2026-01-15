---
title: Traits
description: Type introspection and metaprogramming utilities
---

# Type Traits

Metaprogramming utilities for type introspection.

## ConstructorParameters

A metafunction that deduces the parameter types of the shortest constructor for a given type.

### Synopsis

```cpp
namespace peri {
    template<typename T>
    using ConstructorParameters = /* implementation-defined tuple type */;
}
```

### Description

`ConstructorParameters<T>` produces a `std::tuple` containing the parameter types of `T`'s shortest constructor.

### Example

```cpp
import synodic.periapsis;

using namespace peri;

struct MyStruct {
    int a;
    float b;
    std::string c;

    MyStruct(int x, float y, std::string z) : a(x), b(y), c(z) {}
};

// Get constructor parameter types as a tuple
using Params = ConstructorParameters<MyStruct>;
// Params = std::tuple<int, float, std::string>

// Use with tuple utilities
static_assert(std::tuple_size_v<Params> == 3);
static_assert(std::same_as<std::tuple_element_t<0, Params>, int>);
static_assert(std::same_as<std::tuple_element_t<1, Params>, float>);
static_assert(std::same_as<std::tuple_element_t<2, Params>, std::string>);
```

### Use Cases

| Use Case | Description |
|----------|-------------|
| Serialization | Automatically serialize/deserialize types |
| Reflection | Runtime type inspection without full reflection |
| Factory patterns | Generic object construction |
| Testing | Generate test data for constructors |

### Example: Generic Factory

```cpp
template<typename T>
T make_default() {
    using Params = ConstructorParameters<T>;
    return std::make_from_tuple<T>(Params{});
}

auto obj = make_default<MyStruct>();
// Creates MyStruct with default-constructed parameters
```

### Implementation Notes

- Uses template metaprogramming with friend injection
- Works by attempting construction with increasing parameter counts
- Finds the constructor with the fewest parameters
- Will be replaced when C++ reflection is available

!!! note "Limitations"
    - Doesn't handle overloaded constructors well
    - May not work with all aggregate types
    - Implementation is a temporary workaround until C++ reflection

---

## Future Traits

!!! note "Planned Additions"
    The following traits are planned for future releases:

### Type Reflection

- `member_types<T>` — Get member types of an aggregate
- `member_names<T>` — Get member names (with reflection)
- `base_classes<T>` — Get base class list

### Type Manipulation

- `tuple_to_struct<Tuple, T>` — Convert tuple to struct
- `struct_to_tuple<T>` — Convert struct to tuple
- `for_each_member<T>` — Iterate over members
