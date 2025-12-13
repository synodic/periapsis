export module synodic.periapsis:math.geometry;

import std;

export import :math.vector;

export namespace peri::math
{
	// ============================================================================
	// Ray
	// ============================================================================

	/**
	 * @brief A ray defined by origin and direction
	 * @tparam T Floating-point component type
	 */
	template<std::floating_point T>
	struct Ray
	{
		using value_type = T;

		Vec3<T> origin;
		Vec3<T> direction {T {0}, T {0}, T {-1}};  // Default: looking down -Z

		constexpr Ray() noexcept = default;

		constexpr Ray(const Vec3<T>& origin, const Vec3<T>& direction) noexcept :
			origin {origin},
			direction {direction}
		{
		}

		/**
		 * @brief Get point along ray at parameter t
		 */
		[[nodiscard]] constexpr auto At(T t) const noexcept -> Vec3<T>
		{
			return origin + direction * t;
		}

		[[nodiscard]] constexpr auto operator==(const Ray&) const noexcept -> bool = default;
	};

	/**
	 * @brief Get point along ray at parameter t
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto PointAt(const Ray<T>& ray, T t) noexcept -> Vec3<T>
	{
		return ray.At(t);
	}

	// ============================================================================
	// Plane Storage Types
	// ============================================================================

	/**
	 * @brief Plane storage: normal + distance from origin
	 * @details Plane equation: dot(normal, point) = distance
	 */
	template<std::floating_point T>
	struct PlaneNormalDistance
	{
		using value_type = T;

		Vec3<T> normal {T {0}, T {1}, T {0}};  // Default: XZ plane
		T distance {T {0}};

		constexpr PlaneNormalDistance() noexcept = default;

		constexpr PlaneNormalDistance(const Vec3<T>& normal, T distance) noexcept :
			normal {normal},
			distance {distance}
		{
		}

		// Unified interface
		[[nodiscard]] constexpr auto Normal() const noexcept -> Vec3<T>
		{
			return normal;
		}

		[[nodiscard]] constexpr auto Distance() const noexcept -> T
		{
			return distance;
		}

		[[nodiscard]] constexpr auto Point() const noexcept -> Vec3<T>
		{
			return normal * distance;
		}
	};

	/**
	 * @brief Plane storage: point on plane + normal
	 */
	template<std::floating_point T>
	struct PlanePointNormal
	{
		using value_type = T;

		Vec3<T> point {};
		Vec3<T> normal {T {0}, T {1}, T {0}};

		constexpr PlanePointNormal() noexcept = default;

		constexpr PlanePointNormal(const Vec3<T>& point, const Vec3<T>& normal) noexcept :
			point {point},
			normal {normal}
		{
		}

		// Unified interface
		[[nodiscard]] constexpr auto Normal() const noexcept -> Vec3<T>
		{
			return normal;
		}

		[[nodiscard]] constexpr auto Distance() const noexcept -> T
		{
			return Dot(normal, point);
		}

		[[nodiscard]] constexpr auto Point() const noexcept -> Vec3<T>
		{
			return point;
		}
	};

	/**
	 * @brief Plane storage: coefficients (a, b, c, d) where ax + by + cz + d = 0
	 */
	template<std::floating_point T>
	struct PlaneCoefficients
	{
		using value_type = T;

		Vec4<T> coefficients {T {0}, T {1}, T {0}, T {0}};	// Default: y = 0

		constexpr PlaneCoefficients() noexcept = default;

		constexpr explicit PlaneCoefficients(const Vec4<T>& coefficients) noexcept :
			coefficients {coefficients}
		{
		}

		constexpr PlaneCoefficients(T a, T b, T c, T d) noexcept :
			coefficients {a, b, c, d}
		{
		}

		// Unified interface
		[[nodiscard]] constexpr auto Normal() const noexcept -> Vec3<T>
		{
			return coefficients.xyz();
		}

		[[nodiscard]] constexpr auto Distance() const noexcept -> T
		{
			return -coefficients.w;
		}

		[[nodiscard]] constexpr auto Point() const noexcept -> Vec3<T>
		{
			return Normal() * Distance();
		}
	};

	// Plane storage concept
	template<typename S>
	concept plane_storage = requires(const S& s) {
		typename S::value_type;
		{ s.Normal() } -> std::convertible_to<Vec3<typename S::value_type>>;
		{ s.Distance() } -> std::convertible_to<typename S::value_type>;
	};

