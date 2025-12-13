export module synodic.periapsis:math.matrix;

import std;

export import :math.vector;

export namespace peri::math
{
	// Forward declarations
	template<std::floating_point T>
	struct Mat3;
	template<std::floating_point T>
	struct Mat4;
	template<std::floating_point T>
	struct Quaternion;

	// ============================================================================
	// Mat3 - 3x3 Matrix (Column-Major)
	// ============================================================================

	/**
	 * @brief A 3x3 matrix with column-major storage
	 * @tparam T Floating-point component type
	 * @details Column-major layout matches OpenGL/Vulkan conventions.
	 *          Columns are stored contiguously: [col0, col1, col2]
	 */
	template<std::floating_point T>
	struct Mat3
	{
		using value_type					   = T;
		using column_type					   = Vec3<T>;
		static constexpr std::size_t COLUMNS_V = 3;
		static constexpr std::size_t ROWS_V	   = 3;

		// Column-major storage: columns[0] is first column
		column_type columns[3];

		// Default constructor - identity matrix
		constexpr Mat3() noexcept :
			columns {
				column_type {T {1}, T {0}, T {0}},
				column_type {T {0}, T {1}, T {0}},
				column_type {T {0}, T {0}, T {1}}
		 }
		{
		}

		// Scalar constructor - diagonal matrix
		constexpr explicit Mat3(T diagonal) noexcept :
			columns {
				column_type {diagonal, T {0},	  T {0}   },
				column_type {T {0},	diagonal, T {0}   },
				column_type {T {0},	T {0},	   diagonal}
		}
		{
		}

		// Column constructor
		constexpr Mat3(const column_type& col0, const column_type& col1, const column_type& col2) noexcept :
			columns {col0, col1, col2}
		{
		}

		// Element constructor
		constexpr Mat3(T m00, T m10, T m20, T m01, T m11, T m21, T m02, T m12, T m22) noexcept :
			columns {
				column_type {m00, m10, m20},
				column_type {m01, m11, m21},
				column_type {m02, m12, m22}
		   }
		{
		}

		// Conversion from different precision
		template<std::floating_point U>
			requires(!std::same_as<T, U>)
		constexpr explicit Mat3(const Mat3<U>& other) noexcept :
			columns {column_type {other.columns[0]}, column_type {other.columns[1]}, column_type {other.columns[2]}}
		{
		}

		// Column access
		[[nodiscard]] constexpr auto operator[](std::size_t col) noexcept -> column_type&
		{
			return columns[col];
		}

		[[nodiscard]] constexpr auto operator[](std::size_t col) const noexcept -> const column_type&
		{
			return columns[col];
		}

		// Element access (column, row)
		[[nodiscard]] constexpr auto At(std::size_t col, std::size_t row) noexcept -> T&
		{
			return columns[col][row];
		}

		[[nodiscard]] constexpr auto At(std::size_t col, std::size_t row) const noexcept -> const T&
		{
			return columns[col][row];
		}

		// Data access for interop (column-major flat array)
		[[nodiscard]] constexpr auto Data() noexcept -> T*
		{
			return columns[0].Data();
		}

		[[nodiscard]] constexpr auto Data() const noexcept -> const T*
		{
			return columns[0].Data();
		}

		// Row extraction
		[[nodiscard]] constexpr auto Row(std::size_t r) const noexcept -> Vec3<T>
		{
			return Vec3<T> {columns[0][r], columns[1][r], columns[2][r]};
		}

		// Identity matrix
		[[nodiscard]] static constexpr auto Identity() noexcept -> Mat3
		{
			return Mat3 {};
		}

		// Zero matrix
		[[nodiscard]] static constexpr auto Zero() noexcept -> Mat3
		{
			return Mat3 {T {0}};
		}

		// Compound arithmetic
		constexpr auto operator+=(const Mat3& rhs) noexcept -> Mat3&
		{
			columns[0] += rhs.columns[0];
			columns[1] += rhs.columns[1];
			columns[2] += rhs.columns[2];
			return *this;
		}

