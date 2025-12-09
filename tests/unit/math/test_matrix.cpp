/**
 * @brief Unit tests for matrix types (Mat3, Mat4)
 */

#include <gtest/gtest.h>

import std;
import synodic.library;

using namespace synodic::math;

namespace
{
	// ============================================================================
	// Mat3 Tests
	// ============================================================================

	TEST(MathMat3, DefaultConstructionIdentity)
	{
		constexpr mat3 m;
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
		constexpr mat3 m {2.0f};
		static_assert(m[0][0] == 2.0f);
		static_assert(m[1][1] == 2.0f);
		static_assert(m[2][2] == 2.0f);
		static_assert(m[0][1] == 0.0f);
	}

	TEST(MathMat3, ColumnConstruction)
	{
		constexpr vec3 c0 {1.0f, 2.0f, 3.0f};
		constexpr vec3 c1 {4.0f, 5.0f, 6.0f};
		constexpr vec3 c2 {7.0f, 8.0f, 9.0f};
		constexpr mat3 m {c0, c1, c2};

		static_assert(m[0] == c0);
		static_assert(m[1] == c1);
		static_assert(m[2] == c2);
	}

	TEST(MathMat3, RowExtraction)
	{
		constexpr mat3 m {
			vec3 {1.0f, 4.0f, 7.0f},
			vec3 {2.0f, 5.0f, 8.0f},
			vec3 {3.0f, 6.0f, 9.0f}
		   };
		constexpr vec3 row0 = m.row(0);
		static_assert(row0.x == 1.0f);
		static_assert(row0.y == 2.0f);
		static_assert(row0.z == 3.0f);
	}

	TEST(MathMat3, MatrixAddition)
	{
		constexpr mat3 a {2.0f};
		constexpr mat3 b {3.0f};
		constexpr mat3 c = a + b;
		static_assert(c[0][0] == 5.0f);
		static_assert(c[1][1] == 5.0f);
	}

	TEST(MathMat3, ScalarMultiplication)
	{
		constexpr mat3 m {2.0f};
		constexpr mat3 scaled = m * 3.0f;
		static_assert(scaled[0][0] == 6.0f);
		static_assert(scaled[1][1] == 6.0f);
	}

	TEST(MathMat3, MatrixVectorMultiplication)
	{
		constexpr mat3 m;  // Identity
		constexpr vec3 v {1.0f, 2.0f, 3.0f};
		constexpr vec3 result = m * v;
		static_assert(result == v);	 // Identity * v = v
	}

	TEST(MathMat3, MatrixMatrixMultiplicationIdentity)
	{
		constexpr mat3 a {
			vec3 {1.0f, 0.0f, 0.0f},
			vec3 {0.0f, 2.0f, 0.0f},
			vec3 {0.0f, 0.0f, 3.0f}
		   };
		constexpr mat3 i;  // Identity
		constexpr mat3 result = a * i;
		static_assert(result[0][0] == 1.0f);
		static_assert(result[1][1] == 2.0f);
		static_assert(result[2][2] == 3.0f);
	}

	TEST(MathMat3, Transpose)
	{
		constexpr mat3 m {
			vec3 {1.0f, 2.0f, 3.0f},
			vec3 {4.0f, 5.0f, 6.0f},
			vec3 {7.0f, 8.0f, 9.0f}
		   };
		constexpr mat3 t = Transpose(m);
		static_assert(t[0][0] == 1.0f);
		static_assert(t[0][1] == 4.0f);
		static_assert(t[0][2] == 7.0f);
		static_assert(t[1][0] == 2.0f);
	}

	TEST(MathMat3, Determinant)
	{
		constexpr mat3 m {
			vec3 {1.0f, 0.0f, 0.0f},
			vec3 {0.0f, 2.0f, 0.0f},
			vec3 {0.0f, 0.0f, 3.0f}
		   };
		constexpr float det = Determinant(m);
		static_assert(det == 6.0f);	 // 1 * 2 * 3
	}