	/**
	 * @brief A plane with pluggable storage
	 * @tparam Storage Storage backend (PlaneNormalDistance, PlanePointNormal, PlaneCoefficients)
	 */
	template<typename Storage>
	struct Plane
	{
		using storage_type = Storage;
		using value_type   = typename Storage::value_type;

		Storage storage;

		constexpr Plane() noexcept = default;

		constexpr explicit Plane(const Storage& storage) noexcept :
			storage {storage}
		{
		}

		// Forward storage constructors
		template<typename... Args>
			requires std::constructible_from<Storage, Args...>
		constexpr explicit(false) Plane(Args&&... args) noexcept :
			storage {std::forward<Args>(args)...}
		{
		}

		[[nodiscard]] constexpr auto Normal() const noexcept
		{
			return storage.Normal();
		}

		[[nodiscard]] constexpr auto Distance() const noexcept
		{
			return storage.Distance();
		}

		[[nodiscard]] constexpr auto Point() const noexcept
		{
			return storage.Point();
		}

		// Conversion to different storage
		template<typename OtherStorage>
		[[nodiscard]] constexpr auto As() const noexcept -> Plane<OtherStorage>
		{
			using T = value_type;
			if constexpr (std::same_as<OtherStorage, PlaneNormalDistance<T>>)
			{
				return Plane<OtherStorage> {Normal(), Distance()};
			}
			else if constexpr (std::same_as<OtherStorage, PlanePointNormal<T>>)
			{
				return Plane<OtherStorage> {Point(), Normal()};
			}
			else if constexpr (std::same_as<OtherStorage, PlaneCoefficients<T>>)
			{
				return Plane<OtherStorage> {Normal().x, Normal().y, Normal().z, -Distance()};
			}
		}
	};

	// ============================================================================
	// Plane Operations
	// ============================================================================

	/**
	 * @brief Signed distance from point to plane
	 */
	template<typename Storage>
	[[nodiscard]] constexpr auto
		SignedDistance(const Plane<Storage>& plane, const Vec3<typename Storage::value_type>& point) noexcept ->
		typename Storage::value_type
	{
		return Dot(plane.Normal(), point) - plane.Distance();
	}

	/**
	 * @brief Signed distance - optimized for PlaneCoefficients (uses dot4)
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto SignedDistance(const Plane<PlaneCoefficients<T>>& plane, const Vec3<T>& point) noexcept
		-> T
	{
		// Can use single dot4 with SIMD
		return Dot(plane.storage.coefficients, Vec4<T> {point, T {1}});
	}

	/**
	 * @brief Project point onto plane
	 */
	template<typename Storage>
	[[nodiscard]] constexpr auto
		Project(const Plane<Storage>& plane, const Vec3<typename Storage::value_type>& point) noexcept
		-> Vec3<typename Storage::value_type>
	{
		return point - plane.Normal() * SignedDistance(plane, point);
	}

	// ============================================================================
	// AABB Storage Types
	// ============================================================================

	/**
	 * @brief AABB storage: min and max corners
	 */
	template<std::floating_point T>
	struct AABBMinMax
	{
		using value_type = T;

		Vec3<T> min {};
		Vec3<T> max {};

		constexpr AABBMinMax() noexcept = default;

		constexpr AABBMinMax(const Vec3<T>& min, const Vec3<T>& max) noexcept :
			min {min},
			max {max}
		{
		}

		// Unified interface
		[[nodiscard]] constexpr auto Min() const noexcept -> Vec3<T>
		{
			return min;
		}

		[[nodiscard]] constexpr auto Max() const noexcept -> Vec3<T>
		{
			return max;
		}

		[[nodiscard]] constexpr auto Center() const noexcept -> Vec3<T>
		{
			return (min + max) * T {0.5};
		}

		[[nodiscard]] constexpr auto Extents() const noexcept -> Vec3<T>
		{
			return (max - min) * T {0.5};
		}
	};

	/**
	 * @brief AABB storage: center and half-extents
	 */
	template<std::floating_point T>
	struct AABBCenterExtents
	{
		using value_type = T;

		Vec3<T> center {};
		Vec3<T> extents {};	 // Half-extents

		constexpr AABBCenterExtents() noexcept = default;

