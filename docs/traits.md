# Type Traits

Metaprogramming utilities for type introspection.

## ConstructorParameters

A metafunction that deduces the parameter types of the shortest constructor for a given type. Useful for reflection-like scenarios until C++ gets proper reflection.

```cpp
#include <synodic/library>

using namespace synodic;

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
```

**Implementation Notes:**
- Uses template metaprogramming with friend injection
- Works by attempting construction with increasing parameter counts
- Will be replaced when C++ reflection is available
