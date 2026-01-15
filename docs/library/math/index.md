---
title: Math
description: SIMD-ready vectors, matrices, quaternions, and geometry
---

# Math

The math module provides constexpr-capable, SIMD-ready types for graphics programming and general mathematical computation.

## Overview

<div class="grid cards" markdown>

- :material-vector-line:{ .lg .middle } __Vectors__

    ---

    `Vec2`, `Vec3`, `Vec4` with named member access and swizzle operations.

- :material-grid:{ .lg .middle } __Matrices__

    ---

    `Mat2`, `Mat3`, `Mat4` for transformations.

- :material-rotate-3d:{ .lg .middle } __Quaternions__

    ---

    `Quat` for rotation representation.

- :material-cube-outline:{ .lg .middle } __Geometry__

    ---

    Geometric primitives and operations.

</div>

## Key Features

- __Fully constexpr__ — Usable at compile-time
- __SIMD-ready alignment__ — 16-byte alignment for `Vec3`/`Vec4`/`Mat4` (float)
- __Named member access__ — `v.x`, `v.y`, `v.z`, `v.w`
- __Swizzle accessors__ — `.xy()`, `.xyz()`, `.zw()` etc.
- __Type conversion__ — Explicit conversion between component types

---

## Vector Types

### Synopsis

```cpp
namespace peri {
    template<typename T> struct Vec2;
    template<typename T> struct Vec3;
    template<typename T> struct Vec4;
}
```

### Common Type Aliases

```cpp
using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;
using Vec2i = Vec2<int>;

using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;
using Vec3i = Vec3<int>;

using Vec4f = Vec4<float>;
using Vec4d = Vec4<double>;
using Vec4i = Vec4<int>;
```

### Construction

```cpp
// Default (zero-initialized)
Vec3f v1;  // (0, 0, 0)

// Component-wise
Vec3f v2{1.0f, 2.0f, 3.0f};

// Broadcast
Vec3f v3{5.0f};  // (5, 5, 5)

// From smaller vector
Vec2f v2d{1.0f, 2.0f};
Vec3f v3d{v2d, 3.0f};  // (1, 2, 3)
```

### Member Access

```cpp
Vec4f v{1.0f, 2.0f, 3.0f, 4.0f};

// Named access
float x = v.x;
float y = v.y;
float z = v.z;
float w = v.w;

// Index access
float first = v[0];

// Swizzle
Vec2f xy = v.xy();
Vec3f xyz = v.xyz();
Vec2f zw = v.zw();
```

### Operations

| Operation | Description | Example |
|-----------|-------------|---------|
| `+`, `-`, `*`, `/` | Component-wise arithmetic | `v1 + v2` |
| `Dot(a, b)` | Dot product | `float d = Dot(v1, v2);` |
| `Cross(a, b)` | Cross product (Vec3 only) | `Vec3f n = Cross(v1, v2);` |
| `Length(v)` | Euclidean length | `float len = Length(v);` |
| `LengthSquared(v)` | Squared length | `float len2 = LengthSquared(v);` |
| `Normalize(v)` | Unit vector | `Vec3f unit = Normalize(v);` |
| `Lerp(a, b, t)` | Linear interpolation | `Vec3f mid = Lerp(v1, v2, 0.5f);` |
| `Min(a, b)` | Component-wise minimum | `Vec3f m = Min(v1, v2);` |
| `Max(a, b)` | Component-wise maximum | `Vec3f m = Max(v1, v2);` |

### Example

```cpp
import synodic.periapsis;

using namespace peri;

constexpr Vec3f compute_normal(Vec3f a, Vec3f b, Vec3f c) {
    Vec3f edge1 = b - a;
    Vec3f edge2 = c - a;
    return Normalize(Cross(edge1, edge2));
}

constexpr auto normal = compute_normal(
    Vec3f{0, 0, 0},
    Vec3f{1, 0, 0},
    Vec3f{0, 1, 0}
);
// normal ≈ (0, 0, 1)
```

---

## Matrix Types

### Synopsis

```cpp
namespace peri {
    template<typename T> struct Mat2;
    template<typename T> struct Mat3;
    template<typename T> struct Mat4;
}
```

### Common Type Aliases

```cpp
using Mat2f = Mat2<float>;
using Mat3f = Mat3<float>;
using Mat4f = Mat4<float>;
```

### Construction

