/**
 * @brief Unit tests for geometry primitives (Ray, Plane, AABB)
 */

#include <gtest/gtest.h>

import std;
import synodic.library;

using namespace synodic::math;

namespace
{
	template<std::floating_point T>
	bool ApproxEqual(T a, T b, T epsilon = T {0.0001})
	{
		return std::abs(a - b) < epsilon;
	}

	// ============================================================================
	// Ray Tests
	// ============================================================================

	TEST(MathRay, DefaultConstruction)
	{
		constexpr ray r;
		static_assert(r.origin == vec3 {});
		static_assert(r.direction.z == -1.0f);	// Default looks down -Z
	}

	TEST(MathRay, Construction)
	{
		constexpr ray r {
			vec3 {1.0f, 2.0f, 3.0f},
			vec3 {0.0f, 1.0f, 0.0f}
		   };
		static_assert(r.origin.x == 1.0f);
		static_assert(r.direction.y == 1.0f);
	}

	TEST(MathRay, PointAt)
	{
		constexpr ray r {
			vec3 {0.0f, 0.0f, 0.0f},
			vec3 {1.0f, 0.0f, 0.0f}
		   };
		constexpr vec3 p = r.At(5.0f);
		static_assert(p.x == 5.0f);
		static_assert(p.y == 0.0f);
		static_assert(p.z == 0.0f);
	}

	// ============================================================================
	// Plane Tests
	// ============================================================================

	TEST(MathPlane, NormalDistanceStorage)
	{
		constexpr plane p {
			vec3 {0.0f, 1.0f, 0.0f},
			5.0f
		};
		static_assert(p.Normal().y == 1.0f);
		static_assert(p.Distance() == 5.0f);
	}

	TEST(MathPlane, PointNormalStorage)
	{
		using PlanePN = Plane<PlanePointNormal<float>>;
		constexpr PlanePN p {
			vec3 {0.0f, 5.0f, 0.0f},
			vec3 {0.0f, 1.0f, 0.0f}
		   };
		static_assert(p.Normal().y == 1.0f);
		static_assert(p.Distance() == 5.0f);
	}

	TEST(MathPlane, CoefficientsStorage)
	{
		// Plane: y = 5, or 0x + 1y + 0z - 5 = 0
		constexpr plane_coeffs p {0.0f, 1.0f, 0.0f, -5.0f};
		static_assert(p.Normal().y == 1.0f);
		static_assert(p.Distance() == 5.0f);
	}

	TEST(MathPlane, SignedDistanceAbove)
	{
		constexpr plane p {
			vec3 {0.0f, 1.0f, 0.0f},
			0.0f
		};	// y = 0 plane
		constexpr vec3 point {0.0f, 3.0f, 0.0f};  // Above plane
		constexpr float d = SignedDistance(p, point);
		static_assert(d == 3.0f);
	}

	TEST(MathPlane, SignedDistanceBelow)
	{
		constexpr plane p {
			vec3 {0.0f, 1.0f, 0.0f},
			0.0f
		};	// y = 0 plane
		constexpr vec3 point {0.0f, -2.0f, 0.0f};  // Below plane
		constexpr float d = SignedDistance(p, point);
		static_assert(d == -2.0f);
	}

	TEST(MathPlane, ProjectPoint)
	{
		constexpr plane p {
			vec3 {0.0f, 1.0f, 0.0f},
			0.0f
		};	// y = 0 plane
		constexpr vec3 point {5.0f, 10.0f, 3.0f};
		constexpr vec3 projected = Project(p, point);
		static_assert(projected.x == 5.0f);
		static_assert(projected.y == 0.0f);
		static_assert(projected.z == 3.0f);
	}

	TEST(MathPlane, RayIntersection)
	{
		plane p {
			vec3 {0.0f, 1.0f, 0.0f},
			0.0f
		};	// y = 0 plane
		ray r {
			vec3 {0.0f, 5.0f,  0.0f},
			vec3 {0.0f, -1.0f, 0.0f}
		};	// Pointing down

		auto t = Intersects(p, r);
		EXPECT_TRUE(t.has_value());
		EXPECT_TRUE(ApproxEqual(t.value(), 5.0f));
	}

