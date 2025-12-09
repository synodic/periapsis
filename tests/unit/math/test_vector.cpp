/**
 * @brief Unit tests for math vector types (Vec2, Vec3, Vec4)
 */

#include <gtest/gtest.h>

import std;
import synodic.library;

using namespace synodic::math;

namespace
{
	// Helper for floating-point comparison (needed for runtime tests since std::sqrt is not constexpr)
	template<std::floating_point T>
	bool ApproxEqual(T a, T b, T epsilon = T {0.0001})
	{
		return std::abs(a - b) < epsilon;
	}

	// ============================================================================
	// Vec2 Tests
	// ============================================================================

	TEST(MathVec2, DefaultConstruction)
	{
		constexpr vec2 v;
		static_assert(v.x == 0.0f);
		static_assert(v.y == 0.0f);
	}

	TEST(MathVec2, ScalarBroadcast)
	{
		constexpr vec2 v {5.0f};
		static_assert(v.x == 5.0f);
		static_assert(v.y == 5.0f);
	}

	TEST(MathVec2, ComponentConstruction)
	{
		constexpr vec2 v {1.0f, 2.0f};
		static_assert(v.x == 1.0f);
		static_assert(v.y == 2.0f);
	}

	TEST(MathVec2, TypeConversion)
	{
		constexpr dvec2 d {1.5, 2.5};
		constexpr vec2 f {d};
		static_assert(f.x == 1.5f);
		static_assert(f.y == 2.5f);

		constexpr ivec2 i {d};
		static_assert(i.x == 1);
		static_assert(i.y == 2);
	}

	TEST(MathVec2, ArrayAccess)
	{
		constexpr vec2 v {3.0f, 4.0f};
		static_assert(v[0] == 3.0f);
		static_assert(v[1] == 4.0f);
	}

	TEST(MathVec2, Size)
	{
		static_assert(vec2::size() == 2);
		static_assert(vec2::size_v == 2);
	}

	TEST(MathVec2, Addition)
	{
		constexpr vec2 a {1.0f, 2.0f};
		constexpr vec2 b {3.0f, 4.0f};
		constexpr vec2 c = a + b;
		static_assert(c.x == 4.0f);
		static_assert(c.y == 6.0f);
	}

	TEST(MathVec2, Subtraction)
	{
		constexpr vec2 a {5.0f, 7.0f};
		constexpr vec2 b {2.0f, 3.0f};
		constexpr vec2 c = a - b;
		static_assert(c.x == 3.0f);
		static_assert(c.y == 4.0f);
	}

	TEST(MathVec2, Multiplication)
	{
		constexpr vec2 a {2.0f, 3.0f};
		constexpr vec2 b {4.0f, 5.0f};
		constexpr vec2 c = a * b;
		static_assert(c.x == 8.0f);
		static_assert(c.y == 15.0f);
	}

	TEST(MathVec2, ScalarMultiplication)
	{
		constexpr vec2 a {2.0f, 3.0f};
		constexpr vec2 b = a * 2.0f;
		constexpr vec2 c = 3.0f * a;
		static_assert(b.x == 4.0f);
		static_assert(b.y == 6.0f);
		static_assert(c.x == 6.0f);
		static_assert(c.y == 9.0f);
	}

	TEST(MathVec2, Negation)
	{
		constexpr vec2 a {1.0f, -2.0f};
		constexpr vec2 b = -a;
		static_assert(b.x == -1.0f);
		static_assert(b.y == 2.0f);
	}

	TEST(MathVec2, DotProduct)
	{
		constexpr vec2 a {1.0f, 2.0f};
		constexpr vec2 b {3.0f, 4.0f};
		constexpr float d = Dot(a, b);
		static_assert(d == 11.0f);	// 1*3 + 2*4 = 11
	}

	TEST(MathVec2, LengthSquared)
	{
		constexpr vec2 v {3.0f, 4.0f};
		constexpr float len_sq = LengthSquared(v);
		static_assert(len_sq == 25.0f);	 // 3^2 + 4^2 = 25
	}

	TEST(MathVec2, Length)
	{
		vec2 v {3.0f, 4.0f};
		float len = Length(v);
		EXPECT_TRUE(ApproxEqual(len, 5.0f));  // sqrt(25) = 5
	}

	TEST(MathVec2, Normalize)
	{
		vec2 v {3.0f, 4.0f};
		vec2 n = Normalize(v);
		EXPECT_TRUE(ApproxEqual(n.x, 0.6f));  // 3/5
		EXPECT_TRUE(ApproxEqual(n.y, 0.8f));  // 4/5
	}

