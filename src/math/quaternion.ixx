export module synodic.periapsis:math.quaternion;

import std;

export import :math.vector;
export import :math.matrix;

export namespace peri::math
{
	// ============================================================================
	// Quaternion
	// ============================================================================

	/**
	 * @brief A quaternion for representing 3D rotations
	 * @tparam T Floating-point component type
	 * @details Storage order: (x, y, z, w) where w is the scalar part.
	 *          Represents rotation as: q = w + xi + yj + zk
	 */
	template<std::floating_point T>
	struct alignas(sizeof(T) * 4) Quaternion
	{
		using value_type = T;

		T x {};
		T y {};
		T z {};
		T w {T {1}};  // Identity quaternion by default

		// Default constructor - identity rotation
		constexpr Quaternion() noexcept = default;

		// Component constructor
		constexpr Quaternion(T x, T y, T z, T w) noexcept :
			x {x},
			y {y},
			z {z},
			w {w}
		{
		}

		// Construct from scalar and vector part
		constexpr Quaternion(T scalar, const Vec3<T>& vector) noexcept :
			x {vector.x},
			y {vector.y},
			z {vector.z},
			w {scalar}
		{
		}

		// Construct from Vec4 (x, y, z, w)
		constexpr explicit Quaternion(const Vec4<T>& v) noexcept :
			x {v.x},
			y {v.y},
			z {v.z},
			w {v.w}
		{
		}

		// Conversion from different precision
		template<std::floating_point U>
			requires(!std::same_as<T, U>)
		constexpr explicit Quaternion(const Quaternion<U>& other) noexcept :
			x {static_cast<T>(other.x)},
			y {static_cast<T>(other.y)},
			z {static_cast<T>(other.z)},
			w {static_cast<T>(other.w)}
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

		// Vector part access
		[[nodiscard]] constexpr auto Vector() const noexcept -> Vec3<T>
		{
			return Vec3<T> {x, y, z};
		}

		// Scalar part access
		[[nodiscard]] constexpr auto Scalar() const noexcept -> T
		{
			return w;
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

		// As Vec4
		[[nodiscard]] constexpr auto AsVec4() const noexcept -> Vec4<T>
		{
			return Vec4<T> {x, y, z, w};
		}

		// Identity quaternion
		[[nodiscard]] static constexpr auto Identity() noexcept -> Quaternion
		{
			return Quaternion {};
		}

		// Compound arithmetic
		constexpr auto operator+=(const Quaternion& rhs) noexcept -> Quaternion&
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			w += rhs.w;
			return *this;
		}

		constexpr auto operator-=(const Quaternion& rhs) noexcept -> Quaternion&
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			w -= rhs.w;
			return *this;
		}