	TEST(MathMat3, Inverse)
	{
		constexpr mat3 m {
			vec3 {2.0f, 0.0f, 0.0f},
			vec3 {0.0f, 4.0f, 0.0f},
			vec3 {0.0f, 0.0f, 8.0f}
		   };
		constexpr mat3 inv = Inverse(m);
		static_assert(inv[0][0] == 0.5f);
		static_assert(inv[1][1] == 0.25f);
		static_assert(inv[2][2] == 0.125f);
	}

	// ============================================================================
	// Mat4 Tests
	// ============================================================================

	TEST(MathMat4, DefaultConstructionIdentity)
	{
		constexpr mat4 m;
		static_assert(m[0][0] == 1.0f);
		static_assert(m[1][1] == 1.0f);
		static_assert(m[2][2] == 1.0f);
		static_assert(m[3][3] == 1.0f);
		static_assert(m[3][0] == 0.0f);
	}

	TEST(MathMat4, ConstructFromMat3)
	{
		constexpr mat3 m3 {2.0f};
		constexpr mat4 m4 {m3};
		static_assert(m4[0][0] == 2.0f);
		static_assert(m4[1][1] == 2.0f);
		static_assert(m4[2][2] == 2.0f);
		static_assert(m4[3][3] == 1.0f);  // w component is 1
	}

	TEST(MathMat4, Upper3x3Extraction)
	{
		constexpr mat4 m4 {
			vec4 {1.0f,	2.0f,  3.0f,	0.0f},
			vec4 {4.0f,	5.0f,  6.0f,	0.0f},
			vec4 {7.0f,	8.0f,  9.0f,	0.0f},
			vec4 {10.0f, 11.0f, 12.0f, 1.0f}
		};
		constexpr mat3 m3 = m4.upper3x3();
		static_assert(m3[0][0] == 1.0f);
		static_assert(m3[2][2] == 9.0f);
	}

	TEST(MathMat4, TranslationMatrix)
	{
		constexpr mat4 t = Translate(vec3 {1.0f, 2.0f, 3.0f});
		constexpr vec4 p {0.0f, 0.0f, 0.0f, 1.0f};	// Point at origin
		constexpr vec4 result = t * p;
		static_assert(result.x == 1.0f);
		static_assert(result.y == 2.0f);
		static_assert(result.z == 3.0f);
		static_assert(result.w == 1.0f);
	}

	TEST(MathMat4, ScaleMatrix)
	{
		constexpr mat4 s = Scale(vec3 {2.0f, 3.0f, 4.0f});
		constexpr vec4 p {1.0f, 1.0f, 1.0f, 1.0f};
		constexpr vec4 result = s * p;
		static_assert(result.x == 2.0f);
		static_assert(result.y == 3.0f);
		static_assert(result.z == 4.0f);
	}

	TEST(MathMat4, MatrixVectorMultiplication)
	{
		constexpr mat4 m;  // Identity
		constexpr vec4 v {1.0f, 2.0f, 3.0f, 1.0f};
		constexpr vec4 result = m * v;
		static_assert(result == v);
	}

	TEST(MathMat4, MatrixMatrixMultiplication)
	{
		constexpr mat4 t  = Translate(vec3 {1.0f, 0.0f, 0.0f});
		constexpr mat4 s  = Scale(vec3 {2.0f, 2.0f, 2.0f});
		constexpr mat4 ts = t * s;	// First scale, then translate

		constexpr vec4 p {1.0f, 0.0f, 0.0f, 1.0f};
		constexpr vec4 result = ts * p;
		// Scale: (1,0,0) -> (2,0,0), then translate: -> (3,0,0)
		static_assert(result.x == 3.0f);
	}

	TEST(MathMat4, DeterminantIdentity)
	{
		constexpr mat4 m;
		constexpr float det = Determinant(m);
		static_assert(det == 1.0f);
	}

	TEST(MathMat4, InverseDiagonal)
	{
		constexpr mat4 m {2.0f};
		constexpr mat4 inv = Inverse(m);
		static_assert(inv[0][0] == 0.5f);
		static_assert(inv[1][1] == 0.5f);
		static_assert(inv[2][2] == 0.5f);
		static_assert(inv[3][3] == 0.5f);
	}

	TEST(MathMat4, Alignment)
	{
		static_assert(alignof(mat4) == 16);
	}

}  // namespace