	TEST(MathVec2, Lerp)
	{
		constexpr vec2 a {0.0f, 0.0f};
		constexpr vec2 b {10.0f, 20.0f};
		constexpr vec2 mid = Lerp(a, b, 0.5f);
		static_assert(mid.x == 5.0f);
		static_assert(mid.y == 10.0f);
	}

	TEST(MathVec2, MinMax)
	{
		constexpr vec2 a {1.0f, 5.0f};
		constexpr vec2 b {3.0f, 2.0f};
		constexpr vec2 mn = Min(a, b);
		constexpr vec2 mx = Max(a, b);
		static_assert(mn.x == 1.0f);
		static_assert(mn.y == 2.0f);
		static_assert(mx.x == 3.0f);
		static_assert(mx.y == 5.0f);
	}

	TEST(MathVec2, Equality)
	{
		constexpr vec2 a {1.0f, 2.0f};
		constexpr vec2 b {1.0f, 2.0f};
		constexpr vec2 c {1.0f, 3.0f};
		static_assert(a == b);
		static_assert(a != c);
	}

	// ============================================================================
	// Vec3 Tests
	// ============================================================================

	TEST(MathVec3, DefaultConstruction)
	{
		constexpr vec3 v;
		static_assert(v.x == 0.0f);
		static_assert(v.y == 0.0f);
		static_assert(v.z == 0.0f);
	}

	TEST(MathVec3, ScalarBroadcast)
	{
		constexpr vec3 v {5.0f};
		static_assert(v.x == 5.0f);
		static_assert(v.y == 5.0f);
		static_assert(v.z == 5.0f);
	}

	TEST(MathVec3, ComponentConstruction)
	{
		constexpr vec3 v {1.0f, 2.0f, 3.0f};
		static_assert(v.x == 1.0f);
		static_assert(v.y == 2.0f);
		static_assert(v.z == 3.0f);
	}

	TEST(MathVec3, Vec2ZConstruction)
	{
		constexpr vec2 xy {1.0f, 2.0f};
		constexpr vec3 v {xy, 3.0f};
		static_assert(v.x == 1.0f);
		static_assert(v.y == 2.0f);
		static_assert(v.z == 3.0f);
	}

	TEST(MathVec3, SwizzleXY)
	{
		constexpr vec3 v {1.0f, 2.0f, 3.0f};
		constexpr vec2 xy = v.xy();
		static_assert(xy.x == 1.0f);
		static_assert(xy.y == 2.0f);
	}

	TEST(MathVec3, SwizzleXZ)
	{
		constexpr vec3 v {1.0f, 2.0f, 3.0f};
		constexpr vec2 xz = v.xz();
		static_assert(xz.x == 1.0f);
		static_assert(xz.y == 3.0f);
	}

	TEST(MathVec3, Size)
	{
		static_assert(vec3::size() == 3);
		static_assert(vec3::size_v == 3);
	}

	TEST(MathVec3, Alignment)
	{
		// Vec3<float> should be aligned to 16 bytes for SIMD
		static_assert(alignof(vec3) == 16);
		// But sizeof accounts for padding
		static_assert(sizeof(vec3) == 16);
	}

	TEST(MathVec3, DotProduct)
	{
		constexpr vec3 a {1.0f, 2.0f, 3.0f};
		constexpr vec3 b {4.0f, 5.0f, 6.0f};
		constexpr float d = Dot(a, b);
		static_assert(d == 32.0f);	// 1*4 + 2*5 + 3*6 = 32
	}

	TEST(MathVec3, CrossProduct)
	{
		// Standard basis vectors
		constexpr vec3 x {1.0f, 0.0f, 0.0f};
		constexpr vec3 y {0.0f, 1.0f, 0.0f};
		constexpr vec3 z {0.0f, 0.0f, 1.0f};

		constexpr vec3 xy = Cross(x, y);
		static_assert(xy.x == 0.0f);
		static_assert(xy.y == 0.0f);
		static_assert(xy.z == 1.0f);  // x cross y = z

		constexpr vec3 yz = Cross(y, z);
		static_assert(yz.x == 1.0f);  // y cross z = x
		static_assert(yz.y == 0.0f);
		static_assert(yz.z == 0.0f);
	}

