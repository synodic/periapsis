/**
 * @brief Unit tests for geometry primitives
 */

#include <gtest/gtest.h>

import std;
import synodic.periapsis;

using namespace peri::math;

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
		constexpr Ray<float> r;
		static_assert(r.origin == Vec3<float> {});
		static_assert(r.direction.z == -1.0f);	// Default looks down -Z
	}

	TEST(MathRay, Construction)
	{
		constexpr Ray r {
			Vec3 {1.0f, 2.0f, 3.0f},
			Vec3 {0.0f, 1.0f, 0.0f}
		   };
		static_assert(r.origin.x == 1.0f);
		static_assert(r.direction.y == 1.0f);
	}

	TEST(MathRay, PointAt)
	{
		constexpr Ray r {
			Vec3 {0.0f, 0.0f, 0.0f},
			Vec3 {1.0f, 0.0f, 0.0f}
		   };
		constexpr Vec3<float> p = r.At(5.0f);
		static_assert(p.x == 5.0f);
		static_assert(p.y == 0.0f);
		static_assert(p.z == 0.0f);
	}

	// ============================================================================
	// Plane Tests
	// ============================================================================

	TEST(MathPlane, NormalDistanceStorage)
	{
		constexpr NormalPlane<float> p {
			Vec3<float> {0.0f, 1.0f, 0.0f},
			5.0f
		};
		static_assert(p.Normal().y == 1.0f);
		static_assert(p.Distance() == 5.0f);
	}

	TEST(MathPlane, PointNormalStorage)
	{
		constexpr PointPlane<float> p {
			Vec3<float> {0.0f, 5.0f, 0.0f},
			Vec3<float> {0.0f, 1.0f, 0.0f}
		  };
		static_assert(p.Normal().y == 1.0f);
		static_assert(p.Distance() == 5.0f);
	}

	TEST(MathPlane, CoefficientsStorage)
	{
		// Plane: y = 5, or 0x + 1y + 0z - 5 = 0
		constexpr CoeffsPlane<float> p {0.0f, 1.0f, 0.0f, -5.0f};
		static_assert(p.Normal().y == 1.0f);
		static_assert(p.Distance() == 5.0f);
	}

	TEST(MathPlane, SignedDistanceAbove)
	{
		constexpr NormalPlane<float> p {
			Vec3<float> {0.0f, 1.0f, 0.0f},
			0.0f
		};	// y = 0 plane
		constexpr Vec3<float> point {0.0f, 3.0f, 0.0f};	 // Above plane
		constexpr float d = SignedDistance(p, point);
		static_assert(d == 3.0f);
	}

	TEST(MathPlane, SignedDistanceBelow)
	{
		constexpr NormalPlane<float> p {
			Vec3<float> {0.0f, 1.0f, 0.0f},
			0.0f
		};	// y = 0 plane
		constexpr Vec3<float> point {0.0f, -2.0f, 0.0f};  // Below plane
		constexpr float d = SignedDistance(p, point);
		static_assert(d == -2.0f);
	}

	TEST(MathPlane, ProjectPoint)
	{
		constexpr NormalPlane<float> p {
			Vec3<float> {0.0f, 1.0f, 0.0f},
			0.0f
		};	// y = 0 plane
		constexpr Vec3<float> point {5.0f, 10.0f, 3.0f};
		constexpr Vec3<float> projected = Project(p, point);
		static_assert(projected.x == 5.0f);
		static_assert(projected.y == 0.0f);
		static_assert(projected.z == 3.0f);
	}

	TEST(MathPlane, RayIntersection)
	{
		NormalPlane<float> p {
			Vec3<float> {0.0f, 1.0f, 0.0f},
			0.0f
		};	// y = 0 plane
		Ray<float> r {
			Vec3<float> {0.0f, 5.0f,	 0.0f},
			Vec3<float> {0.0f, -1.0f, 0.0f}
		   };  // Pointing down

		auto t = Intersects(p, r);
		EXPECT_TRUE(t.has_value());
		EXPECT_TRUE(ApproxEqual(t.value(), 5.0f));
	}

	TEST(MathPlane, RayParallelNoIntersection)
	{
		NormalPlane<float> p {
			Vec3<float> {0.0f, 1.0f, 0.0f},
			0.0f
		};	// y = 0 plane
		Ray<float> r {
			Vec3<float> {0.0f, 5.0f, 0.0f},
			Vec3<float> {1.0f, 0.0f, 0.0f}
		  };  // Parallel

		auto t = Intersects(p, r);
		EXPECT_FALSE(t.has_value());
	}

	TEST(MathPlane, StorageConversion)
	{
		constexpr NormalPlane<float> p1 {
			Vec3<float> {0.0f, 1.0f, 0.0f},
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
		constexpr BoundsAABB<float> box {
			Vec3<float> {-1.0f, -1.0f, -1.0f},
			Vec3<float> {1.0f,  1.0f,	 1.0f }
		  };
		static_assert(box.Min().x == -1.0f);
		static_assert(box.Max().x == 1.0f);
	}

	TEST(MathAABB, MinmaxCenterExtents)
	{
		constexpr BoundsAABB<float> box {
			Vec3<float> {-1.0f, -1.0f, -1.0f},
			Vec3<float> {1.0f,  1.0f,	 1.0f }
		  };
		constexpr Vec3<float> c = box.Center();
		constexpr Vec3<float> e = box.Extents();
		static_assert(c.x == 0.0f);
		static_assert(c.y == 0.0f);
		static_assert(c.z == 0.0f);
		static_assert(e.x == 1.0f);
		static_assert(e.y == 1.0f);
		static_assert(e.z == 1.0f);
	}

	TEST(MathAABB, CenterExtentsConstruction)
	{
		constexpr CenterAABB<float> box {
			Vec3<float> {0.0f, 0.0f, 0.0f},
			Vec3<float> {2.0f, 2.0f, 2.0f}
		  };
		static_assert(box.Center().x == 0.0f);
		static_assert(box.Extents().x == 2.0f);
		static_assert(box.Min().x == -2.0f);
		static_assert(box.Max().x == 2.0f);
	}

	TEST(MathAABB, ContainsPointInside)
	{
		constexpr BoundsAABB<float> box {
			Vec3<float> {-1.0f, -1.0f, -1.0f},
			Vec3<float> {1.0f,  1.0f,	 1.0f }
		  };
		constexpr Vec3<float> point {0.0f, 0.0f, 0.0f};
		static_assert(Contains(box, point));
	}

	TEST(MathAABB, ContainsPointOutside)
	{
		constexpr BoundsAABB<float> box {
			Vec3<float> {-1.0f, -1.0f, -1.0f},
			Vec3<float> {1.0f,  1.0f,	 1.0f }
		  };
		constexpr Vec3<float> point {5.0f, 0.0f, 0.0f};
		static_assert(!Contains(box, point));
	}

	TEST(MathAABB, ContainsCenterExtentsInside)
	{
		CenterAABB<float> box {
			Vec3<float> {0.0f, 0.0f, 0.0f},
			Vec3<float> {1.0f, 1.0f, 1.0f}
		  };
		Vec3<float> point {0.5f, 0.5f, 0.5f};
		EXPECT_TRUE(Contains(box, point));
	}

	TEST(MathAABB, IntersectsAABBOverlap)
	{
		constexpr BoundsAABB<float> a {
			Vec3<float> {0.0f, 0.0f, 0.0f},
			Vec3<float> {2.0f, 2.0f, 2.0f}
		  };
		constexpr BoundsAABB<float> b {
			Vec3<float> {1.0f, 1.0f, 1.0f},
			Vec3<float> {3.0f, 3.0f, 3.0f}
		  };
		static_assert(Intersects(a, b));
	}

	TEST(MathAABB, IntersectsAABBNoOverlap)
	{
		constexpr BoundsAABB<float> a {
			Vec3<float> {0.0f, 0.0f, 0.0f},
			Vec3<float> {1.0f, 1.0f, 1.0f}
		  };
		constexpr BoundsAABB<float> b {
			Vec3<float> {5.0f, 5.0f, 5.0f},
			Vec3<float> {6.0f, 6.0f, 6.0f}
		  };
		static_assert(!Intersects(a, b));
	}

	TEST(MathAABB, IntersectsCenterExtents)
	{
		CenterAABB<float> a {
			Vec3<float> {0.0f, 0.0f, 0.0f},
			Vec3<float> {1.0f, 1.0f, 1.0f}
		  };
		CenterAABB<float> b {
			Vec3<float> {1.5f, 0.0f, 0.0f},
			Vec3<float> {1.0f, 1.0f, 1.0f}
		  };
		EXPECT_TRUE(Intersects(a, b));	// Overlap on X axis
	}

	TEST(MathAABB, RayIntersectionHit)
	{
		BoundsAABB<float> box {
			Vec3<float> {-1.0f, -1.0f, -1.0f},
			Vec3<float> {1.0f,  1.0f,	 1.0f }
		  };
		Ray<float> r {
			Vec3<float> {-5.0f, 0.0f, 0.0f},
			Vec3<float> {1.0f,  0.0f, 0.0f}
		  };

		auto t = Intersects(box, r);
		EXPECT_TRUE(t.has_value());
		EXPECT_TRUE(ApproxEqual(t.value(), 4.0f));	// Hit at x = -1
	}

	TEST(MathAABB, RayIntersectionMiss)
	{
		BoundsAABB<float> box {
			Vec3<float> {-1.0f, -1.0f, -1.0f},
			Vec3<float> {1.0f,  1.0f,	 1.0f }
		  };
		Ray<float> r {
			Vec3<float> {-5.0f, 5.0f, 0.0f},
			Vec3<float> {1.0f,  0.0f, 0.0f}
		  };  // Above box

		auto t = Intersects(box, r);
		EXPECT_FALSE(t.has_value());
	}

	TEST(MathAABB, MergeAABBs)
	{
		constexpr BoundsAABB<float> a {
			Vec3<float> {0.0f, 0.0f, 0.0f},
			Vec3<float> {1.0f, 1.0f, 1.0f}
		  };
		constexpr BoundsAABB<float> b {
			Vec3<float> {2.0f, 2.0f, 2.0f},
			Vec3<float> {3.0f, 3.0f, 3.0f}
		  };
		constexpr BoundsAABB<float> merged = Merge(a, b);
		static_assert(merged.Min().x == 0.0f);
		static_assert(merged.Max().x == 3.0f);
	}

	TEST(MathAABB, ExpandWithPoint)
	{
		constexpr BoundsAABB<float> box {
			Vec3<float> {0.0f, 0.0f, 0.0f},
			Vec3<float> {1.0f, 1.0f, 1.0f}
		  };
		constexpr Vec3<float> point {5.0f, -2.0f, 3.0f};
		constexpr BoundsAABB<float> expanded = Expand(box, point);
		static_assert(expanded.Max().x == 5.0f);
		static_assert(expanded.Min().y == -2.0f);
		static_assert(expanded.Max().z == 3.0f);
	}

	TEST(MathAABB, SurfaceArea)
	{
		// Unit cube: 6 faces * 1 * 1 = 6
		constexpr BoundsAABB<float> box {
			Vec3<float> {0.0f, 0.0f, 0.0f},
			Vec3<float> {1.0f, 1.0f, 1.0f}
		  };
		constexpr float area = SurfaceArea(box);
		static_assert(area == 6.0f);
	}

	TEST(MathAABB, Volume)
	{
		constexpr BoundsAABB<float> box {
			Vec3<float> {0.0f, 0.0f, 0.0f},
			Vec3<float> {2.0f, 3.0f, 4.0f}
		  };
		constexpr float vol = Volume(box);
		static_assert(vol == 24.0f);  // 2 * 3 * 4
	}

	TEST(MathAABB, StorageConversion)
	{
		constexpr BoundsAABB<float> box1 {
			Vec3<float> {-1.0f, -1.0f, -1.0f},
			Vec3<float> {1.0f,  1.0f,	 1.0f }
		  };
		constexpr auto box2 = box1.As<AABBCenterExtents<float>>();
		static_assert(box2.Center().x == 0.0f);
		static_assert(box2.Extents().x == 1.0f);
	}
}
