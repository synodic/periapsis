export module synodic.periapsis:structure.tree.fenwick;

import std;
import :type.concepts.algebraic;
import :type.concepts.core;

export namespace peri::structure
{
	/**
	 * @brief A Fenwick tree, Binary Indexed Tree, for efficient range sum queries.
	 *
	 * @details Supports both owned memory and non-owned memory.
	 *			Provides O(log n) update and query operations.
	 *
	 * @tparam T The element type
	 * @tparam Container The backing storage container type
	 */
	template<typename T, typename Container>
		requires peri::semigroup<T> and peri::decayed<Container> and std::ranges::contiguous_range<Container>
	class Fenwick
	{
	public:
		// ========================================================================
		// Type Definitions
		// ========================================================================

		using value_type	  = T;
		using container_type  = Container;
		using size_type		  = std::size_t;
		using difference_type = std::ptrdiff_t;

		// ========================================================================
		// Construction
		// ========================================================================

		/**
		 * @brief Default constructs a Fenwick tree.
		 *
		 * @details For fixed-size containers (std::array), creates a zero-initialized tree.
		 *          For dynamic containers (std::vector), creates an empty tree.
		 */
		constexpr Fenwick()
			requires std::default_initializable<Container> && (!std::is_same_v<Container, std::span<T>>)
			:
			tree_(),
			size_(std::size(tree_))
		{
		}

		/**
		 * @brief Constructs a Fenwick tree with given capacity.
		 *
		 * @param capacity Number of elements to allocate
		 */
		explicit Fenwick(size_type capacity)
			requires requires { Container(capacity, T()); }
			:
			tree_(capacity, T()),
			size_(capacity)
		{
		}

		/**
		 * @brief Constructs a Fenwick tree from an existing container.
		 *
		 * @param container The container to use as backing storage
		 */
		explicit Fenwick(Container container)
			requires(!std::is_same_v<Container, std::span<T>>)
			:
			tree_(std::move(container)),
			size_(std::size(tree_))
		{
		}

		/**
		 * @brief Constructs a Fenwick tree from a span.
		 *
		 * @param span Reference to a contiguous span of data
		 */
		explicit Fenwick(std::span<T> span)
			requires std::is_same_v<Container, std::span<T>>
			:
			tree_(span),
			size_(span.size())
		{
		}

		// ========================================================================
		// Capacity
		// ========================================================================

		[[nodiscard]] constexpr size_type size() const noexcept
		{
			return size_;
		}

		[[nodiscard]] constexpr bool empty() const noexcept
		{
			return size_ == 0;
		}

		// ========================================================================
		// Modifiers
		// ========================================================================

		/**
		 * @brief Updates the value at the given index.
		 *
		 * Adds delta to the element at index, propagating the change through
		 * the Fenwick tree structure.
		 *
		 * @param index Position to update. Must be less than size()
		 * @param delta Amount to add to the element
		 */
		constexpr void update(const size_type index, const T& delta)
		{
			if (index >= size_)
			{
				return;
			}

			// 1-indexed in the tree structure
			for (size_type i = index + 1; i <= size_; i += i & -i)
			{
				tree_[i - 1] = tree_[i - 1] + delta;
			}
		}

		// ========================================================================
		// Queries
		// ========================================================================

		/**
		 * @brief Computes prefix sum over [0, index] (inclusive).
		 *
		 * @param index Right boundary (inclusive)
		 * @return Sum of elements from index 0 to index (inclusive)
		 */
		[[nodiscard]] constexpr T prefix(size_type index) const
		{
			if (index >= size_)
			{
				index = size_ - 1;
			}

			T result = T();
			for (size_type i = index + 1; i > 0; i -= i & -i)
			{
				result = result + tree_[i - 1];
			}
			return result;
		}

		/**
		 * @brief Computes sum over range [left, right] (inclusive on both ends).
		 *
		 * @param left Left boundary (inclusive)
		 * @param right Right boundary (inclusive)
		 * @return Sum of elements from left to right (inclusive)
		 */
		[[nodiscard]] constexpr T inclusive(const size_type left, const size_type right) const
		{
			if (left > right || right >= size_)
			{
				return T();
			}
			if (left == 0)
			{
				return prefix(right);
			}

			return prefix(right) + prefix(left - 1) * T(-1);
		}

		/**
		 * @brief Computes sum over range [left, right) (exclusive on right).
		 *
		 * @param left Left boundary (inclusive)
		 * @param right Right boundary (exclusive)
		 * @return Sum of elements from left to right-1 (inclusive)
		 */
		[[nodiscard]] constexpr T exclusive(const size_type left, size_type right) const
		{
			if (right > size_)
			{
				right = size_;
			}
			if (left >= right)
			{
				return T();
			}

			return inclusive(left, right - 1);
		}

		/**
		 * @brief Computes sum over entire array [0, size()).
		 *
		 * @return Sum of all elements
		 */
		[[nodiscard]] constexpr T sum() const
		{
			if (size_ == 0)
			{
				return T();
			}
			return prefix(size_ - 1);
		}

		// ========================================================================
		// Utilities
		// ========================================================================

		/**
		 * @brief Resets all values to zero.
		 */
		constexpr void clear()
		{
			for (size_type i = 0; i < size_; ++i)
			{
				tree_[i] = T(0);
			}
		}

	private:
		Container tree_;
		size_type size_;
	};

	// ========================================================================
	// Type Aliases
	// ========================================================================

	// Fenwick tree with dynamic std::vector backing
	template<typename T>
	using FenwickDynamic = Fenwick<T, std::vector<T>>;

	// Fenwick tree with non-owned std::span backing
	template<typename T>
	using FenwickSpan = Fenwick<T, std::span<T>>;

	// Fenwick tree with fixed std::array backing (requires template specialization at use site)
	template<typename T, std::size_t N>
	using FenwickFixed = Fenwick<T, std::array<T, N>>;
}
