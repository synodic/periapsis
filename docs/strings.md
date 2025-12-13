# Strings

String utilities provided by the Periapsis.

## HashString

A compile-time string hashing class using the FNV-1a algorithm. Provides O(1) string comparison via hash values while retaining the original string for debugging.

```cpp
#include <synodic/library>

using namespace peri;

// Construction
HashString str("hello");
constexpr HashString constStr("world");

// User-defined literal
using namespace peri;
auto hashed = "hello"_hashed;

// Comparison (O(1) via hash)
if (str == "hello"_hashed) {
    // ...
}

// Access
std::string_view view = str;           // Original string
HashString::HashType hash = str;       // Numeric hash value

// Compile-time usage
constexpr auto h1 = HashString("test");
constexpr auto h2 = HashString("test");
static_assert(h1 == h2);
```

**Key Features:**

- Fully `constexpr` - hashing at compile-time
- O(1) equality comparison via hash
- Retains original string for debugging/display
- FNV-1a 64-bit hash algorithm
- User-defined literal `"string"_hashed`

**Use Cases:**

- String interning
- Fast string lookup tables
- Event/message identifiers
- Resource naming

**Algorithm:**
Uses FNV-1a (Fowler-Noll-Vo) with:

- Offset basis: `0xcbf29ce484222325`
- Prime: `0x100000001b3`
