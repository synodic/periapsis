/**
 * @brief Unit tests for matrix types (Mat3, Mat4)
 */

import std;
import synodic.honesty.test;
import synodic.library;

using namespace honesty::test;
using namespace honesty::test::literals;
using namespace synodic::math;

namespace
{
	// ============================================================================
	// Mat3 Tests
	// ============================================================================

	Suite MAT3_SUITE(
		"math_mat3",
		[]() -> Generator
		{
			co_yield "default_construction_identity"_test = [](const Requirements& requirements)
			{
				constexpr mat3 m;
				// Check diagonal is 1
				static_assert(m[0][0] == 1.0f);
				static_assert(m[1][1] == 1.0f);
				static_assert(m[2][2] == 1.0f);
				// Check off-diagonal is 0
				static_assert(m[0][1] == 0.0f);
				static_assert(m[1][0] == 0.0f);
			};

			co_yield "diagonal_construction"_test = [](const Requirements& requirements)
			{
				constexpr mat3 m {2.0f};
				static_assert(m[0][0] == 2.0f);
				static_assert(m[1][1] == 2.0f);
				static_assert(m[2][2] == 2.0f);
				static_assert(m[0][1] == 0.0f);
			};

			co_yield "column_construction"_test = [](const Requirements& requirements)
			{
				constexpr vec3 c0 {1.0f, 2.0f, 3.0f};
				constexpr vec3 c1 {4.0f, 5.0f, 6.0f};
				constexpr vec3 c2 {7.0f, 8.0f, 9.0f};
				constexpr mat3 m {c0, c1, c2};

				static_assert(m[0] == c0);
				static_assert(m[1] == c1);
				static_assert(m[2] == c2);
			};

			co_yield "row_extraction"_test = [](const Requirements& requirements)
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
			};

			co_yield "matrix_addition"_test = [](const Requirements& requirements)
			{
				constexpr mat3 a {2.0f};
				constexpr mat3 b {3.0f};
				constexpr mat3 c = a + b;
				static_assert(c[0][0] == 5.0f);
				static_assert(c[1][1] == 5.0f);
			};

			co_yield "scalar_multiplication"_test = [](const Requirements& requirements)
			{
				constexpr mat3 m {2.0f};
				constexpr mat3 scaled = m * 3.0f;
				static_assert(scaled[0][0] == 6.0f);
				static_assert(scaled[1][1] == 6.0f);
			};

			co_yield "matrix_vector_multiplication"_test = [](const Requirements& requirements)
			{
				constexpr mat3 m;  // Identity
				constexpr vec3 v {1.0f, 2.0f, 3.0f};
				constexpr vec3 result = m * v;
				static_assert(result == v);	 // Identity * v = v
			};

			co_yield "matrix_matrix_multiplication_identity"_test = [](const Requirements& requirements)
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
			};

			co_yield "transpose"_test = [](const Requirements& requirements)
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
			};

			co_yield "determinant"_test = [](const Requirements& requirements)
			{
				constexpr mat3 m {
					vec3 {1.0f, 0.0f, 0.0f},
					vec3 {0.0f, 2.0f, 0.0f},
					vec3 {0.0f, 0.0f, 3.0f}
				   };
				constexpr float det = Determinant(m);
				static_assert(det == 6.0f);	 // 1 * 2 * 3
			};

			co_yield "inverse"_test = [](const Requirements& requirements)
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
			};
		});
	SuiteRegistrar _mat3(MAT3_SUITE);

	// ============================================================================
	// Mat4 Tests
	// ============================================================================

	Suite MAT4_SUITE(
		"math_mat4",
		[]() -> Generator
		{
			co_yield "default_construction_identity"_test = [](const Requirements& requirements)
			{
				constexpr mat4 m;
				static_assert(m[0][0] == 1.0f);
				static_assert(m[1][1] == 1.0f);
				static_assert(m[2][2] == 1.0f);
				static_assert(m[3][3] == 1.0f);
				static_assert(m[3][0] == 0.0f);
			};

			co_yield "construct_from_mat3"_test = [](const Requirements& requirements)
			{
				constexpr mat3 m3 {2.0f};
				constexpr mat4 m4 {m3};
				static_assert(m4[0][0] == 2.0f);
				static_assert(m4[1][1] == 2.0f);
				static_assert(m4[2][2] == 2.0f);
				static_assert(m4[3][3] == 1.0f);  // w component is 1
			};

			co_yield "upper3x3_extraction"_test = [](const Requirements& requirements)
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
			};

			co_yield "translation_matrix"_test = [](const Requirements& requirements)
			{
				constexpr mat4 t = Translate(vec3 {1.0f, 2.0f, 3.0f});
				constexpr vec4 p {0.0f, 0.0f, 0.0f, 1.0f};	// Point at origin
				constexpr vec4 result = t * p;
				static_assert(result.x == 1.0f);
				static_assert(result.y == 2.0f);
				static_assert(result.z == 3.0f);
				static_assert(result.w == 1.0f);
			};

			co_yield "scale_matrix"_test = [](const Requirements& requirements)
			{
				constexpr mat4 s = Scale(vec3 {2.0f, 3.0f, 4.0f});
				constexpr vec4 p {1.0f, 1.0f, 1.0f, 1.0f};
				constexpr vec4 result = s * p;
				static_assert(result.x == 2.0f);
				static_assert(result.y == 3.0f);
				static_assert(result.z == 4.0f);
			};

			co_yield "matrix_vector_multiplication"_test = [](const Requirements& requirements)
			{
				constexpr mat4 m;  // Identity
				constexpr vec4 v {1.0f, 2.0f, 3.0f, 1.0f};
				constexpr vec4 result = m * v;
				static_assert(result == v);
			};

			co_yield "matrix_matrix_multiplication"_test = [](const Requirements& requirements)
			{
				constexpr mat4 t  = Translate(vec3 {1.0f, 0.0f, 0.0f});
				constexpr mat4 s  = Scale(vec3 {2.0f, 2.0f, 2.0f});
				constexpr mat4 ts = t * s;	// First scale, then translate

				constexpr vec4 p {1.0f, 0.0f, 0.0f, 1.0f};
				constexpr vec4 result = ts * p;
				// Scale: (1,0,0) -> (2,0,0), then translate: -> (3,0,0)
				static_assert(result.x == 3.0f);
			};

			co_yield "determinant_identity"_test = [](const Requirements& requirements)
			{
				constexpr mat4 m;
				constexpr float det = Determinant(m);
				static_assert(det == 1.0f);
			};

			co_yield "inverse_diagonal"_test = [](const Requirements& requirements)
			{
				constexpr mat4 m {2.0f};
				constexpr mat4 inv = Inverse(m);
				static_assert(inv[0][0] == 0.5f);
				static_assert(inv[1][1] == 0.5f);
				static_assert(inv[2][2] == 0.5f);
				static_assert(inv[3][3] == 0.5f);
			};

			co_yield "alignment"_test = [](const Requirements& requirements)
			{
				static_assert(alignof(mat4) == 16);
			};
		});
	SuiteRegistrar _mat4(MAT4_SUITE);

}  // namespace