		constexpr auto operator*=(T scalar) noexcept -> Quaternion&
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			w *= scalar;
			return *this;
		}

		constexpr auto operator/=(T scalar) noexcept -> Quaternion&
		{
			x /= scalar;
			y /= scalar;
			z /= scalar;
			w /= scalar;
			return *this;
		}

		// Unary operators
		[[nodiscard]] constexpr auto operator-() const noexcept -> Quaternion
		{
			return Quaternion {-x, -y, -z, -w};
		}

		[[nodiscard]] constexpr auto operator+() const noexcept -> Quaternion
		{
			return *this;
		}

		// Comparison
		[[nodiscard]] constexpr auto operator==(const Quaternion&) const noexcept -> bool = default;
	};

	// Binary operators
	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator+(Quaternion<T> lhs, const Quaternion<T>& rhs) noexcept -> Quaternion<T>
	{
		return lhs += rhs;
	}

	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator-(Quaternion<T> lhs, const Quaternion<T>& rhs) noexcept -> Quaternion<T>
	{
		return lhs -= rhs;
	}

	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator*(Quaternion<T> lhs, T scalar) noexcept -> Quaternion<T>
	{
		return lhs *= scalar;
	}

	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator*(T scalar, Quaternion<T> rhs) noexcept -> Quaternion<T>
	{
		return rhs *= scalar;
	}

	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator/(Quaternion<T> lhs, T scalar) noexcept -> Quaternion<T>
	{
		return lhs /= scalar;
	}

	// Quaternion multiplication. Hamilton product
	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator*(const Quaternion<T>& p, const Quaternion<T>& q) noexcept -> Quaternion<T>
	{
		return Quaternion<T> {
			p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y,
			p.w * q.y - p.x * q.z + p.y * q.w + p.z * q.x,
			p.w * q.z + p.x * q.y - p.y * q.x + p.z * q.w,
			p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z};
	}

	template<std::floating_point T>
	constexpr auto operator*=(Quaternion<T>& lhs, const Quaternion<T>& rhs) noexcept -> Quaternion<T>&
	{
		lhs = lhs * rhs;
		return lhs;
	}

	// Rotate a vector by a quaternion
	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator*(const Quaternion<T>& q, const Vec3<T>& v) noexcept -> Vec3<T>
	{
		const Vec3<T> qv  = q.Vector();
		const Vec3<T> uv  = Cross(qv, v);
		const Vec3<T> uuv = Cross(qv, uv);
		return v + (uv * q.w + uuv) * T {2};
	}

	// ============================================================================
	// Quaternion Operations
	// ============================================================================

	/**
	 * @brief Dot product of two quaternions
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Dot(const Quaternion<T>& a, const Quaternion<T>& b) noexcept -> T
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	}

	/**
	 * @brief Squared length of a quaternion
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto LengthSquared(const Quaternion<T>& q) noexcept -> T
	{
		return Dot(q, q);
	}

	/**
	 * @brief Length (norm) of a quaternion
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Length(const Quaternion<T>& q) noexcept -> T
	{
		return std::sqrt(LengthSquared(q));
	}

	/**
	 * @brief Normalize a quaternion to unit length
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Normalize(const Quaternion<T>& q) noexcept -> Quaternion<T>
	{
		return q / Length(q);
	}

	/**
	 * @brief Conjugate of a quaternion (negates vector part)
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Conjugate(const Quaternion<T>& q) noexcept -> Quaternion<T>
	{
		return Quaternion<T> {-q.x, -q.y, -q.z, q.w};
	}

	/**
	 * @brief Inverse of a quaternion
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Inverse(const Quaternion<T>& q) noexcept -> Quaternion<T>
	{
		return Conjugate(q) / LengthSquared(q);
	}

	/**
	 * @brief Linear interpolation between quaternions. Not normalized
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Lerp(const Quaternion<T>& a, const Quaternion<T>& b, T t) noexcept -> Quaternion<T>
	{
		return Quaternion<T> {
			a.x + t * (b.x - a.x),
			a.y + t * (b.y - a.y),
			a.z + t * (b.z - a.z),
			a.w + t * (b.w - a.w)};
	}

	/**
	 * @brief Normalized linear interpolation between quaternions
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Nlerp(const Quaternion<T>& a, const Quaternion<T>& b, T t) noexcept -> Quaternion<T>
	{
		return Normalize(Lerp(a, b, t));
	}

	/**
	 * @brief Spherical linear interpolation between quaternions
	 * @details Takes the shortest path on the 4D unit sphere
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Slerp(const Quaternion<T>& a, Quaternion<T> b, T t) noexcept -> Quaternion<T>
	{
		T cosTheta = Dot(a, b);

		// If cos_theta < 0, negate one quaternion to take shorter path
		if (cosTheta < T {0})
		{
			b		 = -b;
			cosTheta = -cosTheta;
		}

		// If quaternions are very close, use linear interpolation
		if (cosTheta > T {0.9995})
		{
			return Nlerp(a, b, t);
		}

		const T theta	 = std::acos(cosTheta);
		const T sinTheta = std::sin(theta);
		const T wa		 = std::sin((T {1} - t) * theta) / sinTheta;
		const T wb		 = std::sin(t * theta) / sinTheta;

		return Quaternion<T> {wa * a.x + wb * b.x, wa * a.y + wb * b.y, wa * a.z + wb * b.z, wa * a.w + wb * b.w};
	}

	// ============================================================================
	// Quaternion Construction Functions
	// ============================================================================

	/**
	 * @brief Create a quaternion from an axis-angle rotation
	 * @param angle Angle in radians
	 * @param axis Normalized axis of rotation
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto QuatFromAxisAngle(T angle, const Vec3<T>& axis) noexcept -> Quaternion<T>
	{
		const T halfAngle = angle / T {2};
		const T s		  = std::sin(halfAngle);
		const T c		  = std::cos(halfAngle);
		const Vec3<T> n	  = Normalize(axis);
		return Quaternion<T> {n.x * s, n.y * s, n.z * s, c};
	}

	/**
	 * @brief Create a quaternion from Euler angles
	 * @param pitch Rotation around X axis in radians
	 * @param yaw Rotation around Y axis in radians
	 * @param roll Rotation around Z axis in radians
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto QuatFromEuler(T pitch, T yaw, T roll) noexcept -> Quaternion<T>
	{
		const T cp = std::cos(pitch * T {0.5});
		const T sp = std::sin(pitch * T {0.5});
		const T cy = std::cos(yaw * T {0.5});
		const T sy = std::sin(yaw * T {0.5});
		const T cr = std::cos(roll * T {0.5});
		const T sr = std::sin(roll * T {0.5});

		return Quaternion<T> {
			sr * cp * cy - cr * sp * sy,
			cr * sp * cy + sr * cp * sy,
			cr * cp * sy - sr * sp * cy,
			cr * cp * cy + sr * sp * sy};
	}

	/**
	 * @brief Create a quaternion from a 3x3 rotation matrix
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto QuatFromMatrix(const Mat3<T>& m) noexcept -> Quaternion<T>
	{
		const T trace = m[0][0] + m[1][1] + m[2][2];

		if (trace > T {0})
		{
			const T s = T {0.5} / std::sqrt(trace + T {1});
			return Quaternion<T> {
				(m[1][2] - m[2][1]) * s,
				(m[2][0] - m[0][2]) * s,
				(m[0][1] - m[1][0]) * s,
				T {0.25} / s};
		}
		else if (m[0][0] > m[1][1] && m[0][0] > m[2][2])
		{
			const T s = T {2} * std::sqrt(T {1} + m[0][0] - m[1][1] - m[2][2]);
			return Quaternion<T> {
				T {0.25} * s,
				(m[1][0] + m[0][1]) / s,
				(m[2][0] + m[0][2]) / s,
				(m[1][2] - m[2][1]) / s};
		}
		else if (m[1][1] > m[2][2])
		{
			const T s = T {2} * std::sqrt(T {1} + m[1][1] - m[0][0] - m[2][2]);
			return Quaternion<T> {
				(m[1][0] + m[0][1]) / s,
				T {0.25} * s,
				(m[2][1] + m[1][2]) / s,
				(m[2][0] - m[0][2]) / s};
		}
		else
		{
			const T s = T {2} * std::sqrt(T {1} + m[2][2] - m[0][0] - m[1][1]);
			return Quaternion<T> {
				(m[2][0] + m[0][2]) / s,
				(m[2][1] + m[1][2]) / s,
				T {0.25} * s,
				(m[0][1] - m[1][0]) / s};
		}
	}

	/**
	 * @brief Create a quaternion from a 4x4 transformation matrix
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto QuatFromMatrix(const Mat4<T>& m) noexcept -> Quaternion<T>
	{
		return QuatFromMatrix(m.Upper3x3());
	}

	/**
	 * @brief Convert a quaternion to a 3x3 rotation matrix
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto QuatToMatrix3(const Quaternion<T>& q) noexcept -> Mat3<T>
	{
		const T xx = q.x * q.x;
		const T yy = q.y * q.y;
		const T zz = q.z * q.z;
		const T xy = q.x * q.y;
		const T xz = q.x * q.z;
		const T yz = q.y * q.z;
		const T wx = q.w * q.x;
		const T wy = q.w * q.y;
		const T wz = q.w * q.z;

		return Mat3<T> {
			Vec3<T> {T {1} - T {2} * (yy + zz), T {2} * (xy + wz),		   T {2} * (xz - wy)		},
			Vec3<T> {T {2} * (xy - wz),			T {1} - T {2} * (xx + zz), T {2} * (yz + wx)		},
			Vec3<T> {T {2} * (xz + wy),			T {2} * (yz - wx),		   T {1} - T {2} * (xx + yy)}
		 };
	}

	/**
	 * @brief Convert a quaternion to a 4x4 rotation matrix
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto QuatToMatrix4(const Quaternion<T>& q) noexcept -> Mat4<T>
	{
		return Mat4<T> {QuatToMatrix3(q)};
	}

	/**
	 * @brief Extract Euler angles from a quaternion in radians
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto QuatToEuler(const Quaternion<T>& q) noexcept -> Vec3<T>
	{
		// Roll
		const T sinr_cosp = T {2} * (q.w * q.x + q.y * q.z);
		const T cosr_cosp = T {1} - T {2} * (q.x * q.x + q.y * q.y);
		const T roll	  = std::atan2(sinr_cosp, cosr_cosp);

		// Pitch
		const T sinp = T {2} * (q.w * q.y - q.z * q.x);
		T pitch;
		if (std::abs(sinp) >= T {1})
		{
			pitch = std::copysign(std::numbers::pi_v<T> / T {2}, sinp);
		}
		else
		{
			pitch = std::asin(sinp);
		}

		// Yaw
		const T siny_cosp = T {2} * (q.w * q.z + q.x * q.y);
		const T cosy_cosp = T {1} - T {2} * (q.y * q.y + q.z * q.z);
		const T yaw		  = std::atan2(siny_cosp, cosy_cosp);

		return Vec3<T> {pitch, yaw, roll};
	}

	/**
	 * @brief Get the axis of rotation from a quaternion
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto QuatAxis(const Quaternion<T>& q) noexcept -> Vec3<T>
	{
		const T s = std::sqrt(T {1} - q.w * q.w);
		if (s < T {0.0001})
		{
			return Vec3<T> {T {1}, T {0}, T {0}};  // Arbitrary axis for identity
		}
		return q.Vector() / s;
	}

	/**
	 * @brief Get the angle of rotation from a quaternion in radians
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto QuatAngle(const Quaternion<T>& q) noexcept -> T
	{
		return T {2} * std::acos(std::clamp(q.w, T {-1}, T {1}));
	}
}