```cpp
// Identity matrix
Mat4f identity = Mat4f::Identity();

// From columns
Mat4f m{col0, col1, col2, col3};

// Zero matrix
Mat4f zero{};
```

### Operations

| Operation | Description |
|-----------|-------------|
| `m * v` | Matrix-vector multiplication |
| `m1 * m2` | Matrix-matrix multiplication |
| `Transpose(m)` | Matrix transpose |
| `Inverse(m)` | Matrix inverse |
| `Determinant(m)` | Matrix determinant |

### Transformation Matrices

```cpp
// Translation
Mat4f trans = Mat4f::Translation(Vec3f{10, 20, 30});

// Rotation around axis
Mat4f rot = Mat4f::Rotation(axis, angle);

// Scale
Mat4f scale = Mat4f::Scale(Vec3f{2, 2, 2});

// Combine transformations
Mat4f transform = trans * rot * scale;
```

---

## Quaternion

### Synopsis

```cpp
namespace peri {
    template<typename T> struct Quat;
}
```

### Type Aliases

```cpp
using Quatf = Quat<float>;
using Quatd = Quat<double>;
```

### Construction

```cpp
// Identity (no rotation)
Quatf q1 = Quatf::Identity();

// From axis-angle
Quatf q2 = Quatf::FromAxisAngle(Vec3f{0, 1, 0}, radians);

// From Euler angles
Quatf q3 = Quatf::FromEuler(pitch, yaw, roll);

// From rotation matrix
Quatf q4 = Quatf::FromMatrix(rotationMatrix);
```

### Operations

| Operation | Description |
|-----------|-------------|
| `q1 * q2` | Quaternion multiplication (composition) |
| `q * v` | Rotate vector by quaternion |
| `Conjugate(q)` | Quaternion conjugate |
| `Inverse(q)` | Quaternion inverse |
| `Normalize(q)` | Unit quaternion |
| `Slerp(q1, q2, t)` | Spherical linear interpolation |
| `ToMatrix(q)` | Convert to rotation matrix |

### Example

```cpp
import synodic.periapsis;

using namespace peri;

// Smooth rotation interpolation
Quatf start = Quatf::FromAxisAngle(Vec3f{0, 1, 0}, 0.0f);
Quatf end = Quatf::FromAxisAngle(Vec3f{0, 1, 0}, 3.14159f);

// Interpolate rotation
for (float t = 0; t <= 1.0f; t += 0.1f) {
    Quatf current = Slerp(start, end, t);
    Vec3f rotated = current * Vec3f{1, 0, 0};
}
```

---

## Geometry

### Primitives

The geometry module provides common geometric primitives:

- __Ray__ — Origin and direction
- __AABB__ — Axis-aligned bounding box
- __Sphere__ — Center and radius
- __Plane__ — Normal and distance

### Operations

```cpp
// Ray-AABB intersection
auto result = Intersects(ray, aabb);

// Point in AABB
bool inside = Contains(aabb, point);

// AABB-AABB intersection
bool overlap = Intersects(aabb1, aabb2);
```

---

## SIMD Alignment

Types are aligned for efficient SIMD operations:

| Type | Alignment | Notes |
|------|-----------|-------|
| `Vec2<float>` | 8 bytes | |
| `Vec3<float>` | 16 bytes | Padded to 4 components |
| `Vec4<float>` | 16 bytes | |
| `Mat4<float>` | 16 bytes | |
| `Quat<float>` | 16 bytes | |

This enables direct loading into SIMD registers with future `std::simd` integration.

---

## ADL-Based Algorithm Specialization

Users can provide optimized overloads for custom types via ADL:

```cpp
// Your custom packed storage
struct MyPackedAABB {
    using value_type = float;
    // ... compressed representation

    constexpr auto get_min() const noexcept -> Vec3<float>;
    constexpr auto get_max() const noexcept -> Vec3<float>;
};

// Your optimized intersection (found via ADL)
[[nodiscard]] constexpr auto intersects(
    const AABB<MyPackedAABB>& box,
    const Ray& r
) noexcept -> std::optional<float> {
    // Custom SIMD-optimized implementation
}
```

---

## C++26 Preparation

The math module is designed for seamless C++26 integration:

### std::simd Ready

All types use proper alignment for direct SIMD register loads.

### Reflection Ready

Simple struct layouts enable future reflection-based:

- Automatic AoS ↔ SoA conversion
- Serialization
- Shader binding generation