		constexpr auto operator-=(const Mat3& rhs) noexcept -> Mat3&
		{
			columns[0] -= rhs.columns[0];
			columns[1] -= rhs.columns[1];
			columns[2] -= rhs.columns[2];
			return *this;
		}

		constexpr auto operator*=(T scalar) noexcept -> Mat3&
		{
			columns[0] *= scalar;
			columns[1] *= scalar;
			columns[2] *= scalar;
			return *this;
		}

		constexpr auto operator/=(T scalar) noexcept -> Mat3&
		{
			columns[0] /= scalar;
			columns[1] /= scalar;
			columns[2] /= scalar;
			return *this;
		}

		// Unary operators
		[[nodiscard]] constexpr auto operator-() const noexcept -> Mat3
		{
			return Mat3 {-columns[0], -columns[1], -columns[2]};
		}

		[[nodiscard]] constexpr auto operator+() const noexcept -> Mat3
		{
			return *this;
		}

		// Comparison
		[[nodiscard]] constexpr auto operator==(const Mat3&) const noexcept -> bool = default;
	};

	// Binary operators for Mat3
	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator+(Mat3<T> lhs, const Mat3<T>& rhs) noexcept -> Mat3<T>
	{
		return lhs += rhs;
	}

	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator-(Mat3<T> lhs, const Mat3<T>& rhs) noexcept -> Mat3<T>
	{
		return lhs -= rhs;
	}

	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator*(Mat3<T> lhs, T scalar) noexcept -> Mat3<T>
	{
		return lhs *= scalar;
	}

	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator*(T scalar, Mat3<T> rhs) noexcept -> Mat3<T>
	{
		return rhs *= scalar;
	}

	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator/(Mat3<T> lhs, T scalar) noexcept -> Mat3<T>
	{
		return lhs /= scalar;
	}

	// Matrix-vector multiplication
	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator*(const Mat3<T>& m, const Vec3<T>& v) noexcept -> Vec3<T>
	{
		return m.columns[0] * v.x + m.columns[1] * v.y + m.columns[2] * v.z;
	}

	// Matrix-matrix multiplication
	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator*(const Mat3<T>& a, const Mat3<T>& b) noexcept -> Mat3<T>
	{
		return Mat3<T> {a * b.columns[0], a * b.columns[1], a * b.columns[2]};
	}

	// ============================================================================
	// Mat4 - 4x4 Matrix
	// ============================================================================

	/**
	 * @brief A 4x4 matrix with column-major storage
	 * @tparam T Floating-point component type
	 * @details Column-major layout matches OpenGL/Vulkan conventions.
	 *          Ideal for transforms: translation in column 3.
	 */
	template<std::floating_point T>
	struct alignas(sizeof(T) * 4) Mat4
	{
		using value_type					   = T;
		using column_type					   = Vec4<T>;
		static constexpr std::size_t COLUMNS_V = 4;
		static constexpr std::size_t ROWS_V	   = 4;

		// Column-major storage
		column_type columns[4];

		// Default constructor - identity matrix
		constexpr Mat4() noexcept :
			columns {
				column_type {T {1}, T {0}, T {0}, T {0}},
				column_type {T {0}, T {1}, T {0}, T {0}},
				column_type {T {0}, T {0}, T {1}, T {0}},
				column_type {T {0}, T {0}, T {0}, T {1}}
		}
		{
		}

		// Scalar constructor - diagonal matrix
		constexpr explicit Mat4(T diagonal) noexcept :
			columns {
				column_type {diagonal, T {0},	  T {0},	 T {0}	  },
				column_type {T {0},	diagonal, T {0},	 T {0}	  },
				column_type {T {0},	T {0},	   diagonal, T {0}	  },
				column_type {T {0},	T {0},	   T {0},	  diagonal}
		   }
		{
		}

