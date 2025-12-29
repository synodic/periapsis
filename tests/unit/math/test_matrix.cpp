/**
 * @brief Unit tests for matrix types (Mat3, Mat4)
 */

#include <gtest/gtest.h>

import std;
import synodic.periapsis;

using namespace peri::math;

namespace
{
	// ============================================================================
	// Mat3 Tests
	// ============================================================================

	TEST(MathMat3, DefaultConstructionIdentity)
	{
		constexpr Mat3<float> m;
		// Check diagonal is 1
		static_assert(m[0][0] == 1.0f);
		static_assert(m[1][1] == 1.0f);
		static_assert(m[2][2] == 1.0f);
		// Check off-diagonal is 0
		static_assert(m[0][1] == 0.0f);
		static_assert(m[1][0] == 0.0f);
	}

	TEST(MathMat3, DiagonalConstruction)
	{
		constexpr Mat3<float> m {2.0f};
		static_assert(m[0][0] == 2.0f);
		static_assert(m[1][1] == 2.0f);
		static_assert(m[2][2] == 2.0f);
		static_assert(m[0][1] == 0.0f);
	}

	TEST(MathMat3, ColumnConstruction)
	{
		constexpr Vec3<float> c0 {1.0f, 2.0f, 3.0f};
		constexpr Vec3<float> c1 {4.0f, 5.0f, 6.0f};
		constexpr Vec3<float> c2 {7.0f, 8.0f, 9.0f};
		constexpr Mat3<float> m {c0, c1, c2};

		static_assert(m[0] == c0);
		static_assert(m[1] == c1);
		static_assert(m[2] == c2);
	}

	TEST(MathMat3, RowExtraction)
	{
		constexpr Mat3<float> m {
			Vec3<float> {1.0f, 4.0f, 7.0f},
			Vec3<float> {2.0f, 5.0f, 8.0f},
			Vec3<float> {3.0f, 6.0f, 9.0f}
		  };
		constexpr Vec3<float> row0 = m.Row(0);
		static_assert(row0.x == 1.0f);
		static_assert(row0.y == 2.0f);
		static_assert(row0.z == 3.0f);
	}

	TEST(MathMat3, MatrixAddition)
	{
		constexpr Mat3<float> a {2.0f};
		constexpr Mat3<float> b {3.0f};
		constexpr Mat3<float> c = a + b;
		static_assert(c[0][0] == 5.0f);
		static_assert(c[1][1] == 5.0f);
	}

	TEST(MathMat3, ScalarMultiplication)
	{
		constexpr Mat3<float> m {2.0f};
		constexpr Mat3<float> scaled = m * 3.0f;
		static_assert(scaled[0][0] == 6.0f);
		static_assert(scaled[1][1] == 6.0f);
	}

	TEST(MathMat3, MatrixVectorMultiplication)
	{
		constexpr Mat3<float> m;  // Identity
		constexpr Vec3<float> v {1.0f, 2.0f, 3.0f};
		constexpr Vec3<float> result = m * v;
		static_assert(result == v);	 // Identity * v = v
	}

	TEST(MathMat3, MatrixMatrixMultiplicationIdentity)
	{
		constexpr Mat3<float> a {
			Vec3<float> {1.0f, 0.0f, 0.0f},
			Vec3<float> {0.0f, 2.0f, 0.0f},
			Vec3<float> {0.0f, 0.0f, 3.0f}
		  };
		constexpr Mat3<float> i;  // Identity
		constexpr Mat3<float> result = a * i;
		static_assert(result[0][0] == 1.0f);
		static_assert(result[1][1] == 2.0f);
		static_assert(result[2][2] == 3.0f);
	}

	TEST(MathMat3, Transpose)
	{
		constexpr Mat3<float> m {
			Vec3<float> {1.0f, 2.0f, 3.0f},
			Vec3<float> {4.0f, 5.0f, 6.0f},
			Vec3<float> {7.0f, 8.0f, 9.0f}
		  };
		constexpr Mat3<float> t = Transpose(m);
		static_assert(t[0][0] == 1.0f);
		static_assert(t[0][1] == 4.0f);
		static_assert(t[0][2] == 7.0f);
		static_assert(t[1][0] == 2.0f);
	}

	TEST(MathMat3, Determinant)
	{
		constexpr Mat3<float> m {
			Vec3<float> {1.0f, 0.0f, 0.0f},
			Vec3<float> {0.0f, 2.0f, 0.0f},
			Vec3<float> {0.0f, 0.0f, 3.0f}
		  };
		constexpr float det = Determinant(m);
		static_assert(det == 6.0f);	 // 1 * 2 * 3
	}

