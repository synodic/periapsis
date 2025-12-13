export module synodic.periapsis:math.vector;

import std;

export import :math.concepts;

export namespace peri::math
{
	// Forward declarations for friend functions
	template<vector_component T>
	struct Vec2;
	template<vector_component T>
	struct Vec3;
	template<vector_component T>
	struct Vec4;

	// ============================================================================
	// Vec2
	// ============================================================================

	/**
	 * @brief A 2-component vector with named members
	 * @tparam T Component type
	 */
	template<vector_component T>
	struct Vec2
	{
		using value_type                    = T;
		static constexpr std::size_t SIZE_V = 2;

		T x{};
		T y{};

		// Default constructor - zero initialized
		constexpr Vec2() noexcept = default;

		// Scalar broadcast constructor
		constexpr explicit Vec2(T scalar) noexcept :
			x{scalar},
			y{scalar}
		{
		}

		// Component constructor
		constexpr Vec2(T x, T y) noexcept :
			x{x},
			y{y}
		{
		}

		// Conversion constructor from different component type
		template<vector_component U>
			requires(!std::same_as<T, U>)
		constexpr explicit Vec2(const Vec2<U>& other) noexcept :
			x{static_cast<T>(other.x)},
			y{static_cast<T>(other.y)}
		{
		}

		// Array-style access
		[[nodiscard]] constexpr auto operator[](const std::size_t index) noexcept -> T&
		{
			if (index == 0)
			{
				return x;
			}
			return y;
		}

		[[nodiscard]] constexpr auto operator[](const std::size_t index) const noexcept -> const T&
		{
			if (index == 0)
			{
				return x;
			}
			return y;
		}

		// Size
		[[nodiscard]] static constexpr auto Size() noexcept -> std::size_t
		{
			return SIZE_V;
		}

		// Data access for interop
		[[nodiscard]] constexpr auto Data() noexcept -> T*
		{
			return &x;
		}

		[[nodiscard]] constexpr auto Data() const noexcept -> const T*
		{
			return &x;
		}

		// Arithmetic operators
		constexpr auto operator+=(const Vec2& rhs) noexcept -> Vec2&
		{
			x += rhs.x;
			y += rhs.y;
			return *this;
		}

		constexpr auto operator-=(const Vec2& rhs) noexcept -> Vec2&
		{
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}

		constexpr auto operator*=(const Vec2& rhs) noexcept -> Vec2&
		{
			x *= rhs.x;
			y *= rhs.y;
			return *this;
		}

		constexpr auto operator/=(const Vec2& rhs) noexcept -> Vec2&
		{
			x /= rhs.x;
			y /= rhs.y;
			return *this;
		}

		constexpr auto operator*=(T scalar) noexcept -> Vec2&
		{
			x *= scalar;
			y *= scalar;
			return *this;
		}

		constexpr auto operator/=(T scalar) noexcept -> Vec2&
		{
			x /= scalar;
			y /= scalar;
			return *this;
		}

		// Unary operators
		[[nodiscard]] constexpr auto operator-() const noexcept -> Vec2
		{
			return Vec2{-x, -y};
		}

		[[nodiscard]] constexpr auto operator+() const noexcept -> Vec2
		{
			return *this;
		}

		// Comparison
		[[nodiscard]] constexpr auto operator==(const Vec2&) const noexcept -> bool = default;
	};