	TEST(MathVec3, Length)
	{
		vec3 v {2.0f, 3.0f, 6.0f};
		float len = Length(v);
		EXPECT_TRUE(ApproxEqual(len, 7.0f));  // sqrt(4+9+36) = sqrt(49) = 7
	}

	TEST(MathVec3, Normalize)
	{
		vec3 v {0.0f, 3.0f, 4.0f};
		vec3 n = Normalize(v);
		EXPECT_TRUE(ApproxEqual(n.x, 0.0f));
		EXPECT_TRUE(ApproxEqual(n.y, 0.6f));  // 3/5
		EXPECT_TRUE(ApproxEqual(n.z, 0.8f));  // 4/5
	}

	TEST(MathVec3, Reflect)
	{
		// Reflect downward vector off horizontal surface
		constexpr vec3 incident {0.0f, -1.0f, 0.0f};
		constexpr vec3 normal {0.0f, 1.0f, 0.0f};
		constexpr vec3 reflected = Reflect(incident, normal);
		static_assert(reflected.x == 0.0f);
		static_assert(reflected.y == 1.0f);
		static_assert(reflected.z == 0.0f);
	}

	TEST(MathVec3, Lerp)
	{
		constexpr vec3 a {0.0f, 0.0f, 0.0f};
		constexpr vec3 b {10.0f, 20.0f, 30.0f};
		constexpr vec3 quarter = Lerp(a, b, 0.25f);
		static_assert(quarter.x == 2.5f);
		static_assert(quarter.y == 5.0f);
		static_assert(quarter.z == 7.5f);
	}

	TEST(MathVec3, Clamp)
	{
		constexpr vec3 v {-1.0f, 0.5f, 2.0f};
		constexpr vec3 lo {0.0f, 0.0f, 0.0f};
		constexpr vec3 hi {1.0f, 1.0f, 1.0f};
		constexpr vec3 c = Clamp(v, lo, hi);
		static_assert(c.x == 0.0f);	 // clamped up
		static_assert(c.y == 0.5f);	 // unchanged
		static_assert(c.z == 1.0f);	 // clamped down
	}

	TEST(MathVec3, Equality)
	{
		constexpr vec3 a {1.0f, 2.0f, 3.0f};
		constexpr vec3 b {1.0f, 2.0f, 3.0f};
		constexpr vec3 c {1.0f, 2.0f, 4.0f};
		static_assert(a == b);
		static_assert(a != c);
	}

	// ============================================================================
	// Vec4 Tests
	// ============================================================================

	TEST(MathVec4, DefaultConstruction)
	{
		constexpr vec4 v;
		static_assert(v.x == 0.0f);
		static_assert(v.y == 0.0f);
		static_assert(v.z == 0.0f);
		static_assert(v.w == 0.0f);
	}

	TEST(MathVec4, ScalarBroadcast)
	{
		constexpr vec4 v {5.0f};
		static_assert(v.x == 5.0f);
		static_assert(v.y == 5.0f);
		static_assert(v.z == 5.0f);
		static_assert(v.w == 5.0f);
	}

	TEST(MathVec4, ComponentConstruction)
	{
		constexpr vec4 v {1.0f, 2.0f, 3.0f, 4.0f};
		static_assert(v.x == 1.0f);
		static_assert(v.y == 2.0f);
		static_assert(v.z == 3.0f);
		static_assert(v.w == 4.0f);
	}

	TEST(MathVec4, Vec3WConstruction)
	{
		constexpr vec3 xyz {1.0f, 2.0f, 3.0f};
		constexpr vec4 v {xyz, 1.0f};  // Homogeneous point
		static_assert(v.x == 1.0f);
		static_assert(v.y == 2.0f);
		static_assert(v.z == 3.0f);
		static_assert(v.w == 1.0f);
	}

	TEST(MathVec4, Vec2Vec2Construction)
	{
		constexpr vec2 xy {1.0f, 2.0f};
		constexpr vec2 zw {3.0f, 4.0f};
		constexpr vec4 v {xy, zw};
		static_assert(v.x == 1.0f);
		static_assert(v.y == 2.0f);
		static_assert(v.z == 3.0f);
		static_assert(v.w == 4.0f);
	}

	TEST(MathVec4, SwizzleXYZ)
	{
		constexpr vec4 v {1.0f, 2.0f, 3.0f, 4.0f};
		constexpr vec3 xyz = v.xyz();
		static_assert(xyz.x == 1.0f);
		static_assert(xyz.y == 2.0f);
		static_assert(xyz.z == 3.0f);
	}