	TEST(MathPlane, RayParallelNoIntersection)
	{
		plane p {
			vec3 {0.0f, 1.0f, 0.0f},
			0.0f
		};	// y = 0 plane
		ray r {
			vec3 {0.0f, 5.0f, 0.0f},
			vec3 {1.0f, 0.0f, 0.0f}
		   };  // Parallel

		auto t = Intersects(p, r);
		EXPECT_FALSE(t.has_value());
	}

	TEST(MathPlane, StorageConversion)
	{
		constexpr plane p1 {
			vec3 {0.0f, 1.0f, 0.0f},
			5.0f
		};
		constexpr auto p2 = p1.As<PlaneCoefficients<float>>();
		static_assert(p2.Normal().y == 1.0f);
		static_assert(p2.Distance() == 5.0f);
	}

	// ============================================================================
	// AABB Tests
	// ============================================================================

	TEST(MathAABB, MinmaxConstruction)
	{
		constexpr aabb box {
			vec3 {-1.0f, -1.0f, -1.0f},
			vec3 {1.0f,	1.0f,  1.0f }
		   };
		static_assert(box.Min().x == -1.0f);
		static_assert(box.Max().x == 1.0f);
	}

	TEST(MathAABB, MinmaxCenterExtents)
	{
		constexpr aabb box {
			vec3 {-1.0f, -1.0f, -1.0f},
			vec3 {1.0f,	1.0f,  1.0f }
		   };
		constexpr vec3 c = box.Center();
		constexpr vec3 e = box.Extents();
		static_assert(c.x == 0.0f);
		static_assert(c.y == 0.0f);
		static_assert(c.z == 0.0f);
		static_assert(e.x == 1.0f);
		static_assert(e.y == 1.0f);
		static_assert(e.z == 1.0f);
	}

	TEST(MathAABB, CenterExtentsConstruction)
	{
		constexpr aabb_center box {
			vec3 {0.0f, 0.0f, 0.0f},
			vec3 {2.0f, 2.0f, 2.0f}
		   };
		static_assert(box.Center().x == 0.0f);
		static_assert(box.Extents().x == 2.0f);
		static_assert(box.Min().x == -2.0f);
		static_assert(box.Max().x == 2.0f);
	}

	TEST(MathAABB, ContainsPointInside)
	{
		constexpr aabb box {
			vec3 {-1.0f, -1.0f, -1.0f},
			vec3 {1.0f,	1.0f,  1.0f }
		   };
		constexpr vec3 point {0.0f, 0.0f, 0.0f};
		static_assert(Contains(box, point));
	}

	TEST(MathAABB, ContainsPointOutside)
	{
		constexpr aabb box {
			vec3 {-1.0f, -1.0f, -1.0f},
			vec3 {1.0f,	1.0f,  1.0f }
		   };
		constexpr vec3 point {5.0f, 0.0f, 0.0f};
		static_assert(!Contains(box, point));
	}

	TEST(MathAABB, ContainsCenterExtentsInside)
	{
		aabb_center box {
			vec3 {0.0f, 0.0f, 0.0f},
			vec3 {1.0f, 1.0f, 1.0f}
		   };
		vec3 point {0.5f, 0.5f, 0.5f};
		EXPECT_TRUE(Contains(box, point));
	}

	TEST(MathAABB, IntersectsAABBOverlap)
	{
		constexpr aabb a {
			vec3 {0.0f, 0.0f, 0.0f},
			vec3 {2.0f, 2.0f, 2.0f}
		   };
		constexpr aabb b {
			vec3 {1.0f, 1.0f, 1.0f},
			vec3 {3.0f, 3.0f, 3.0f}
		   };
		static_assert(Intersects(a, b));
	}