	TEST(MathMat3, Inverse)
	{
		constexpr Mat3<float> m {
			Vec3<float> {2.0f, 0.0f, 0.0f},
			Vec3<float> {0.0f, 4.0f, 0.0f},
			Vec3<float> {0.0f, 0.0f, 8.0f}
		  };
		constexpr Mat3<float> inv = Inverse(m);
		static_assert(inv[0][0] == 0.5f);
		static_assert(inv[1][1] == 0.25f);
		static_assert(inv[2][2] == 0.125f);
	}

	// ============================================================================
	// Mat4 Tests
	// ============================================================================

	TEST(MathMat4, DefaultConstructionIdentity)
	{
		constexpr Mat4<float> m;
		static_assert(m[0][0] == 1.0f);
		static_assert(m[1][1] == 1.0f);
		static_assert(m[2][2] == 1.0f);
		static_assert(m[3][3] == 1.0f);
		static_assert(m[3][0] == 0.0f);
	}

	TEST(MathMat4, ConstructFromMat3)
	{
		constexpr Mat3<float> m3 {2.0f};
		constexpr Mat4<float> m4 {m3};
		static_assert(m4[0][0] == 2.0f);
		static_assert(m4[1][1] == 2.0f);
		static_assert(m4[2][2] == 2.0f);
		static_assert(m4[3][3] == 1.0f);  // w component is 1
	}

	TEST(MathMat4, Upper3x3Extraction)
	{
		constexpr Mat4<float> m4 {
			Vec4<float> {1.0f,  2.0f,	 3.0f,  0.0f},
			Vec4<float> {4.0f,  5.0f,	 6.0f,  0.0f},
			Vec4<float> {7.0f,  8.0f,	 9.0f,  0.0f},
			Vec4<float> {10.0f, 11.0f, 12.0f, 1.0f}
		   };
		constexpr Mat3<float> m3 = m4.Upper3x3();
		static_assert(m3[0][0] == 1.0f);
		static_assert(m3[2][2] == 9.0f);
	}

	TEST(MathMat4, TranslationMatrix)
	{
		constexpr Mat4<float> t = Translate(Vec3<float> {1.0f, 2.0f, 3.0f});
		constexpr Vec4<float> p {0.0f, 0.0f, 0.0f, 1.0f};  // Point at origin
		constexpr Vec4<float> result = t * p;
		static_assert(result.x == 1.0f);
		static_assert(result.y == 2.0f);
		static_assert(result.z == 3.0f);
		static_assert(result.w == 1.0f);
	}

	TEST(MathMat4, ScaleMatrix)
	{
		constexpr Mat4<float> s = Scale(Vec3<float> {2.0f, 3.0f, 4.0f});
		constexpr Vec4<float> p {1.0f, 1.0f, 1.0f, 1.0f};
		constexpr Vec4<float> result = s * p;
		static_assert(result.x == 2.0f);
		static_assert(result.y == 3.0f);
		static_assert(result.z == 4.0f);
	}

	TEST(MathMat4, MatrixVectorMultiplication)
	{
		constexpr Mat4<float> m;  // Identity
		constexpr Vec4<float> v {1.0f, 2.0f, 3.0f, 1.0f};
		constexpr Vec4<float> result = m * v;
		static_assert(result == v);
	}

	TEST(MathMat4, MatrixMatrixMultiplication)
	{
		constexpr Mat4<float> t	 = Translate(Vec3<float> {1.0f, 0.0f, 0.0f});
		constexpr Mat4<float> s	 = Scale(Vec3<float> {2.0f, 2.0f, 2.0f});
		constexpr Mat4<float> ts = t * s;  // First scale, then translate

		constexpr Vec4<float> p {1.0f, 0.0f, 0.0f, 1.0f};
		constexpr Vec4<float> result = ts * p;
		// Scale: (1,0,0) -> (2,0,0), then translate: -> (3,0,0)
		static_assert(result.x == 3.0f);
	}

	TEST(MathMat4, DeterminantIdentity)
	{
		constexpr Mat4<float> m;
		constexpr float det = Determinant(m);
		static_assert(det == 1.0f);
	}

	TEST(MathMat4, InverseDiagonal)
	{
		constexpr Mat4<float> m {2.0f};
		constexpr Mat4<float> inv = Inverse(m);
		static_assert(inv[0][0] == 0.5f);
		static_assert(inv[1][1] == 0.5f);
		static_assert(inv[2][2] == 0.5f);
		static_assert(inv[3][3] == 0.5f);
	}

	TEST(MathMat4, Alignment)
	{
		static_assert(alignof(Mat4<float>) == 16);
	}
}