		// Column constructor
		constexpr Mat4(
			const column_type& col0,
			const column_type& col1,
			const column_type& col2,
			const column_type& col3) noexcept :
			columns {col0, col1, col2, col3}
		{
		}

		// Construct from Mat3 (upper-left 3x3, rest is identity)
		constexpr explicit Mat4(const Mat3<T>& m) noexcept :
			columns {
				column_type {m.columns[0], T {0}},
				column_type {m.columns[1], T {0}},
				column_type {m.columns[2], T {0}},
				column_type {T {0}, T {0}, T {0}, T {1}}
		}
		{
		}

		// Element constructor
		constexpr Mat4(
			T m00,
			T m10,
			T m20,
			T m30,
			T m01,
			T m11,
			T m21,
			T m31,
			T m02,
			T m12,
			T m22,
			T m32,
			T m03,
			T m13,
			T m23,
			T m33) noexcept :
			columns {
				column_type {m00, m10, m20, m30},
				column_type {m01, m11, m21, m31},
				column_type {m02, m12, m22, m32},
				column_type {m03, m13, m23, m33}
		}
		{
		}

		// Conversion from different precision
		template<std::floating_point U>
			requires(!std::same_as<T, U>)
		constexpr explicit Mat4(const Mat4<U>& other) noexcept :
			columns {
				column_type {other.columns[0]},
				column_type {other.columns[1]},
				column_type {other.columns[2]},
				column_type {other.columns[3]}}
		{
		}

		// Column access
		[[nodiscard]] constexpr auto operator[](std::size_t col) noexcept -> column_type&
		{
			return columns[col];
		}

		[[nodiscard]] constexpr auto operator[](std::size_t col) const noexcept -> const column_type&
		{
			return columns[col];
		}

		// Element access (column, row)
		[[nodiscard]] constexpr auto At(std::size_t col, std::size_t row) noexcept -> T&
		{
			return columns[col][row];
		}

		[[nodiscard]] constexpr auto At(std::size_t col, std::size_t row) const noexcept -> const T&
		{
			return columns[col][row];
		}

		// Data access for interop
		[[nodiscard]] constexpr auto Data() noexcept -> T*
		{
			return columns[0].Data();
		}

		[[nodiscard]] constexpr auto Data() const noexcept -> const T*
		{
			return columns[0].Data();
		}

		// Row extraction
		[[nodiscard]] constexpr auto Row(std::size_t r) const noexcept -> Vec4<T>
		{
			return Vec4<T> {columns[0][r], columns[1][r], columns[2][r], columns[3][r]};
		}

		// Extract upper-left 3x3
		[[nodiscard]] constexpr auto Upper3x3() const noexcept -> Mat3<T>
		{
			return Mat3<T> {columns[0].xyz(), columns[1].xyz(), columns[2].xyz()};
		}

		// Identity matrix
		[[nodiscard]] static constexpr auto Identity() noexcept -> Mat4
		{
			return Mat4 {};
		}

		// Zero matrix
		[[nodiscard]] static constexpr auto Zero() noexcept -> Mat4
		{
			return Mat4 {T {0}};
		}

		// Compound arithmetic
		constexpr auto operator+=(const Mat4& rhs) noexcept -> Mat4&
		{
			columns[0] += rhs.columns[0];
			columns[1] += rhs.columns[1];
			columns[2] += rhs.columns[2];
			columns[3] += rhs.columns[3];
			return *this;
		}

		constexpr auto operator-=(const Mat4& rhs) noexcept -> Mat4&
		{
			columns[0] -= rhs.columns[0];
			columns[1] -= rhs.columns[1];
			columns[2] -= rhs.columns[2];
			columns[3] -= rhs.columns[3];
			return *this;
		}

		constexpr auto operator*=(T scalar) noexcept -> Mat4&
		{
			columns[0] *= scalar;
			columns[1] *= scalar;
			columns[2] *= scalar;
			columns[3] *= scalar;
			return *this;
		}