		constexpr AABBCenterExtents(const Vec3<T>& center, const Vec3<T>& extents) noexcept :
			center {center},
			extents {extents}
		{
		}

		// Unified interface
		[[nodiscard]] constexpr auto Min() const noexcept -> Vec3<T>
		{
			return center - extents;
		}

		[[nodiscard]] constexpr auto Max() const noexcept -> Vec3<T>
		{
			return center + extents;
		}

		[[nodiscard]] constexpr auto Center() const noexcept -> Vec3<T>
		{
			return center;
		}

		[[nodiscard]] constexpr auto Extents() const noexcept -> Vec3<T>
		{
			return extents;
		}
	};

	// AABB storage concept
	template<typename S>
	concept aabb_storage = requires(const S& s) {
		typename S::value_type;
		{ s.Min() } -> std::convertible_to<Vec3<typename S::value_type>>;
		{ s.Max() } -> std::convertible_to<Vec3<typename S::value_type>>;
		{ s.Center() } -> std::convertible_to<Vec3<typename S::value_type>>;
		{ s.Extents() } -> std::convertible_to<Vec3<typename S::value_type>>;
	};

	/**
	 * @brief An axis-aligned bounding box with pluggable storage
	 * @tparam Storage Storage backend (AABBMinMax, AABBCenterExtents)
	 */
	template<aabb_storage Storage>
	struct AABB
	{
		using storage_type = Storage;
		using value_type   = typename Storage::value_type;

		Storage storage;

		constexpr AABB() noexcept = default;

		constexpr explicit AABB(const Storage& storage) noexcept :
			storage {storage}
		{
		}

		// Forward storage constructors
		template<typename... Args>
			requires std::constructible_from<Storage, Args...>
		constexpr explicit(false) AABB(Args&&... args) noexcept :
			storage {std::forward<Args>(args)...}
		{
		}

		[[nodiscard]] constexpr auto Min() const noexcept
		{
			return storage.Min();
		}

		[[nodiscard]] constexpr auto Max() const noexcept
		{
			return storage.Max();
		}

		[[nodiscard]] constexpr auto Center() const noexcept
		{
			return storage.Center();
		}

		[[nodiscard]] constexpr auto Extents() const noexcept
		{
			return storage.Extents();
		}

		// Size. Full extent, not half
		[[nodiscard]] constexpr auto Size() const noexcept
		{
			return Extents() * value_type {2};
		}

		// Conversion to different storage
		template<aabb_storage OtherStorage>
		[[nodiscard]] constexpr auto As() const noexcept -> AABB<OtherStorage>
		{
			using T = value_type;
			if constexpr (std::same_as<OtherStorage, AABBMinMax<T>>)
			{
				return AABB<OtherStorage> {Min(), Max()};
			}
			else if constexpr (std::same_as<OtherStorage, AABBCenterExtents<T>>)
			{
				return AABB<OtherStorage> {Center(), Extents()};
			}
		}

		/**
		 * @brief Create an empty AABB with inverted bounds for incremental expansion
		 * @details Min is +infinity, Max is -infinity. First Expand() call sets actual bounds.
		 */
		[[nodiscard]] static constexpr auto Empty() noexcept -> AABB
		{
			using T			= value_type;
			constexpr T inf = std::numeric_limits<T>::infinity();
			if constexpr (std::same_as<Storage, AABBMinMax<T>>)
			{
				return AABB {
					Vec3<T> {inf,  inf,  inf },
					Vec3<T> {-inf, -inf, -inf}
				  };
			}
			else if constexpr (std::same_as<Storage, AABBCenterExtents<T>>)
			{
				// Center at origin, negative extents represent empty
				return AABB {
					Vec3<T> {},
					Vec3<T> {-inf, -inf, -inf}
				  };
			}
		}

		/**
		 * @brief Check if this AABB is empty (has inverted or zero bounds)
		 */
		[[nodiscard]] constexpr auto IsEmpty() const noexcept -> bool
		{
			const auto mn = Min();
			const auto mx = Max();
			return mn.x > mx.x || mn.y > mx.y || mn.z > mx.z;
		}
	};

	// ============================================================================
	// AABB Operations
	// ============================================================================

