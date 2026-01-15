---
title: Strings
description: Compile-time string hashing and fixed-capacity strings
---

# Strings

String utilities provided by Periapsis for compile-time operations and fixed-capacity storage.

## HashString

A compile-time string hashing class using the FNV-1a algorithm. Provides O(1) string comparison via hash values while retaining the original string.

### Synopsis

```cpp
namespace peri {
    class HashString;
}
```

### Key Features

- **Fully constexpr** — Hashing at compile-time
- **O(1) equality** — Comparison via hash value
- **Original retained** — String view available for debugging
- **FNV-1a 64-bit** — Well-distributed hash function
- **User-defined literal** — `"string"_hashed`

### Construction

```cpp
using namespace peri;

// From string literal
HashString str("hello");
constexpr HashString constStr("world");

// User-defined literal
auto hashed = "hello"_hashed;
```

### Operations

| Method | Returns | Description |
|--------|---------|-------------|
| `operator==` | `bool` | Compare by hash (O(1)) |
| `operator!=` | `bool` | Compare by hash (O(1)) |
| `operator std::string_view` | `std::string_view` | Get original string |
| `operator HashType` | `std::uint64_t` | Get hash value |

### Example

```cpp
import synodic.periapsis;

using namespace peri;

// String comparison via hash
HashString str("hello");

if (str == "hello"_hashed) {
    // O(1) comparison via hash
}

// Access original string
std::string_view view = str;
std::cout << view << "\n";  // "hello"

// Access hash value
HashString::HashType hash = str;
std::cout << std::hex << hash << "\n";

// Compile-time usage
constexpr auto h1 = HashString("test");
constexpr auto h2 = HashString("test");
static_assert(h1 == h2);
```

### Use Cases

| Use Case | Description |
|----------|-------------|
| String interning | Deduplicate strings by hash |
| Fast lookup tables | Hash-based string maps |
| Event identifiers | Type-safe string-based events |
| Resource naming | Asset identification |
| Message dispatch | Route by string efficiently |

### Algorithm

Uses FNV-1a (Fowler-Noll-Vo) hash:

- **Offset basis**: `0xcbf29ce484222325`
- **Prime**: `0x100000001b3`

```cpp
constexpr auto fnv1a(std::string_view str) {
    std::uint64_t hash = 0xcbf29ce484222325;
    for (char c : str) {
        hash ^= static_cast<std::uint64_t>(c);
        hash *= 0x100000001b3;
    }
    return hash;
}
```

---

## InplaceString

A string with an internal buffer that mimics `std::basic_string` while supporting modifications and a smaller storage footprint.

### Synopsis

```cpp
namespace peri {
    template<
        typename CharT,
        std::size_t N,
        typename Traits = std::char_traits<CharT>
    >
    class BasicInplaceString;

    template<std::size_t N>
    using InplaceString = BasicInplaceString<char, N>;

    template<std::size_t N>
    using InplaceWString = BasicInplaceString<wchar_t, N>;
}
```

### Template Parameters

| Parameter | Description |
|-----------|-------------|
| `CharT` | Character type |
| `N` | Maximum capacity (excluding null terminator) |
| `Traits` | Character traits (default: `std::char_traits<CharT>`) |

### Key Features

- **Fixed capacity** — No heap allocation
- **Constexpr** — Usable at compile-time
- **Mutable** — Unlike `std::fixed_string` (P3094)
- **Compatible** — Works with `std::string_view`

### Construction

```cpp
using namespace peri;

// Default (empty)
InplaceString<32> str;

// From string literal
InplaceString<32> str("hello");

// From string_view
std::string_view sv = "world";
InplaceString<32> str(sv);

// Variadic characters
InplaceString<8> str('a', 'b', 'c');
```

### Capacity Operations

| Method | Description |
|--------|-------------|
| `size()` / `length()` | Current length |
| `max_size()` | Maximum capacity (`N`) |
| `empty()` | Test if empty |
| `capacity()` | Returns `N` |

### Element Access

| Method | Description |
|--------|-------------|
| `operator[](i)` | Access character |
| `at(i)` | Access with bounds check |
| `front()` | First character |
| `back()` | Last character |
| `data()` | Pointer to data |
| `c_str()` | Null-terminated pointer |

### Modifiers

| Method | Description |
|--------|-------------|
| `clear()` | Remove all characters |
| `push_back(c)` | Append character |
| `pop_back()` | Remove last character |
| `append(str)` | Append string |
| `operator+=` | Append string |
| `resize(n)` | Resize string |

### Conversion

```cpp
InplaceString<32> str("hello");

// To string_view (implicit)
std::string_view view = str;

// To std::string (explicit)
std::string stdstr(str.data(), str.size());
```

### Example

```cpp
import synodic.periapsis;

using namespace peri;

// Fixed-capacity string for embedded systems
constexpr auto build_message() {
    InplaceString<64> msg("Error: ");
    msg += "file not found";
    return msg;
}

constexpr auto msg = build_message();
static_assert(msg.size() == 21);

// Runtime usage
InplaceString<128> buffer;
buffer += "Name: ";
buffer += user_name;
buffer += "\n";
```

### Comparison with Alternatives

| Feature | `InplaceString<N>` | `std::string` | `std::fixed_string` (P3094) |
|---------|-------------------|---------------|----------------------------|
| Heap allocation | :material-close: | :material-check: | :material-close: |
| Constexpr | :material-check: | :material-check: (C++20) | :material-check: |
| Mutable | :material-check: | :material-check: | :material-close: |
| SSO | N/A | Sometimes | N/A |
| Size overhead | Minimal | ~24 bytes | Minimal |