	TEST(MathVec4, SwizzleZW)
	{
		constexpr vec4 v {1.0f, 2.0f, 3.0f, 4.0f};
		constexpr vec2 zw = v.zw();
		static_assert(zw.x == 3.0f);
		static_assert(zw.y == 4.0f);
	}

	TEST(MathVec4, Size)
	{
		static_assert(vec4::size() == 4);
		static_assert(vec4::size_v == 4);
	}

	TEST(MathVec4, Alignment)
	{
		// Vec4<float> should be aligned to 16 bytes for SIMD
		static_assert(alignof(vec4) == 16);
		static_assert(sizeof(vec4) == 16);
	}

	TEST(MathVec4, DotProduct)
	{
		constexpr vec4 a {1.0f, 2.0f, 3.0f, 4.0f};
		constexpr vec4 b {5.0f, 6.0f, 7.0f, 8.0f};
		constexpr float d = Dot(a, b);
		static_assert(d == 70.0f);	// 1*5 + 2*6 + 3*7 + 4*8 = 70
	}

	TEST(MathVec4, Length)
	{
		vec4 v {1.0f, 2.0f, 2.0f, 4.0f};
		float len = Length(v);
		EXPECT_TRUE(ApproxEqual(len, 5.0f));  // sqrt(1+4+4+16) = sqrt(25) = 5
	}

	TEST(MathVec4, Lerp)
	{
		constexpr vec4 a {0.0f, 0.0f, 0.0f, 0.0f};
		constexpr vec4 b {4.0f, 8.0f, 12.0f, 16.0f};
		constexpr vec4 mid = Lerp(a, b, 0.5f);
		static_assert(mid.x == 2.0f);
		static_assert(mid.y == 4.0f);
		static_assert(mid.z == 6.0f);
		static_assert(mid.w == 8.0f);
	}

	TEST(MathVec4, Equality)
	{
		constexpr vec4 a {1.0f, 2.0f, 3.0f, 4.0f};
		constexpr vec4 b {1.0f, 2.0f, 3.0f, 4.0f};
		constexpr vec4 c {1.0f, 2.0f, 3.0f, 5.0f};
		static_assert(a == b);
		static_assert(a != c);
	}

	// ============================================================================
	// Integer Vector Tests
	// ============================================================================

	TEST(MathIntVectors, Ivec2Operations)
	{
		constexpr ivec2 a {10, 20};
		constexpr ivec2 b {3, 7};
		constexpr ivec2 sum	 = a + b;
		constexpr ivec2 diff = a - b;
		constexpr ivec2 prod = a * b;

		static_assert(sum.x == 13);
		static_assert(sum.y == 27);
		static_assert(diff.x == 7);
		static_assert(diff.y == 13);
		static_assert(prod.x == 30);
		static_assert(prod.y == 140);
	}

	TEST(MathIntVectors, Uvec3Operations)
	{
		constexpr uvec3 a {100u, 200u, 300u};
		constexpr uvec3 b {10u, 20u, 30u};
		constexpr uvec3 sum = a + b;

		static_assert(sum.x == 110u);
		static_assert(sum.y == 220u);
		static_assert(sum.z == 330u);
	}

	TEST(MathIntVectors, Uvec4Alignment)
	{
		// uvec4 should be aligned for SIMD (16 bytes for 4x uint32)
		static_assert(alignof(uvec4) == 16);
		static_assert(sizeof(uvec4) == 16);
	}

	TEST(MathIntVectors, IntegerDotProduct)
	{
		constexpr ivec3 a {1, 2, 3};
		constexpr ivec3 b {4, 5, 6};
		constexpr std::int32_t d = Dot(a, b);
		static_assert(d == 32);	 // 1*4 + 2*5 + 3*6 = 32
	}

	// ============================================================================
	// Distance Tests
	// ============================================================================

	TEST(MathDistance, Vec2Distance)
	{
		vec2 a {0.0f, 0.0f};
		vec2 b {3.0f, 4.0f};
		float d = Distance(a, b);
		EXPECT_TRUE(ApproxEqual(d, 5.0f));
	}

	TEST(MathDistance, Vec3Distance)
	{
		vec3 a {1.0f, 2.0f, 3.0f};
		vec3 b {1.0f, 2.0f, 10.0f};	 // Only z differs by 7
		float d = Distance(a, b);
		EXPECT_TRUE(ApproxEqual(d, 7.0f));
	}

}  // namespace
