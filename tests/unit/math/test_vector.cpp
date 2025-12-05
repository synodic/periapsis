/**
 * @brief Unit tests for math vector types (Vec2, Vec3, Vec4)
 */

import std;
import synodic.honesty.test;
import synodic.library;

using namespace honesty::test;
using namespace honesty::test::literals;
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

	Suite VEC2_SUITE(
		"math_vec2",
		[]() -> Generator
		{
			co_yield "default_construction"_test = [](const Requirements& requirements)
			{
				constexpr vec2 v;
				static_assert(v.x == 0.0f);
				static_assert(v.y == 0.0f);
			};

			co_yield "scalar_broadcast"_test = [](const Requirements& requirements)
			{
				constexpr vec2 v {5.0f};
				static_assert(v.x == 5.0f);
				static_assert(v.y == 5.0f);
			};

			co_yield "component_construction"_test = [](const Requirements& requirements)
			{
				constexpr vec2 v {1.0f, 2.0f};
				static_assert(v.x == 1.0f);
				static_assert(v.y == 2.0f);
			};

			co_yield "type_conversion"_test = [](const Requirements& requirements)
			{
				constexpr dvec2 d {1.5, 2.5};
				constexpr vec2 f {d};
				static_assert(f.x == 1.5f);
				static_assert(f.y == 2.5f);

				constexpr ivec2 i {d};
				static_assert(i.x == 1);
				static_assert(i.y == 2);
			};

			co_yield "array_access"_test = [](const Requirements& requirements)
			{
				constexpr vec2 v {3.0f, 4.0f};
				static_assert(v[0] == 3.0f);
				static_assert(v[1] == 4.0f);
			};

			co_yield "size"_test = [](const Requirements& requirements)
			{
				static_assert(vec2::size() == 2);
				static_assert(vec2::size_v == 2);
			};

			co_yield "addition"_test = [](const Requirements& requirements)
			{
				constexpr vec2 a {1.0f, 2.0f};
				constexpr vec2 b {3.0f, 4.0f};
				constexpr vec2 c = a + b;
				static_assert(c.x == 4.0f);
				static_assert(c.y == 6.0f);
			};

			co_yield "subtraction"_test = [](const Requirements& requirements)
			{
				constexpr vec2 a {5.0f, 7.0f};
				constexpr vec2 b {2.0f, 3.0f};
				constexpr vec2 c = a - b;
				static_assert(c.x == 3.0f);
				static_assert(c.y == 4.0f);
			};

			co_yield "multiplication"_test = [](const Requirements& requirements)
			{
				constexpr vec2 a {2.0f, 3.0f};
				constexpr vec2 b {4.0f, 5.0f};
				constexpr vec2 c = a * b;
				static_assert(c.x == 8.0f);
				static_assert(c.y == 15.0f);
			};

			co_yield "scalar_multiplication"_test = [](const Requirements& requirements)
			{
				constexpr vec2 a {2.0f, 3.0f};
				constexpr vec2 b = a * 2.0f;
				constexpr vec2 c = 3.0f * a;
				static_assert(b.x == 4.0f);
				static_assert(b.y == 6.0f);
				static_assert(c.x == 6.0f);
				static_assert(c.y == 9.0f);
			};

			co_yield "negation"_test = [](const Requirements& requirements)
			{
				constexpr vec2 a {1.0f, -2.0f};
				constexpr vec2 b = -a;
				static_assert(b.x == -1.0f);
				static_assert(b.y == 2.0f);
			};

			co_yield "dot_product"_test = [](const Requirements& requirements)
			{
				constexpr vec2 a {1.0f, 2.0f};
				constexpr vec2 b {3.0f, 4.0f};
				constexpr float d = Dot(a, b);
				static_assert(d == 11.0f);	// 1*3 + 2*4 = 11
			};

			co_yield "length_squared"_test = [](const Requirements& requirements)
			{
				constexpr vec2 v {3.0f, 4.0f};
				constexpr float len_sq = LengthSquared(v);
				static_assert(len_sq == 25.0f);	 // 3^2 + 4^2 = 25
			};

			co_yield "length"_test = [](const Requirements& requirements)
			{
				vec2 v {3.0f, 4.0f};
				float len = Length(v);
				requirements.Expect(ApproxEqual(len, 5.0f));  // sqrt(25) = 5
			};

			co_yield "normalize"_test = [](const Requirements& requirements)
			{
				vec2 v {3.0f, 4.0f};
				vec2 n = Normalize(v);
				requirements.Expect(ApproxEqual(n.x, 0.6f));  // 3/5
				requirements.Expect(ApproxEqual(n.y, 0.8f));  // 4/5
			};

			co_yield "lerp"_test = [](const Requirements& requirements)
			{
				constexpr vec2 a {0.0f, 0.0f};
				constexpr vec2 b {10.0f, 20.0f};
				constexpr vec2 mid = Lerp(a, b, 0.5f);
				static_assert(mid.x == 5.0f);
				static_assert(mid.y == 10.0f);
			};

			co_yield "min_max"_test = [](const Requirements& requirements)
			{
				constexpr vec2 a {1.0f, 5.0f};
				constexpr vec2 b {3.0f, 2.0f};
				constexpr vec2 mn = Min(a, b);
				constexpr vec2 mx = Max(a, b);
				static_assert(mn.x == 1.0f);
				static_assert(mn.y == 2.0f);
				static_assert(mx.x == 3.0f);
				static_assert(mx.y == 5.0f);
			};

			co_yield "equality"_test = [](const Requirements& requirements)
			{
				constexpr vec2 a {1.0f, 2.0f};
				constexpr vec2 b {1.0f, 2.0f};
				constexpr vec2 c {1.0f, 3.0f};
				static_assert(a == b);
				static_assert(a != c);
			};
		});
	SuiteRegistrar _vec2(VEC2_SUITE);

	// ============================================================================
	// Vec3 Tests
	// ============================================================================

	Suite VEC3_SUITE(
		"math_vec3",
		[]() -> Generator
		{
			co_yield "default_construction"_test = [](const Requirements& requirements)
			{
				constexpr vec3 v;
				static_assert(v.x == 0.0f);
				static_assert(v.y == 0.0f);
				static_assert(v.z == 0.0f);
			};

			co_yield "scalar_broadcast"_test = [](const Requirements& requirements)
			{
				constexpr vec3 v {5.0f};
				static_assert(v.x == 5.0f);
				static_assert(v.y == 5.0f);
				static_assert(v.z == 5.0f);
			};

			co_yield "component_construction"_test = [](const Requirements& requirements)
			{
				constexpr vec3 v {1.0f, 2.0f, 3.0f};
				static_assert(v.x == 1.0f);
				static_assert(v.y == 2.0f);
				static_assert(v.z == 3.0f);
			};

			co_yield "vec2_z_construction"_test = [](const Requirements& requirements)
			{
				constexpr vec2 xy {1.0f, 2.0f};
				constexpr vec3 v {xy, 3.0f};
				static_assert(v.x == 1.0f);
				static_assert(v.y == 2.0f);
				static_assert(v.z == 3.0f);
			};

			co_yield "swizzle_xy"_test = [](const Requirements& requirements)
			{
				constexpr vec3 v {1.0f, 2.0f, 3.0f};
				constexpr vec2 xy = v.xy();
				static_assert(xy.x == 1.0f);
				static_assert(xy.y == 2.0f);
			};

			co_yield "swizzle_xz"_test = [](const Requirements& requirements)
			{
				constexpr vec3 v {1.0f, 2.0f, 3.0f};
				constexpr vec2 xz = v.xz();
				static_assert(xz.x == 1.0f);
				static_assert(xz.y == 3.0f);
			};

			co_yield "size"_test = [](const Requirements& requirements)
			{
				static_assert(vec3::size() == 3);
				static_assert(vec3::size_v == 3);
			};

			co_yield "alignment"_test = [](const Requirements& requirements)
			{
				// Vec3<float> should be aligned to 16 bytes for SIMD
				static_assert(alignof(vec3) == 16);
				// But sizeof accounts for padding
				static_assert(sizeof(vec3) == 16);
			};

			co_yield "dot_product"_test = [](const Requirements& requirements)
			{
				constexpr vec3 a {1.0f, 2.0f, 3.0f};
				constexpr vec3 b {4.0f, 5.0f, 6.0f};
				constexpr float d = Dot(a, b);
				static_assert(d == 32.0f);	// 1*4 + 2*5 + 3*6 = 32
			};

			co_yield "cross_product"_test = [](const Requirements& requirements)
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
			};

			co_yield "length"_test = [](const Requirements& requirements)
			{
				vec3 v {2.0f, 3.0f, 6.0f};
				float len = Length(v);
				requirements.Expect(ApproxEqual(len, 7.0f));  // sqrt(4+9+36) = sqrt(49) = 7
			};

			co_yield "normalize"_test = [](const Requirements& requirements)
			{
				vec3 v {0.0f, 3.0f, 4.0f};
				vec3 n = Normalize(v);
				requirements.Expect(ApproxEqual(n.x, 0.0f));
				requirements.Expect(ApproxEqual(n.y, 0.6f));  // 3/5
				requirements.Expect(ApproxEqual(n.z, 0.8f));  // 4/5
			};

			co_yield "reflect"_test = [](const Requirements& requirements)
			{
				// Reflect downward vector off horizontal surface
				constexpr vec3 incident {0.0f, -1.0f, 0.0f};
				constexpr vec3 normal {0.0f, 1.0f, 0.0f};
				constexpr vec3 reflected = Reflect(incident, normal);
				static_assert(reflected.x == 0.0f);
				static_assert(reflected.y == 1.0f);
				static_assert(reflected.z == 0.0f);
			};

			co_yield "lerp"_test = [](const Requirements& requirements)
			{
				constexpr vec3 a {0.0f, 0.0f, 0.0f};
				constexpr vec3 b {10.0f, 20.0f, 30.0f};
				constexpr vec3 quarter = Lerp(a, b, 0.25f);
				static_assert(quarter.x == 2.5f);
				static_assert(quarter.y == 5.0f);
				static_assert(quarter.z == 7.5f);
			};

			co_yield "clamp"_test = [](const Requirements& requirements)
			{
				constexpr vec3 v {-1.0f, 0.5f, 2.0f};
				constexpr vec3 lo {0.0f, 0.0f, 0.0f};
				constexpr vec3 hi {1.0f, 1.0f, 1.0f};
				constexpr vec3 c = Clamp(v, lo, hi);
				static_assert(c.x == 0.0f);	 // clamped up
				static_assert(c.y == 0.5f);	 // unchanged
				static_assert(c.z == 1.0f);	 // clamped down
			};

			co_yield "equality"_test = [](const Requirements& requirements)
			{
				constexpr vec3 a {1.0f, 2.0f, 3.0f};
				constexpr vec3 b {1.0f, 2.0f, 3.0f};
				constexpr vec3 c {1.0f, 2.0f, 4.0f};
				static_assert(a == b);
				static_assert(a != c);
			};
		});
	SuiteRegistrar _vec3(VEC3_SUITE);

	// ============================================================================
	// Vec4 Tests
	// ============================================================================

	Suite VEC4_SUITE(
		"math_vec4",
		[]() -> Generator
		{
			co_yield "default_construction"_test = [](const Requirements& requirements)
			{
				constexpr vec4 v;
				static_assert(v.x == 0.0f);
				static_assert(v.y == 0.0f);
				static_assert(v.z == 0.0f);
				static_assert(v.w == 0.0f);
			};

			co_yield "scalar_broadcast"_test = [](const Requirements& requirements)
			{
				constexpr vec4 v {5.0f};
				static_assert(v.x == 5.0f);
				static_assert(v.y == 5.0f);
				static_assert(v.z == 5.0f);
				static_assert(v.w == 5.0f);
			};

			co_yield "component_construction"_test = [](const Requirements& requirements)
			{
				constexpr vec4 v {1.0f, 2.0f, 3.0f, 4.0f};
				static_assert(v.x == 1.0f);
				static_assert(v.y == 2.0f);
				static_assert(v.z == 3.0f);
				static_assert(v.w == 4.0f);
			};

			co_yield "vec3_w_construction"_test = [](const Requirements& requirements)
			{
				constexpr vec3 xyz {1.0f, 2.0f, 3.0f};
				constexpr vec4 v {xyz, 1.0f};  // Homogeneous point
				static_assert(v.x == 1.0f);
				static_assert(v.y == 2.0f);
				static_assert(v.z == 3.0f);
				static_assert(v.w == 1.0f);
			};

			co_yield "vec2_vec2_construction"_test = [](const Requirements& requirements)
			{
				constexpr vec2 xy {1.0f, 2.0f};
				constexpr vec2 zw {3.0f, 4.0f};
				constexpr vec4 v {xy, zw};
				static_assert(v.x == 1.0f);
				static_assert(v.y == 2.0f);
				static_assert(v.z == 3.0f);
				static_assert(v.w == 4.0f);
			};

			co_yield "swizzle_xyz"_test = [](const Requirements& requirements)
			{
				constexpr vec4 v {1.0f, 2.0f, 3.0f, 4.0f};
				constexpr vec3 xyz = v.xyz();
				static_assert(xyz.x == 1.0f);
				static_assert(xyz.y == 2.0f);
				static_assert(xyz.z == 3.0f);
			};

			co_yield "swizzle_zw"_test = [](const Requirements& requirements)
			{
				constexpr vec4 v {1.0f, 2.0f, 3.0f, 4.0f};
				constexpr vec2 zw = v.zw();
				static_assert(zw.x == 3.0f);
				static_assert(zw.y == 4.0f);
			};

			co_yield "size"_test = [](const Requirements& requirements)
			{
				static_assert(vec4::size() == 4);
				static_assert(vec4::size_v == 4);
			};

			co_yield "alignment"_test = [](const Requirements& requirements)
			{
				// Vec4<float> should be aligned to 16 bytes for SIMD
				static_assert(alignof(vec4) == 16);
				static_assert(sizeof(vec4) == 16);
			};

			co_yield "dot_product"_test = [](const Requirements& requirements)
			{
				constexpr vec4 a {1.0f, 2.0f, 3.0f, 4.0f};
				constexpr vec4 b {5.0f, 6.0f, 7.0f, 8.0f};
				constexpr float d = Dot(a, b);
				static_assert(d == 70.0f);	// 1*5 + 2*6 + 3*7 + 4*8 = 70
			};

			co_yield "length"_test = [](const Requirements& requirements)
			{
				vec4 v {1.0f, 2.0f, 2.0f, 4.0f};
				float len = Length(v);
				requirements.Expect(ApproxEqual(len, 5.0f));  // sqrt(1+4+4+16) = sqrt(25) = 5
			};

			co_yield "lerp"_test = [](const Requirements& requirements)
			{
				constexpr vec4 a {0.0f, 0.0f, 0.0f, 0.0f};
				constexpr vec4 b {4.0f, 8.0f, 12.0f, 16.0f};
				constexpr vec4 mid = Lerp(a, b, 0.5f);
				static_assert(mid.x == 2.0f);
				static_assert(mid.y == 4.0f);
				static_assert(mid.z == 6.0f);
				static_assert(mid.w == 8.0f);
			};

			co_yield "equality"_test = [](const Requirements& requirements)
			{
				constexpr vec4 a {1.0f, 2.0f, 3.0f, 4.0f};
				constexpr vec4 b {1.0f, 2.0f, 3.0f, 4.0f};
				constexpr vec4 c {1.0f, 2.0f, 3.0f, 5.0f};
				static_assert(a == b);
				static_assert(a != c);
			};
		});
	SuiteRegistrar _vec4(VEC4_SUITE);

	// ============================================================================
	// Integer Vector Tests
	// ============================================================================

	Suite INT_VEC_SUITE(
		"math_int_vectors",
		[]() -> Generator
		{
			co_yield "ivec2_operations"_test = [](const Requirements& requirements)
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
			};

			co_yield "uvec3_operations"_test = [](const Requirements& requirements)
			{
				constexpr uvec3 a {100u, 200u, 300u};
				constexpr uvec3 b {10u, 20u, 30u};
				constexpr uvec3 sum = a + b;

				static_assert(sum.x == 110u);
				static_assert(sum.y == 220u);
				static_assert(sum.z == 330u);
			};

			co_yield "uvec4_alignment"_test = [](const Requirements& requirements)
			{
				// uvec4 should be aligned for SIMD (16 bytes for 4x uint32)
				static_assert(alignof(uvec4) == 16);
				static_assert(sizeof(uvec4) == 16);
			};

			co_yield "integer_dot_product"_test = [](const Requirements& requirements)
			{
				constexpr ivec3 a {1, 2, 3};
				constexpr ivec3 b {4, 5, 6};
				constexpr std::int32_t d = Dot(a, b);
				static_assert(d == 32);	 // 1*4 + 2*5 + 3*6 = 32
			};
		});
	SuiteRegistrar _int_vec(INT_VEC_SUITE);

	// ============================================================================
	// Distance Tests
	// ============================================================================

	Suite DISTANCE_SUITE(
		"math_distance",
		[]() -> Generator
		{
			co_yield "vec2_distance"_test = [](const Requirements& requirements)
			{
				vec2 a {0.0f, 0.0f};
				vec2 b {3.0f, 4.0f};
				float d = Distance(a, b);
				requirements.Expect(ApproxEqual(d, 5.0f));
			};

			co_yield "vec3_distance"_test = [](const Requirements& requirements)
			{
				vec3 a {1.0f, 2.0f, 3.0f};
				vec3 b {1.0f, 2.0f, 10.0f};	 // Only z differs by 7
				float d = Distance(a, b);
				requirements.Expect(ApproxEqual(d, 7.0f));
			};
		});
	SuiteRegistrar _distance(DISTANCE_SUITE);

}  // namespace
