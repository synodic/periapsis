/**
 * @brief Unit tests for quaternion type
 */

#include <gtest/gtest.h>

import std;
import synodic.periapsis;

using namespace peri::math;

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

	TEST(MathQuat, DefaultConstructionIdentity)
	{
		constexpr Quaternion<float> q;
		static_assert(q.x == 0.0f);
		static_assert(q.y == 0.0f);
		static_assert(q.z == 0.0f);
		static_assert(q.w == 1.0f);
	}

	TEST(MathQuat, ComponentConstruction)
	{
		constexpr Quaternion<float> q {1.0f, 2.0f, 3.0f, 4.0f};
		static_assert(q.x == 1.0f);
		static_assert(q.y == 2.0f);
		static_assert(q.z == 3.0f);
		static_assert(q.w == 4.0f);
	}

	TEST(MathQuat, ScalarVectorConstruction)
	{
		constexpr Quaternion<float> q {
			4.0f,
			Vec3<float> {1.0f, 2.0f, 3.0f}
		  };
		static_assert(q.x == 1.0f);
		static_assert(q.y == 2.0f);
		static_assert(q.z == 3.0f);
		static_assert(q.w == 4.0f);
	}

	TEST(MathQuat, VectorPart)
	{
		constexpr Quaternion<float> q {1.0f, 2.0f, 3.0f, 4.0f};
		constexpr Vec3<float> v = q.Vector();
		static_assert(v.x == 1.0f);
		static_assert(v.y == 2.0f);
		static_assert(v.z == 3.0f);
	}

	TEST(MathQuat, ScalarPart)
	{
		constexpr Quaternion<float> q {1.0f, 2.0f, 3.0f, 4.0f};
		static_assert(q.Scalar() == 4.0f);
	}

	TEST(MathQuat, DotProduct)
	{
		constexpr Quaternion<float> a {1.0f, 0.0f, 0.0f, 0.0f};
		constexpr Quaternion<float> b {1.0f, 0.0f, 0.0f, 0.0f};
		constexpr float d = Dot(a, b);
		static_assert(d == 1.0f);
	}

	TEST(MathQuat, LengthIdentity)
	{
		Quaternion<float> q;  // Identity
		float len = Length(q);
		EXPECT_TRUE(ApproxEqual(len, 1.0f));
	}

	TEST(MathQuat, Conjugate)
	{
		constexpr Quaternion<float> q {1.0f, 2.0f, 3.0f, 4.0f};
		constexpr Quaternion<float> c = Conjugate(q);
		static_assert(c.x == -1.0f);
		static_assert(c.y == -2.0f);
		static_assert(c.z == -3.0f);
		static_assert(c.w == 4.0f);	 // Scalar unchanged
	}

	TEST(MathQuat, MultiplicationIdentity)
	{
		constexpr Quaternion<float> q {0.5f, 0.5f, 0.5f, 0.5f};
		constexpr Quaternion<float> i;	// Identity
		constexpr Quaternion<float> result = q * i;
		static_assert(result == q);
	}

	TEST(MathQuat, RotateVectorIdentity)
	{
		constexpr Quaternion<float> q;	// Identity
		constexpr Vec3<float> v {1.0f, 0.0f, 0.0f};
		constexpr Vec3<float> result = q * v;
		static_assert(result == v);
	}

	TEST(MathQuat, Nlerp)
	{
		Quaternion<float> a;						   // Identity
		Quaternion<float> b {0.0f, 0.0f, 0.0f, 1.0f};  // Also identity (different sign)
		Quaternion<float> mid = Nlerp(a, b, 0.5f);
		// Should be normalized
		float len = Length(mid);
		EXPECT_TRUE(ApproxEqual(len, 1.0f));
	}

	TEST(MathQuat, Alignment)
	{
		static_assert(alignof(Quaternion<float>) == 16);
		static_assert(sizeof(Quaternion<float>) == 16);
	}

	TEST(MathQuat, AxisAngleConstruction)
	{
		// 90 degree rotation around Y axis
		const float angle = std::numbers::pi_v<float> / 2.0f;
		const Vec3<float> axis {0.0f, 1.0f, 0.0f};
		const Quaternion<float> q = QuatFromAxisAngle(angle, axis);

		// Rotate X axis -> should become -Z axis
		const Vec3<float> x {1.0f, 0.0f, 0.0f};
		const Vec3<float> result = q * x;

		EXPECT_TRUE(ApproxEqual(result.x, 0.0f));
		EXPECT_TRUE(ApproxEqual(result.y, 0.0f));
		EXPECT_TRUE(ApproxEqual(result.z, -1.0f));
	}

	TEST(MathQuat, MatrixRoundtrip)
	{
		// Create quaternion from axis-angle
		const float angle		  = std::numbers::pi_v<float> / 4.0f;  // 45 degrees
		const Vec3<float> axis	  = Normalize(Vec3<float> {1.0f, 1.0f, 1.0f});
		const Quaternion<float> q = QuatFromAxisAngle(angle, axis);

		// Convert to matrix and back
		const Mat3<float> m		   = QuatToMatrix3(q);
		const Quaternion<float> q2 = QuatFromMatrix(m);

		// Should be equivalent (might differ by sign)
		const float d = std::abs(Dot(q, q2));
		EXPECT_TRUE(ApproxEqual(d, 1.0f));
	}

	TEST(MathQuat, SlerpEndpoints)
	{
		const Quaternion<float> a = QuatFromAxisAngle(0.0f, Vec3<float> {0.0f, 1.0f, 0.0f});
		const Quaternion<float> b = QuatFromAxisAngle(std::numbers::pi_v<float> / 2.0f, Vec3<float> {0.0f, 1.0f, 0.0f});

		const Quaternion<float> at_0 = Slerp(a, b, 0.0f);
		const Quaternion<float> at_1 = Slerp(a, b, 1.0f);

		EXPECT_TRUE(ApproxEqual(Dot(at_0, a), 1.0f));
		EXPECT_TRUE(ApproxEqual(std::abs(Dot(at_1, b)), 1.0f));
	}
}
