# Math

The Synodic Template Library provides constexpr-capable, SIMD-ready vector and matrix types for graphics programming and general mathematical computation.

## Vector Types

### Overview

All vector types feature:

- **Fully constexpr** - usable at compile-time
- **Named member access** - `v.x`, `v.y`, `v.z`, `v.w`
- **SIMD-ready alignment** - `Vec3`/`Vec4` aligned to 16 bytes (for `float`)
- **Type conversion** - explicit conversion between different component types
- **Swizzle accessors** - `.xy()`, `.xyz()`, `.zw()` etc.

### ADL-Based Algorithm Specialization

Users can provide optimized overloads for custom storage types:

```cpp
// Your custom packed storage
struct MyPackedAABB {
    using value_type = float;
    // ... compressed representation

    constexpr auto get_min() const noexcept -> vec3;
    constexpr auto get_max() const noexcept -> vec3;
    constexpr auto get_center() const noexcept -> vec3;
    constexpr auto get_extents() const noexcept -> vec3;
};

// Your optimized intersection (found via ADL)
[[nodiscard]] constexpr auto intersects(
    const AABB<MyPackedAABB>& box,
    const ray& r
) noexcept -> std::optional<float>
{
    // Custom SIMD-optimized implementation
}
```

The library provides optimized overloads for built-in storage types (e.g., `CenterExtents` uses abs-based comparisons).

---

## C++26 Preparation

The math module is designed for seamless C++26 integration:

### std::simd Ready

All types use proper alignment for direct SIMD register loads:

- `Vec3<float>`: 16-byte aligned (padded to 4 components internally)
- `Vec4<float>`, `Mat4<float>`: 16-byte aligned
- `Quat<float>`: 16-byte aligned

### Reflection Ready

Simple struct layouts enable future reflection-based:

- Automatic AoS ↔ SoA conversion
- Serialization
- Shader binding generation