	// Binary operators for Vec2
	template<vector_component T>
	[[nodiscard]] constexpr auto operator+(Vec2<T> lhs, const Vec2<T>& rhs) noexcept -> Vec2<T>
	{
		return lhs += rhs;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto operator-(Vec2<T> lhs, const Vec2<T>& rhs) noexcept -> Vec2<T>
	{
		return lhs -= rhs;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto operator*(Vec2<T> lhs, const Vec2<T>& rhs) noexcept -> Vec2<T>
	{
		return lhs *= rhs;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto operator/(Vec2<T> lhs, const Vec2<T>& rhs) noexcept -> Vec2<T>
	{
		return lhs /= rhs;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto operator*(Vec2<T> lhs, T scalar) noexcept -> Vec2<T>
	{
		return lhs *= scalar;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto operator*(T scalar, Vec2<T> rhs) noexcept -> Vec2<T>
	{
		return rhs *= scalar;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto operator/(Vec2<T> lhs, T scalar) noexcept -> Vec2<T>
	{
		return lhs /= scalar;
	}

	// ============================================================================
	// Vec3
	// ============================================================================

	/**
	 * @brief A 3-component vector with named members
	 * @tparam T Component type
	 * @note Internally may be padded to 4 components for SIMD alignment
	 */
	template<vector_component T>
	struct alignas(sizeof(T) * 4) Vec3
	{
		using value_type                    = T;
		static constexpr std::size_t SIZE_V = 3;

		T x{};
		T y{};
		T z{};

	private:
		// Padding for SIMD alignment - not accessible
		[[maybe_unused]] T pad_{};

	public:
		// Default constructor - zero initialized
		constexpr Vec3() noexcept = default;

		// Scalar broadcast constructor
		constexpr explicit Vec3(T scalar) noexcept :
			x{scalar},
			y{scalar},
			z{scalar}
		{
		}

		// Component constructor
		constexpr Vec3(T x, T y, T z) noexcept :
			x{x},
			y{y},
			z{z}
		{
		}

		// Construct from Vec2 + z
		constexpr Vec3(const Vec2<T>& xy, T z) noexcept :
			x{xy.x},
			y{xy.y},
			z{z}
		{
		}

		// Conversion constructor from different component type
		template<vector_component U>
			requires(!std::same_as<T, U>)
		constexpr explicit Vec3(const Vec3<U>& other) noexcept :
			x{static_cast<T>(other.x)},
			y{static_cast<T>(other.y)},
			z{static_cast<T>(other.z)}
		{
		}

		// Array-style access
		[[nodiscard]] constexpr auto operator[](const std::size_t index) noexcept -> T&
		{
			if (index == 0)
			{
				return x;
			}
			if (index == 1)
			{
				return y;
			}
			return z;
		}

		[[nodiscard]] constexpr auto operator[](const std::size_t index) const noexcept -> const T&
		{
			if (index == 0)
			{
				return x;
			}
			if (index == 1)
			{
				return y;
			}
			return z;
		}

		// Size
		[[nodiscard]] static constexpr auto Size() noexcept -> std::size_t
		{
			return SIZE_V;
		}

		// Data access for interop
		[[nodiscard]] constexpr auto Data() noexcept -> T*
		{
			return &x;
		}

		[[nodiscard]] constexpr auto Data() const noexcept -> const T*
		{
			return &x;
		}

		// Swizzle accessors
		[[nodiscard]] constexpr auto xy() const noexcept -> Vec2<T>
		{
			return Vec2<T>{x, y};
		}

		[[nodiscard]] constexpr auto xz() const noexcept -> Vec2<T>
		{
			return Vec2<T>{x, z};
		}

		[[nodiscard]] constexpr auto yz() const noexcept -> Vec2<T>
		{
			return Vec2<T>{y, z};
		}

		// Arithmetic operators
		constexpr auto operator+=(const Vec3& rhs) noexcept -> Vec3&
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			return *this;
		}

		constexpr auto operator-=(const Vec3& rhs) noexcept -> Vec3&
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			return *this;
		}

		constexpr auto operator*=(const Vec3& rhs) noexcept -> Vec3&
		{
			x *= rhs.x;
			y *= rhs.y;
			z *= rhs.z;
			return *this;
		}

		constexpr auto operator/=(const Vec3& rhs) noexcept -> Vec3&
		{
			x /= rhs.x;
			y /= rhs.y;
			z /= rhs.z;
			return *this;
		}

		constexpr auto operator*=(T scalar) noexcept -> Vec3&
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			return *this;
		}

		constexpr auto operator/=(T scalar) noexcept -> Vec3&
		{
			x /= scalar;
			y /= scalar;
			z /= scalar;
			return *this;
		}

		// Unary operators
		[[nodiscard]] constexpr auto operator-() const noexcept -> Vec3
		{
			return Vec3{-x, -y, -z};
		}

		[[nodiscard]] constexpr auto operator+() const noexcept -> Vec3
		{
			return *this;
		}

		// Comparison
		[[nodiscard]] constexpr auto operator==(const Vec3& other) const noexcept -> bool
		{
			return x == other.x && y == other.y && z == other.z;
		}
	};

	// Binary operators for Vec3
	template<vector_component T>
	[[nodiscard]] constexpr auto operator+(Vec3<T> lhs, const Vec3<T>& rhs) noexcept -> Vec3<T>
	{
		return lhs += rhs;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto operator-(Vec3<T> lhs, const Vec3<T>& rhs) noexcept -> Vec3<T>
	{
		return lhs -= rhs;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto operator*(Vec3<T> lhs, const Vec3<T>& rhs) noexcept -> Vec3<T>
	{
		return lhs *= rhs;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto operator/(Vec3<T> lhs, const Vec3<T>& rhs) noexcept -> Vec3<T>
	{
		return lhs /= rhs;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto operator*(Vec3<T> lhs, T scalar) noexcept -> Vec3<T>
	{
		return lhs *= scalar;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto operator*(T scalar, Vec3<T> rhs) noexcept -> Vec3<T>
	{
		return rhs *= scalar;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto operator/(Vec3<T> lhs, T scalar) noexcept -> Vec3<T>
	{
		return lhs /= scalar;
	}

	// ============================================================================
	// Vec4
	// ============================================================================

	/**
	 * @brief A 4-component vector with named members
	 * @tparam T Component type
	 * @note Naturally aligned for SIMD operations
	 */
	template<vector_component T>
	struct alignas(sizeof(T) * 4) Vec4
	{
		using value_type                    = T;
		static constexpr std::size_t SIZE_V = 4;

		T x{};
		T y{};
		T z{};
		T w{};

		// Default constructor - zero initialized
		constexpr Vec4() noexcept = default;

		// Scalar broadcast constructor
		constexpr explicit Vec4(T scalar) noexcept :
			x{scalar},
			y{scalar},
			z{scalar},
			w{scalar}
		{
		}

		// Component constructor
		constexpr Vec4(T x, T y, T z, T w) noexcept :
			x{x},
			y{y},
			z{z},
			w{w}
		{
		}

		// Construct from Vec3 + w
		constexpr Vec4(const Vec3<T>& xyz, T w) noexcept :
			x{xyz.x},
			y{xyz.y},
			z{xyz.z},
			w{w}
		{
		}

		// Construct from Vec2 + Vec2
		constexpr Vec4(const Vec2<T>& xy, const Vec2<T>& zw) noexcept :
			x{xy.x},
			y{xy.y},
			z{zw.x},
			w{zw.y}
		{
		}

		// Construct from Vec2 + z + w
		constexpr Vec4(const Vec2<T>& xy, T z, T w) noexcept :
			x{xy.x},
			y{xy.y},
			z{z},
			w{w}
		{
		}

		// Conversion constructor from different component type
		template<vector_component U>
			requires(!std::same_as<T, U>)
		constexpr explicit Vec4(const Vec4<U>& other) noexcept :
			x{static_cast<T>(other.x)},
			y{static_cast<T>(other.y)},
			z{static_cast<T>(other.z)},
			w{static_cast<T>(other.w)}
		{
		}

		// Array-style access
		[[nodiscard]] constexpr auto operator[](const std::size_t index) noexcept -> T&
		{
			if (index == 0)
			{
				return x;
			}
			if (index == 1)
			{
				return y;
			}
			if (index == 2)
			{
				return z;
			}
			return w;
		}

		[[nodiscard]] constexpr auto operator[](const std::size_t index) const noexcept -> const T&
		{
			if (index == 0)
			{
				return x;
			}
			if (index == 1)
			{
				return y;
			}
			if (index == 2)
			{
				return z;
			}
			return w;
		}

		// Size
		[[nodiscard]] static constexpr auto Size() noexcept -> std::size_t
		{
			return SIZE_V;
		}

		// Data access for interop
		[[nodiscard]] constexpr auto Data() noexcept -> T*
		{
			return &x;
		}

		[[nodiscard]] constexpr auto Data() const noexcept -> const T*
		{
			return &x;
		}

		// Swizzle accessors (common ones)
		[[nodiscard]] constexpr auto xy() const noexcept -> Vec2<T>
		{
			return Vec2<T>{x, y};
		}

		[[nodiscard]] constexpr auto xyz() const noexcept -> Vec3<T>
		{
			return Vec3<T>{x, y, z};
		}

		[[nodiscard]] constexpr auto xz() const noexcept -> Vec2<T>
		{
			return Vec2<T>{x, z};
		}

		[[nodiscard]] constexpr auto yz() const noexcept -> Vec2<T>
		{
			return Vec2<T>{y, z};
		}

		[[nodiscard]] constexpr auto zw() const noexcept -> Vec2<T>
		{
			return Vec2<T>{z, w};
		}

		// Arithmetic operators (compound)
		constexpr auto operator+=(const Vec4& rhs) noexcept -> Vec4&
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			w += rhs.w;
			return *this;
		}

		constexpr auto operator-=(const Vec4& rhs) noexcept -> Vec4&
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			w -= rhs.w;
			return *this;
		}

		constexpr auto operator*=(const Vec4& rhs) noexcept -> Vec4&
		{
			x *= rhs.x;
			y *= rhs.y;
			z *= rhs.z;
			w *= rhs.w;
			return *this;
		}

		constexpr auto operator/=(const Vec4& rhs) noexcept -> Vec4&
		{
			x /= rhs.x;
			y /= rhs.y;
			z /= rhs.z;
			w /= rhs.w;
			return *this;
		}

		constexpr auto operator*=(T scalar) noexcept -> Vec4&
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			w *= scalar;
			return *this;
		}

		constexpr auto operator/=(T scalar) noexcept -> Vec4&
		{
			x /= scalar;
			y /= scalar;
			z /= scalar;
			w /= scalar;
			return *this;
		}

		// Unary operators
		[[nodiscard]] constexpr auto operator-() const noexcept -> Vec4
		{
			return Vec4{-x, -y, -z, -w};
		}

		[[nodiscard]] constexpr auto operator+() const noexcept -> Vec4
		{
			return *this;
		}

		// Comparison
		[[nodiscard]] constexpr auto operator==(const Vec4&) const noexcept -> bool = default;
	};

	// Binary operators for Vec4
	template<vector_component T>
	[[nodiscard]] constexpr auto operator+(Vec4<T> lhs, const Vec4<T>& rhs) noexcept -> Vec4<T>
	{
		return lhs += rhs;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto operator-(Vec4<T> lhs, const Vec4<T>& rhs) noexcept -> Vec4<T>
	{
		return lhs -= rhs;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto operator*(Vec4<T> lhs, const Vec4<T>& rhs) noexcept -> Vec4<T>
	{
		return lhs *= rhs;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto operator/(Vec4<T> lhs, const Vec4<T>& rhs) noexcept -> Vec4<T>
	{
		return lhs /= rhs;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto operator*(Vec4<T> lhs, T scalar) noexcept -> Vec4<T>
	{
		return lhs *= scalar;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto operator*(T scalar, Vec4<T> rhs) noexcept -> Vec4<T>
	{
		return rhs *= scalar;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto operator/(Vec4<T> lhs, T scalar) noexcept -> Vec4<T>
	{
		return lhs /= scalar;
	}

	// ============================================================================
	// Vector Operations (Free Functions)
	// ============================================================================

	/**
	 * @brief Compute the dot product of two vectors
	 */
	template<vector_component T>
	[[nodiscard]] constexpr auto Dot(const Vec2<T>& a, const Vec2<T>& b) noexcept -> T
	{
		return a.x * b.x + a.y * b.y;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto Dot(const Vec3<T>& a, const Vec3<T>& b) noexcept -> T
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto Dot(const Vec4<T>& a, const Vec4<T>& b) noexcept -> T
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	}

	/**
	 * @brief Compute the cross product of two 3D vectors
	 */
	template<vector_component T>
	[[nodiscard]] constexpr auto Cross(const Vec3<T>& a, const Vec3<T>& b) noexcept -> Vec3<T>
	{
		return Vec3<T>{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
	}

	/**
	 * @brief Compute the squared length of a vector (avoids sqrt)
	 */
	template<vector_component T>
	[[nodiscard]] constexpr auto LengthSquared(const Vec2<T>& v) noexcept -> T
	{
		return Dot(v, v);
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto LengthSquared(const Vec3<T>& v) noexcept -> T
	{
		return Dot(v, v);
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto LengthSquared(const Vec4<T>& v) noexcept -> T
	{
		return Dot(v, v);
	}

	/**
	 * @brief Compute the length (magnitude) of a vector
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Length(const Vec2<T>& v) noexcept -> T
	{
		return std::sqrt(LengthSquared(v));
	}

	template<std::floating_point T>
	[[nodiscard]] constexpr auto Length(const Vec3<T>& v) noexcept -> T
	{
		return std::sqrt(LengthSquared(v));
	}

	template<std::floating_point T>
	[[nodiscard]] constexpr auto Length(const Vec4<T>& v) noexcept -> T
	{
		return std::sqrt(LengthSquared(v));
	}

	/**
	 * @brief Normalize a vector to unit length
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Normalize(const Vec2<T>& v) noexcept -> Vec2<T>
	{
		return v / Length(v);
	}

	template<std::floating_point T>
	[[nodiscard]] constexpr auto Normalize(const Vec3<T>& v) noexcept -> Vec3<T>
	{
		return v / Length(v);
	}

	template<std::floating_point T>
	[[nodiscard]] constexpr auto Normalize(const Vec4<T>& v) noexcept -> Vec4<T>
	{
		return v / Length(v);
	}

	/**
	 * @brief Compute the distance between two points
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Distance(const Vec2<T>& a, const Vec2<T>& b) noexcept -> T
	{
		return Length(b - a);
	}

	template<std::floating_point T>
	[[nodiscard]] constexpr auto Distance(const Vec3<T>& a, const Vec3<T>& b) noexcept -> T
	{
		return Length(b - a);
	}

	template<std::floating_point T>
	[[nodiscard]] constexpr auto Distance(const Vec4<T>& a, const Vec4<T>& b) noexcept -> T
	{
		return Length(b - a);
	}

	/**
	 * @brief Reflect a vector about a normal
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Reflect(const Vec2<T>& incident, const Vec2<T>& normal) noexcept -> Vec2<T>
	{
		return incident - T{2} * Dot(incident, normal) * normal;
	}

	template<std::floating_point T>
	[[nodiscard]] constexpr auto Reflect(const Vec3<T>& incident, const Vec3<T>& normal) noexcept -> Vec3<T>
	{
		return incident - T{2} * Dot(incident, normal) * normal;
	}

	/**
	 * @brief Linearly interpolate between two vectors
	 */
	template<vector_component T, std::floating_point U>
	[[nodiscard]] constexpr auto Lerp(const Vec2<T>& a, const Vec2<T>& b, U t) noexcept -> Vec2<T>
	{
		return Vec2<T>{static_cast<T>(a.x + t * (b.x - a.x)), static_cast<T>(a.y + t * (b.y - a.y))};
	}

	template<vector_component T, std::floating_point U>
	[[nodiscard]] constexpr auto Lerp(const Vec3<T>& a, const Vec3<T>& b, U t) noexcept -> Vec3<T>
	{
		return Vec3<T>{
			static_cast<T>(a.x + t * (b.x - a.x)),
			static_cast<T>(a.y + t * (b.y - a.y)),
			static_cast<T>(a.z + t * (b.z - a.z))};
	}

	template<vector_component T, std::floating_point U>
	[[nodiscard]] constexpr auto Lerp(const Vec4<T>& a, const Vec4<T>& b, U t) noexcept -> Vec4<T>
	{
		return Vec4<T>{
			static_cast<T>(a.x + t * (b.x - a.x)),
			static_cast<T>(a.y + t * (b.y - a.y)),
			static_cast<T>(a.z + t * (b.z - a.z)),
			static_cast<T>(a.w + t * (b.w - a.w))};
	}

	/**
	 * @brief Component-wise minimum
	 */
	template<vector_component T>
	[[nodiscard]] constexpr auto Min(const Vec2<T>& a, const Vec2<T>& b) noexcept -> Vec2<T>
	{
		return Vec2<T>{std::min(a.x, b.x), std::min(a.y, b.y)};
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto Min(const Vec3<T>& a, const Vec3<T>& b) noexcept -> Vec3<T>
	{
		return Vec3<T>{std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto Min(const Vec4<T>& a, const Vec4<T>& b) noexcept -> Vec4<T>
	{
		return Vec4<T>{std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w)};
	}

	/**
	 * @brief Component-wise maximum
	 */
	template<vector_component T>
	[[nodiscard]] constexpr auto Max(const Vec2<T>& a, const Vec2<T>& b) noexcept -> Vec2<T>
	{
		return Vec2<T>{std::max(a.x, b.x), std::max(a.y, b.y)};
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto Max(const Vec3<T>& a, const Vec3<T>& b) noexcept -> Vec3<T>
	{
		return Vec3<T>{std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto Max(const Vec4<T>& a, const Vec4<T>& b) noexcept -> Vec4<T>
	{
		return Vec4<T>{std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w)};
	}

	/**
	 * @brief Clamp each component to [min, max]
	 */
	template<vector_component T>
	[[nodiscard]] constexpr auto Clamp(const Vec2<T>& v, const Vec2<T>& lo, const Vec2<T>& hi) noexcept -> Vec2<T>
	{
		return Min(Max(v, lo), hi);
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto Clamp(const Vec3<T>& v, const Vec3<T>& lo, const Vec3<T>& hi) noexcept -> Vec3<T>
	{
		return Min(Max(v, lo), hi);
	}

	template<vector_component T>
	[[nodiscard]] constexpr auto Clamp(const Vec4<T>& v, const Vec4<T>& lo, const Vec4<T>& hi) noexcept -> Vec4<T>
	{
		return Min(Max(v, lo), hi);
	}
}