	/**
	 * @brief Check if point is inside AABB
	 */
	template<aabb_storage Storage>
		requires(!std::same_as<Storage, AABBCenterExtents<typename Storage::value_type>>)
	[[nodiscard]] constexpr auto
		Contains(const AABB<Storage>& box, const Vec3<typename Storage::value_type>& point) noexcept -> bool
	{
		const auto mn = box.Min();
		const auto mx = box.Max();
		return point.x >= mn.x && point.x <= mx.x && point.y >= mn.y && point.y <= mx.y && point.z >= mn.z &&
			   point.z <= mx.z;
	}

	/**
	 * @brief Check if point is inside AABB - optimized for AABBCenterExtents
	 * @details Uses absolute value check instead of computing min/max bounds
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Contains(const AABB<AABBCenterExtents<T>>& box, const Vec3<T>& point) noexcept -> bool
	{
		const auto diff = point - box.storage.center;
		const auto& ext = box.storage.extents;
		return std::abs(diff.x) <= ext.x && std::abs(diff.y) <= ext.y && std::abs(diff.z) <= ext.z;
	}

	/**
	 * @brief Check if two AABBs intersect
	 */
	template<aabb_storage StorageA, aabb_storage StorageB>
	[[nodiscard]] constexpr auto Intersects(const AABB<StorageA>& a, const AABB<StorageB>& b) noexcept -> bool
	{
		const auto aMin = a.Min();
		const auto aMax = a.Max();
		const auto bMin = b.Min();
		const auto bMax = b.Max();

		return aMin.x <= bMax.x && aMax.x >= bMin.x && aMin.y <= bMax.y && aMax.y >= bMin.y && aMin.z <= bMax.z &&
			   aMax.z >= bMin.z;
	}

	/**
	 * @brief Pre-computed inverse ray direction for repeated intersection tests
	 * @details Avoids repeated division in tight loops.
	 *          Size: 6 floats (24 bytes) or 6 doubles (48 bytes).
	 */
	template<std::floating_point T>
	struct RayInverse
	{
		Vec3<T> origin {};
		Vec3<T> inv_direction {};

		constexpr RayInverse() noexcept = default;

		constexpr explicit RayInverse(const Ray<T>& ray) noexcept :
			origin {ray.origin},
			inv_direction {T {1} / ray.direction.x, T {1} / ray.direction.y, T {1} / ray.direction.z}
		{
		}
	};

	/**
	 * @brief Ray-AABB intersection test
	 * @details Uses the slab method with proper handling of zero direction components.
	 *          When direction component is zero, division produces +/-inf which
	 *          correctly handles rays parallel to slab boundaries.
	 * @return Optional containing t parameter if hit, empty otherwise
	 */
	template<aabb_storage Storage, std::floating_point T>
	[[nodiscard]] constexpr auto Intersects(const AABB<Storage>& box, const Ray<T>& ray) noexcept -> std::optional<T>
	{
		const auto mn = box.Min();
		const auto mx = box.Max();
		const auto& o = ray.origin;

		// Pre-compute inverse direction. inf is fine, handles parallel rays correctly
		const Vec3<T> invD {T {1} / ray.direction.x, T {1} / ray.direction.y, T {1} / ray.direction.z};

		// Compute slab intersections
		T t1   = (mn.x - o.x) * invD.x;
		T t2   = (mx.x - o.x) * invD.x;
		T tMin = std::min(t1, t2);
		T tMax = std::max(t1, t2);

		t1	 = (mn.y - o.y) * invD.y;
		t2	 = (mx.y - o.y) * invD.y;
		tMin = std::max(tMin, std::min(t1, t2));
		tMax = std::min(tMax, std::max(t1, t2));

		t1	 = (mn.z - o.z) * invD.z;
		t2	 = (mx.z - o.z) * invD.z;
		tMin = std::max(tMin, std::min(t1, t2));
		tMax = std::min(tMax, std::max(t1, t2));

		// Check for valid intersection: tMax >= tMin and tMax >= 0
		if (tMax < tMin || tMax < T {0})
		{
			return std::nullopt;
		}

		// Return entry point (or 0 if inside box)
		return tMin >= T {0} ? tMin : T {0};
	}

