/**
 * @brief Geometric primitives with storage abstraction
 *
 * Provides Ray, Plane, AABB with pluggable storage backends.
 * Users can provide optimized overloads via ADL for custom storage types.
 * Prepared for C++26 reflection and std::simd integration.
 */

export module synodic.library:math.geometry;

import std;

export import :math.vector;

export namespace synodic::math
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

		Vec3<T> origin {};
		Vec3<T> direction {T {0}, T {0}, T {-1}};  // Default: looking down -Z

		constexpr Ray() noexcept = default;

		constexpr Ray(const Vec3<T>& origin, const Vec3<T>& direction) noexcept :
			origin {origin},
			direction {direction}
		{
		}

		// Get point along ray at parameter t
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

		Vec4<T> coeffs {T {0}, T {1}, T {0}, T {0}};  // Default: y = 0

		constexpr PlaneCoefficients() noexcept = default;

		constexpr explicit PlaneCoefficients(const Vec4<T>& coeffs) noexcept :
			coeffs {coeffs}
		{
		}

		constexpr PlaneCoefficients(T a, T b, T c, T d) noexcept :
			coeffs {a, b, c, d}
		{
		}

		// Unified interface
		[[nodiscard]] constexpr auto Normal() const noexcept -> Vec3<T>
		{
			return coeffs.xyz();
		}

		[[nodiscard]] constexpr auto Distance() const noexcept -> T
		{
			return -coeffs.w;
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
		constexpr Plane(Args&&... args) noexcept :
			storage {std::forward<Args>(args)...}
		{
		}

		// Unified interface (delegates to storage)
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
	// Plane Operations (ADL-friendly overloads)
	// ============================================================================

	/**
	 * @brief Signed distance from point to plane (generic)
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
		return Dot(plane.storage.coeffs, Vec4<T> {point, T {1}});
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
		constexpr AABB(Args&&... args) noexcept :
			storage {std::forward<Args>(args)...}
		{
		}

		// Unified interface
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

		// Size (full extent, not half)
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
	// AABB Operations (ADL-friendly overloads)
	// ============================================================================

	/**
	 * @brief Check if point is inside AABB (generic)
	 */
	template<aabb_storage Storage>
	[[nodiscard]] constexpr auto
		Contains(const AABB<Storage>& box, const Vec3<typename Storage::value_type>& point) noexcept -> bool
	{
		const auto mn = box.Min();
		const auto mx = box.Max();
		return point.x >= mn.x && point.x <= mx.x && point.y >= mn.y && point.y <= mx.y && point.z >= mn.z &&
			   point.z <= mx.z;
	}

	/**
	 * @brief Check if two AABBs intersect (generic)
	 */
	template<aabb_storage StorageA, aabb_storage StorageB>
	[[nodiscard]] constexpr auto Intersects(const AABB<StorageA>& a, const AABB<StorageB>& b) noexcept -> bool
	{
		const auto a_min = a.Min();
		const auto a_max = a.Max();
		const auto b_min = b.Min();
		const auto b_max = b.Max();

		return a_min.x <= b_max.x && a_max.x >= b_min.x && a_min.y <= b_max.y && a_max.y >= b_min.y &&
			   a_min.z <= b_max.z && a_max.z >= b_min.z;
	}

	/**
	 * @brief Ray-AABB intersection test (generic)
	 * @return Optional containing t parameter if hit, empty otherwise
	 */
	template<aabb_storage Storage, std::floating_point T>
	[[nodiscard]] constexpr auto Intersects(const AABB<Storage>& box, const Ray<T>& ray) noexcept -> std::optional<T>
	{
		const auto mn = box.Min();
		const auto mx = box.Max();
		const auto& o = ray.origin;
		const auto& d = ray.direction;

		T tmin = (mn.x - o.x) / d.x;
		T tmax = (mx.x - o.x) / d.x;
		if (tmin > tmax)
		{
			std::swap(tmin, tmax);
		}

		T tymin = (mn.y - o.y) / d.y;
		T tymax = (mx.y - o.y) / d.y;
		if (tymin > tymax)
		{
			std::swap(tymin, tymax);
		}

		if (tmin > tymax || tymin > tmax)
		{
			return std::nullopt;
		}

		tmin = std::max(tmin, tymin);
		tmax = std::min(tmax, tymax);

		T tzmin = (mn.z - o.z) / d.z;
		T tzmax = (mx.z - o.z) / d.z;
		if (tzmin > tzmax)
		{
			std::swap(tzmin, tzmax);
		}

		if (tmin > tzmax || tzmin > tmax)
		{
			return std::nullopt;
		}

		tmin = std::max(tmin, tzmin);

		if (tmin < T {0})
		{
			return std::nullopt;
		}

		return tmin;
	}

	/**
	 * @brief Merge two AABBs into one containing both
	 */
	template<aabb_storage StorageA, aabb_storage StorageB>
	[[nodiscard]] constexpr auto Merge(const AABB<StorageA>& a, const AABB<StorageB>& b) noexcept
		-> AABB<AABBMinMax<typename StorageA::value_type>>
	{
		using T = typename StorageA::value_type;
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
		using T = typename Storage::value_type;
		return AABB<AABBMinMax<T>> {math::Min(box.Min(), point), math::Max(box.Max(), point)};
	}

	/**
	 * @brief Compute surface area of AABB (useful for BVH)
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
		const T denom = Dot(plane.Normal(), ray.direction);

		// Ray parallel to plane
		if (std::abs(denom) < T {0.0001})
		{
			return std::nullopt;
		}

		const T t = (plane.Distance() - Dot(plane.Normal(), ray.origin)) / denom;

		// Intersection behind ray origin
		if (t < T {0})
		{
			return std::nullopt;
		}

		return t;
	}

	// ============================================================================
	// Type Aliases
	// ============================================================================

	// Ray
	using ray  = Ray<float>;
	using dray = Ray<double>;

	// Plane with default storage
	using plane		   = Plane<PlaneNormalDistance<float>>;
	using dplane	   = Plane<PlaneNormalDistance<double>>;
	using plane_coeffs = Plane<PlaneCoefficients<float>>;

	// AABB with default storage
	using aabb		  = AABB<AABBMinMax<float>>;
	using daabb		  = AABB<AABBMinMax<double>>;
	using aabb_center = AABB<AABBCenterExtents<float>>;

}  // namespace synodic::math