		constexpr auto operator/=(T scalar) noexcept -> Mat4&
		{
			columns[0] /= scalar;
			columns[1] /= scalar;
			columns[2] /= scalar;
			columns[3] /= scalar;
			return *this;
		}

		// Unary operators
		[[nodiscard]] constexpr auto operator-() const noexcept -> Mat4
		{
			return Mat4 {-columns[0], -columns[1], -columns[2], -columns[3]};
		}

		[[nodiscard]] constexpr auto operator+() const noexcept -> Mat4
		{
			return *this;
		}

		// Comparison
		[[nodiscard]] constexpr auto operator==(const Mat4&) const noexcept -> bool = default;
	};

	// Binary operators for Mat4
	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator+(Mat4<T> lhs, const Mat4<T>& rhs) noexcept -> Mat4<T>
	{
		return lhs += rhs;
	}

	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator-(Mat4<T> lhs, const Mat4<T>& rhs) noexcept -> Mat4<T>
	{
		return lhs -= rhs;
	}

	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator*(Mat4<T> lhs, T scalar) noexcept -> Mat4<T>
	{
		return lhs *= scalar;
	}

	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator*(T scalar, Mat4<T> rhs) noexcept -> Mat4<T>
	{
		return rhs *= scalar;
	}

	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator/(Mat4<T> lhs, T scalar) noexcept -> Mat4<T>
	{
		return lhs /= scalar;
	}

	// Matrix-vector multiplication
	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator*(const Mat4<T>& m, const Vec4<T>& v) noexcept -> Vec4<T>
	{
		return m.columns[0] * v.x + m.columns[1] * v.y + m.columns[2] * v.z + m.columns[3] * v.w;
	}

	// Matrix-matrix multiplication
	template<std::floating_point T>
	[[nodiscard]] constexpr auto operator*(const Mat4<T>& a, const Mat4<T>& b) noexcept -> Mat4<T>
	{
		return Mat4<T> {a * b.columns[0], a * b.columns[1], a * b.columns[2], a * b.columns[3]};
	}

	// Compound matrix multiplication
	template<std::floating_point T>
	constexpr auto operator*=(Mat4<T>& a, const Mat4<T>& b) noexcept -> Mat4<T>&
	{
		a = a * b;
		return a;
	}

	template<std::floating_point T>
	constexpr auto operator*=(Mat3<T>& a, const Mat3<T>& b) noexcept -> Mat3<T>&
	{
		a = a * b;
		return a;
	}

	// ============================================================================
	// Matrix Operations
	// ============================================================================