	/**
	 * @brief Ray-AABB intersection with pre-computed inverse direction
	 * @details Optimized for BVH traversal where the same ray tests many boxes
	 */
	template<aabb_storage Storage, std::floating_point T>
	[[nodiscard]] constexpr auto Intersects(const AABB<Storage>& box, const RayInverse<T>& ray) noexcept
		-> std::optional<T>
	{
		const auto mn	 = box.Min();
		const auto mx	 = box.Max();
		const auto& o	 = ray.origin;
		const auto& invD = ray.inv_direction;

		T t1   = (mn.x - o.x) * invD.x;
		T t2   = (mx.x - o.x) * invD.x;
		T tMin = std::min(t1, t2);
		T tMax = std::max(t1, t2);

		t1	 = (mn.y - o.y) * invD.y;
		t2	 = (mx.y - o.y) * invD.y;
		tMin = std::max(tMin, std::min(t1, t2));
		tMax = std::min(tMax, std::max(t1, t2));

		t1	 = (mn.z - o.z) * invD.z;
		t2	 = (mx.z - o.z) * invD.z;
		tMin = std::max(tMin, std::min(t1, t2));
		tMax = std::min(tMax, std::max(t1, t2));

		if (tMax < tMin || tMax < T {0})
		{
			return std::nullopt;
		}

		return tMin >= T {0} ? tMin : T {0};
	}

	/**
	 * @brief Merge two AABBs into one containing both
	 */
	template<aabb_storage StorageA, aabb_storage StorageB>
	[[nodiscard]] constexpr auto Merge(const AABB<StorageA>& a, const AABB<StorageB>& b) noexcept
		-> AABB<AABBMinMax<typename StorageA::value_type>>
	{
		using T = StorageA::value_type;
		return AABB<AABBMinMax<T>> {math::Min(a.Min(), b.Min()), math::Max(a.Max(), b.Max())};
	}

	/**
	 * @brief Expand AABB to include a point
	 */
	template<aabb_storage Storage>
	[[nodiscard]] constexpr auto
		Expand(const AABB<Storage>& box, const Vec3<typename Storage::value_type>& point) noexcept
		-> AABB<AABBMinMax<typename Storage::value_type>>
	{
		using T = Storage::value_type;
		return AABB<AABBMinMax<T>> {math::Min(box.Min(), point), math::Max(box.Max(), point)};
	}

	/**
	 * @brief Compute surface area of AABB
	 */
	template<aabb_storage Storage>
	[[nodiscard]] constexpr auto SurfaceArea(const AABB<Storage>& box) noexcept -> typename Storage::value_type
	{
		const auto e = box.Extents() * typename Storage::value_type {2};  // Full size
		return typename Storage::value_type {2} * (e.x * e.y + e.y * e.z + e.z * e.x);
	}

	/**
	 * @brief Compute volume of AABB
	 */
	template<aabb_storage Storage>
	[[nodiscard]] constexpr auto Volume(const AABB<Storage>& box) noexcept -> typename Storage::value_type
	{
		const auto s = box.Size();
		return s.x * s.y * s.z;
	}

	// ============================================================================
	// Plane-Ray intersection
	// ============================================================================

	/**
	 * @brief Ray-Plane intersection
	 * @return Optional containing t parameter if hit, empty if parallel
	 */
	template<typename Storage, std::floating_point T>
	[[nodiscard]] constexpr auto Intersects(const Plane<Storage>& plane, const Ray<T>& ray) noexcept -> std::optional<T>
	{
		// Use machine epsilon scaled by typical values for robust parallel detection
		constexpr T epsilon = T {128} * std::numeric_limits<T>::epsilon();

		const auto normal	= plane.Normal();
		const T denominator = Dot(normal, ray.direction);

		// Ray parallel to plane
		if (std::abs(denominator) < epsilon)
		{
			return std::nullopt;
		}

		// Reuse normal instead of calling plane.Normal() twice
		const T t = (plane.Distance() - Dot(normal, ray.origin)) / denominator;

		// Intersection behind ray origin
		if (t < T {0})
		{
			return std::nullopt;
		}

		return t;
	}

	// ============================================================================
	// Template Aliases
	// ============================================================================

	// Plane with different storage backends
	template<std::floating_point T>
	using NormalPlane = Plane<PlaneNormalDistance<T>>;

	template<std::floating_point T>
	using PointPlane = Plane<PlanePointNormal<T>>;

	template<std::floating_point T>
	using CoeffsPlane = Plane<PlaneCoefficients<T>>;

	// AABB with different storage backends
	template<std::floating_point T>
	using BoundsAABB = AABB<AABBMinMax<T>>;

	template<std::floating_point T>
	using CenterAABB = AABB<AABBCenterExtents<T>>;
}
