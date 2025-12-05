/**
 * @brief Unit tests for quaternion type
 */

import std;
import synodic.honesty.test;
import synodic.library;

using namespace honesty::test;
using namespace honesty::test::literals;
using namespace synodic::math;

namespace
{
	// Helper for floating-point comparison
	template<std::floating_point T>
	bool ApproxEqual(T a, T b, T epsilon = T {0.001})
	{
		return std::abs(a - b) < epsilon;
	}

	template<std::floating_point T>
	constexpr bool ApproxEqual(const Vec3<T>& a, const Vec3<T>& b, T epsilon = T {0.001})
	{
		return approx_equal(a.x, b.x, epsilon) && approx_equal(a.y, b.y, epsilon) && approx_equal(a.z, b.z, epsilon);
	}

	Suite QUAT_SUITE(
		"math_quat",
		[]() -> Generator
		{
			co_yield "default_construction_identity"_test = [](const Requirements& requirements)
			{
				constexpr Quat q;
				static_assert(q.x == 0.0f);
				static_assert(q.y == 0.0f);
				static_assert(q.z == 0.0f);
				static_assert(q.w == 1.0f);
			};

			co_yield "component_construction"_test = [](const Requirements& requirements)
			{
				constexpr Quat q {1.0f, 2.0f, 3.0f, 4.0f};
				static_assert(q.x == 1.0f);
				static_assert(q.y == 2.0f);
				static_assert(q.z == 3.0f);
				static_assert(q.w == 4.0f);
			};

			co_yield "scalar_vector_construction"_test = [](const Requirements& requirements)
			{
				constexpr Quat q {
					4.0f,
					vec3 {1.0f, 2.0f, 3.0f}
				   };
				static_assert(q.x == 1.0f);
				static_assert(q.y == 2.0f);
				static_assert(q.z == 3.0f);
				static_assert(q.w == 4.0f);
			};

			co_yield "vector_part"_test = [](const Requirements& requirements)
			{
				constexpr Quat q {1.0f, 2.0f, 3.0f, 4.0f};
				constexpr vec3 v = q.vector();
				static_assert(v.x == 1.0f);
				static_assert(v.y == 2.0f);
				static_assert(v.z == 3.0f);
			};

			co_yield "scalar_part"_test = [](const Requirements& requirements)
			{
				constexpr Quat q {1.0f, 2.0f, 3.0f, 4.0f};
				static_assert(q.scalar() == 4.0f);
			};

			co_yield "dot_product"_test = [](const Requirements& requirements)
			{
				constexpr Quat a {1.0f, 0.0f, 0.0f, 0.0f};
				constexpr Quat b {1.0f, 0.0f, 0.0f, 0.0f};
				constexpr float d = Dot(a, b);
				static_assert(d == 1.0f);
			};

			co_yield "length_identity"_test = [](const Requirements& requirements)
			{
				Quat q;	 // Identity
				float len = Length(q);
				requirements.Expect(ApproxEqual(len, 1.0f));
			};

			co_yield "conjugate"_test = [](const Requirements& requirements)
			{
				constexpr Quat q {1.0f, 2.0f, 3.0f, 4.0f};
				constexpr Quat c = Conjugate(q);
				static_assert(c.x == -1.0f);
				static_assert(c.y == -2.0f);
				static_assert(c.z == -3.0f);
				static_assert(c.w == 4.0f);	 // Scalar unchanged
			};

			co_yield "multiplication_identity"_test = [](const Requirements& requirements)
			{
				constexpr Quat q {0.5f, 0.5f, 0.5f, 0.5f};
				constexpr Quat i;  // Identity
				constexpr Quat result = q * i;
				static_assert(result == q);
			};

			co_yield "rotate_vector_identity"_test = [](const Requirements& requirements)
			{
				constexpr Quat q;  // Identity
				constexpr vec3 v {1.0f, 0.0f, 0.0f};
				constexpr vec3 result = q * v;
				static_assert(result == v);
			};

			co_yield "nlerp"_test = [](const Requirements& requirements)
			{
				Quat a;	 // Identity
				Quat b {0.0f, 0.0f, 0.0f, 1.0f};  // Also identity (different sign)
				Quat mid = Nlerp(a, b, 0.5f);
				// Should be normalized
				float len = Length(mid);
				requirements.Expect(ApproxEqual(len, 1.0f));
			};

			co_yield "alignment"_test = [](const Requirements& requirements)
			{
				static_assert(alignof(Quat) == 16);
				static_assert(sizeof(Quat) == 16);
			};

			co_yield "axis_angle_construction"_test = [](const Requirements& requirements)
			{
				// 90 degree rotation around Y axis
				const float angle = std::numbers::pi_v<float> / 2.0f;
				const vec3 axis {0.0f, 1.0f, 0.0f};
				const Quat q = QuatFromAxisAngle(angle, axis);

				// Rotate X axis -> should become -Z axis
				const vec3 x {1.0f, 0.0f, 0.0f};
				const vec3 result = q * x;

				requirements.Expect(ApproxEqual(result.x, 0.0f));
				requirements.Expect(ApproxEqual(result.y, 0.0f));
				requirements.Expect(ApproxEqual(result.z, -1.0f));
			};

			co_yield "matrix_roundtrip"_test = [](const Requirements& requirements)
			{
				// Create quaternion from axis-angle
				const float angle = std::numbers::pi_v<float> / 4.0f;  // 45 degrees
				const vec3 axis	  = Normalize(vec3 {1.0f, 1.0f, 1.0f});
				const Quat q	  = QuatFromAxisAngle(angle, axis);

				// Convert to matrix and back
				const mat3 m  = QuatToMatrix3(q);
				const Quat q2 = QuatFromMatrix(m);

				// Should be equivalent (might differ by sign)
				const float d = std::abs(Dot(q, q2));
				requirements.Expect(ApproxEqual(d, 1.0f));
			};

			co_yield "slerp_endpoints"_test = [](const Requirements& requirements)
			{
				const Quat a = QuatFromAxisAngle(0.0f, vec3 {0.0f, 1.0f, 0.0f});
				const Quat b = QuatFromAxisAngle(std::numbers::pi_v<float> / 2.0f, vec3 {0.0f, 1.0f, 0.0f});

				const Quat at_0 = Slerp(a, b, 0.0f);
				const Quat at_1 = Slerp(a, b, 1.0f);

				requirements.Expect(ApproxEqual(Dot(at_0, a), 1.0f));
				requirements.Expect(ApproxEqual(std::abs(Dot(at_1, b)), 1.0f));
			};
		});
	SuiteRegistrar _quat(QUAT_SUITE);

}  // namespace