	TEST(MathAABB, IntersectsAABBNoOverlap)
	{
		constexpr aabb a {
			vec3 {0.0f, 0.0f, 0.0f},
			vec3 {1.0f, 1.0f, 1.0f}
		   };
		constexpr aabb b {
			vec3 {5.0f, 5.0f, 5.0f},
			vec3 {6.0f, 6.0f, 6.0f}
		   };
		static_assert(!Intersects(a, b));
	}

	TEST(MathAABB, IntersectsCenterExtents)
	{
		aabb_center a {
			vec3 {0.0f, 0.0f, 0.0f},
			vec3 {1.0f, 1.0f, 1.0f}
		   };
		aabb_center b {
			vec3 {1.5f, 0.0f, 0.0f},
			vec3 {1.0f, 1.0f, 1.0f}
		   };
		EXPECT_TRUE(Intersects(a, b));	// Overlap on X axis
	}

	TEST(MathAABB, RayIntersectionHit)
	{
		aabb box {
			vec3 {-1.0f, -1.0f, -1.0f},
			vec3 {1.0f,	1.0f,  1.0f }
		   };
		ray r {
			vec3 {-5.0f, 0.0f, 0.0f},
			vec3 {1.0f,	0.0f, 0.0f}
		   };

		auto t = Intersects(box, r);
		EXPECT_TRUE(t.has_value());
		EXPECT_TRUE(ApproxEqual(t.value(), 4.0f));	// Hit at x = -1
	}

	TEST(MathAABB, RayIntersectionMiss)
	{
		aabb box {
			vec3 {-1.0f, -1.0f, -1.0f},
			vec3 {1.0f,	1.0f,  1.0f }
		   };
		ray r {
			vec3 {-5.0f, 5.0f, 0.0f},
			vec3 {1.0f,	0.0f, 0.0f}
		   };  // Above box

		auto t = Intersects(box, r);
		EXPECT_FALSE(t.has_value());
	}

	TEST(MathAABB, MergeAABBs)
	{
		constexpr aabb a {
			vec3 {0.0f, 0.0f, 0.0f},
			vec3 {1.0f, 1.0f, 1.0f}
		   };
		constexpr aabb b {
			vec3 {2.0f, 2.0f, 2.0f},
			vec3 {3.0f, 3.0f, 3.0f}
		   };
		constexpr aabb merged = Merge(a, b);
		static_assert(merged.Min().x == 0.0f);
		static_assert(merged.Max().x == 3.0f);
	}

	TEST(MathAABB, ExpandWithPoint)
	{
		constexpr aabb box {
			vec3 {0.0f, 0.0f, 0.0f},
			vec3 {1.0f, 1.0f, 1.0f}
		   };
		constexpr vec3 point {5.0f, -2.0f, 3.0f};
		constexpr aabb expanded = Expand(box, point);
		static_assert(expanded.Max().x == 5.0f);
		static_assert(expanded.Min().y == -2.0f);
		static_assert(expanded.Max().z == 3.0f);
	}

	TEST(MathAABB, SurfaceArea)
	{
		// Unit cube: 6 faces * 1 * 1 = 6
		constexpr aabb box {
			vec3 {0.0f, 0.0f, 0.0f},
			vec3 {1.0f, 1.0f, 1.0f}
		   };
		constexpr float area = SurfaceArea(box);
		static_assert(area == 6.0f);
	}

	TEST(MathAABB, Volume)
	{
		constexpr aabb box {
			vec3 {0.0f, 0.0f, 0.0f},
			vec3 {2.0f, 3.0f, 4.0f}
		   };
		constexpr float vol = Volume(box);
		static_assert(vol == 24.0f);  // 2 * 3 * 4
	}

	TEST(MathAABB, StorageConversion)
	{
		constexpr aabb box1 {
			vec3 {-1.0f, -1.0f, -1.0f},
			vec3 {1.0f,	1.0f,  1.0f }
		   };
		constexpr auto box2 = box1.As<AABBCenterExtents<float>>();
		static_assert(box2.Center().x == 0.0f);
		static_assert(box2.Extents().x == 1.0f);
	}

}  // namespace