	/**
	 * @brief Transpose a 3x3 matrix
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Transpose(const Mat3<T>& m) noexcept -> Mat3<T>
	{
		return Mat3<T> {m.Row(0), m.Row(1), m.Row(2)};
	}

	/**
	 * @brief Transpose a 4x4 matrix
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Transpose(const Mat4<T>& m) noexcept -> Mat4<T>
	{
		return Mat4<T> {m.Row(0), m.Row(1), m.Row(2), m.Row(3)};
	}

	/**
	 * @brief Determinant of a 3x3 matrix
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Determinant(const Mat3<T>& m) noexcept -> T
	{
		return m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2]) - m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2]) +
			   m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]);
	}

	/**
	 * @brief Determinant of a 4x4 matrix
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Determinant(const Mat4<T>& m) noexcept -> T
	{
		const T a = m[2][2] * m[3][3] - m[3][2] * m[2][3];
		const T b = m[1][2] * m[3][3] - m[3][2] * m[1][3];
		const T c = m[1][2] * m[2][3] - m[2][2] * m[1][3];
		const T d = m[0][2] * m[3][3] - m[3][2] * m[0][3];
		const T e = m[0][2] * m[2][3] - m[2][2] * m[0][3];
		const T f = m[0][2] * m[1][3] - m[1][2] * m[0][3];

		return m[0][0] * (m[1][1] * a - m[2][1] * b + m[3][1] * c) -
			   m[1][0] * (m[0][1] * a - m[2][1] * d + m[3][1] * e) +
			   m[2][0] * (m[0][1] * b - m[1][1] * d + m[3][1] * f) -
			   m[3][0] * (m[0][1] * c - m[1][1] * e + m[2][1] * f);
	}

	/**
	 * @brief Inverse of a 3x3 matrix
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Inverse(const Mat3<T>& m) noexcept -> Mat3<T>
	{
		const T det	   = Determinant(m);
		const T invDet = T {1} / det;

		Mat3<T> result;
		result[0][0] = (m[1][1] * m[2][2] - m[2][1] * m[1][2]) * invDet;
		result[0][1] = (m[2][1] * m[0][2] - m[0][1] * m[2][2]) * invDet;
		result[0][2] = (m[0][1] * m[1][2] - m[1][1] * m[0][2]) * invDet;
		result[1][0] = (m[2][0] * m[1][2] - m[1][0] * m[2][2]) * invDet;
		result[1][1] = (m[0][0] * m[2][2] - m[2][0] * m[0][2]) * invDet;
		result[1][2] = (m[1][0] * m[0][2] - m[0][0] * m[1][2]) * invDet;
		result[2][0] = (m[1][0] * m[2][1] - m[2][0] * m[1][1]) * invDet;
		result[2][1] = (m[2][0] * m[0][1] - m[0][0] * m[2][1]) * invDet;
		result[2][2] = (m[0][0] * m[1][1] - m[1][0] * m[0][1]) * invDet;
		return result;
	}

	/**
	 * @brief Inverse of a 4x4 matrix
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Inverse(const Mat4<T>& m) noexcept -> Mat4<T>
	{
		const T c00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
		const T c02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
		const T c03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];
		const T c04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
		const T c06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
		const T c07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];
		const T c08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
		const T c10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
		const T c11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];
		const T c12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
		const T c14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
		const T c15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];
		const T c16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
		const T c18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
		const T c19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];
		const T c20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
		const T c22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
		const T c23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

		Vec4<T> fac0 {c00, c00, c02, c03};
		Vec4<T> fac1 {c04, c04, c06, c07};
		Vec4<T> fac2 {c08, c08, c10, c11};
		Vec4<T> fac3 {c12, c12, c14, c15};
		Vec4<T> fac4 {c16, c16, c18, c19};
		Vec4<T> fac5 {c20, c20, c22, c23};

		Vec4<T> vec0 {m[1][0], m[0][0], m[0][0], m[0][0]};
		Vec4<T> vec1 {m[1][1], m[0][1], m[0][1], m[0][1]};
		Vec4<T> vec2 {m[1][2], m[0][2], m[0][2], m[0][2]};
		Vec4<T> vec3 {m[1][3], m[0][3], m[0][3], m[0][3]};

		Vec4<T> inv0 {vec1 * fac0 - vec2 * fac1 + vec3 * fac2};
		Vec4<T> inv1 {vec0 * fac0 - vec2 * fac3 + vec3 * fac4};
		Vec4<T> inv2 {vec0 * fac1 - vec1 * fac3 + vec3 * fac5};
		Vec4<T> inv3 {vec0 * fac2 - vec1 * fac4 + vec2 * fac5};

		Vec4<T> signA {+T {1}, -T {1}, +T {1}, -T {1}};
		Vec4<T> signB {-T {1}, +T {1}, -T {1}, +T {1}};

		Mat4<T> inv {inv0 * signA, inv1 * signB, inv2 * signA, inv3 * signB};

		Vec4<T> row0 {inv[0][0], inv[1][0], inv[2][0], inv[3][0]};
		Vec4<T> col0 {m[0][0], m[0][1], m[0][2], m[0][3]};
		T det	 = Dot(col0, row0);
		T invDet = T {1} / det;

		return inv * invDet;
	}

	// ============================================================================
	// Transform Construction Functions (ADL friendly)
	// ============================================================================

	/**
	 * @brief Create a translation matrix
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Translate(const Vec3<T>& v) noexcept -> Mat4<T>
	{
		Mat4<T> result {};
		result[3] = Vec4<T> {v, T {1}};
		return result;
	}

	/**
	 * @brief Create a scale matrix
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Scale(const Vec3<T>& v) noexcept -> Mat4<T>
	{
		Mat4<T> result {};
		result[0][0] = v.x;
		result[1][1] = v.y;
		result[2][2] = v.z;
		return result;
	}

	/**
	 * @brief Create a rotation matrix around an axis
	 * @param angle Angle in radians
	 * @param axis Normalized axis of rotation
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Rotate(T angle, const Vec3<T>& axis) noexcept -> Mat4<T>
	{
		const T c = std::cos(angle);
		const T s = std::sin(angle);
		const T t = T {1} - c;

		const Vec3<T> n = Normalize(axis);
		const T x		= n.x;
		const T y		= n.y;
		const T z		= n.z;

		return Mat4<T> {
			Vec4<T> {t * x * x + c,		t * x * y + s * z, t * x * z - s * y, T {0}},
			Vec4<T> {t * x * y - s * z, t * y * y + c,	   t * y * z + s * x, T {0}},
			Vec4<T> {t * x * z + s * y, t * y * z - s * x, t * z * z + c,	  T {0}},
			Vec4<T> {T {0},				T {0},			   T {0},			  T {1}}
		};
	}

	/**
	 * @brief Create a look-at view matrix (right-handed)
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto LookAt(const Vec3<T>& eye, const Vec3<T>& center, const Vec3<T>& up) noexcept
		-> Mat4<T>
	{
		const Vec3<T> f = Normalize(center - eye);
		const Vec3<T> s = Normalize(Cross(f, up));
		const Vec3<T> u = Cross(s, f);

		Mat4<T> result {};
		result[0][0] = s.x;
		result[1][0] = s.y;
		result[2][0] = s.z;
		result[0][1] = u.x;
		result[1][1] = u.y;
		result[2][1] = u.z;
		result[0][2] = -f.x;
		result[1][2] = -f.y;
		result[2][2] = -f.z;
		result[3][0] = -Dot(s, eye);
		result[3][1] = -Dot(u, eye);
		result[3][2] = Dot(f, eye);
		return result;
	}

	/**
	 * @brief Create a perspective projection matrix (right-handed, zero-to-one depth)
	 * @param fovY Vertical field of view in radians
	 * @param aspect Aspect ratio (width / height)
	 * @param nearPlane Near clipping plane distance
	 * @param farPlane Far clipping plane distance
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Perspective(T fovY, T aspect, T nearPlane, T farPlane) noexcept -> Mat4<T>
	{
		const T tanHalfFov = std::tan(fovY / T {2});

		Mat4<T> result {T {0}};
		result[0][0] = T {1} / (aspect * tanHalfFov);
		result[1][1] = T {1} / tanHalfFov;
		result[2][2] = farPlane / (nearPlane - farPlane);
		result[2][3] = -T {1};
		result[3][2] = -(farPlane * nearPlane) / (farPlane - nearPlane);
		return result;
	}

	/**
	 * @brief Create an orthographic projection matrix (right-handed, zero-to-one depth)
	 */
	template<std::floating_point T>
	[[nodiscard]] constexpr auto Ortho(T left, T right, T bottom, T top, T nearPlane, T farPlane) noexcept -> Mat4<T>
	{
		Mat4<T> result {};
		result[0][0] = T {2} / (right - left);
		result[1][1] = T {2} / (top - bottom);
		result[2][2] = -T {1} / (farPlane - nearPlane);
		result[3][0] = -(right + left) / (right - left);
		result[3][1] = -(top + bottom) / (top - bottom);
		result[3][2] = -nearPlane / (farPlane - nearPlane);
		return result;
	}
}
