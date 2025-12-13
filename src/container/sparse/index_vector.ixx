export module synodic.periapsis:container.sparse.index_vector;

import std;

export namespace peri::sparse
{
	/**
	 * @brief A dynamic sparse index set backed by a sorted vector.
	 *
	 * @details Provides O(log n) lookup, O(n) insertion/removal, and O(1) iteration over set indices.
	 *			Ideal for sparse sets where iteration order matters or where the set is
	 *			mostly read-only after construction.
	 *
	 * @tparam Index The index type
	 * @tparam Allocator The allocator type for the underlying vector
	 */
	template<std::integral Index = std::size_t, typename Allocator = std::allocator<Index>>
	class IndexVector
	{
	public:
		// ========================================================================
		// Type Definitions
		// ========================================================================

		using container_type  = std::vector<Index, Allocator>;
		using value_type      = Index;
		using allocator_type  = Allocator;
		using size_type       = std::size_t;
		using difference_type = std::ptrdiff_t;
		using reference       = container_type::reference;
		using const_reference = container_type::const_reference;
		using pointer         = container_type::pointer;
		using const_pointer   = container_type::const_pointer;
		using iterator        = container_type::iterator;
		using const_iterator  = container_type::const_iterator;

		static constexpr size_type NOT_VALID = static_cast<size_type>(-1);

		// ========================================================================
		// Construction
		// ========================================================================

		/**
		 * @brief Default constructor.
		 */
		constexpr IndexVector() = default;

		/**
		 * @brief Constructs with a specific allocator.
		 */
		constexpr explicit IndexVector(const Allocator& alloc) :
			indices_(alloc)
		{
		}

		/**
		 * @brief Constructs from an initializer list.
		 *
		 * @param init Initializer list of indices
		 * @param alloc Allocator to use
		 */
		constexpr IndexVector(std::initializer_list<value_type> init, const Allocator& alloc = Allocator()) :
			indices_(alloc)
		{
			reserve(init.size());
			for (const auto& value: init)
			{
				insert(value);
			}
		}

		/**
		 * @brief Constructs from a range.
		 *
		 * @param first Iterator to the beginning
		 * @param last Iterator to the end
		 * @param alloc Allocator to use
		 */
		template<std::input_iterator It>
		constexpr IndexVector(It first, It last, const Allocator& alloc = Allocator()) :
			indices_(alloc)
		{
			for (; first != last; ++first)
			{
				insert(*first);
			}
		}

		// ========================================================================
		// Iterators
		// ========================================================================

		[[nodiscard]] constexpr iterator begin() noexcept
		{
			return indices_.begin();
		}

		[[nodiscard]] constexpr const_iterator begin() const noexcept
		{
			return indices_.begin();
		}

		[[nodiscard]] constexpr const_iterator cbegin() const noexcept
		{
			return indices_.cbegin();
		}

		[[nodiscard]] constexpr iterator end() noexcept
		{
			return indices_.end();
		}

		[[nodiscard]] constexpr const_iterator end() const noexcept
		{
			return indices_.end();
		}

		[[nodiscard]] constexpr const_iterator cend() const noexcept
		{
			return indices_.cend();
		}

		// ========================================================================
		// Capacity
		// ========================================================================

		[[nodiscard]] constexpr size_type size() const noexcept
		{
			return indices_.size();
		}

		[[nodiscard]] constexpr bool empty() const noexcept
		{
			return indices_.empty();
		}

		[[nodiscard]] constexpr size_type capacity() const noexcept
		{
			return indices_.capacity();
		}

		constexpr void reserve(const size_type newCapacity)
		{
			indices_.reserve(newCapacity);
		}

		[[nodiscard]] constexpr allocator_type get_allocator() const noexcept
		{
			return indices_.get_allocator();
		}

		// ========================================================================
		// Element Access
		// ========================================================================

		/**
		 * @brief Returns the index at the given position in the sorted order.
		 */
		[[nodiscard]] constexpr const_reference operator[](const size_type pos) const noexcept
		{
			return indices_[pos];
		}

		/**
		 * @brief Returns the index at the given position with bounds checking.
		 */
		[[nodiscard]] constexpr const_reference at(const size_type pos) const
		{
			if (pos >= size())
			{
				throw std::out_of_range("IndexVector::at: position out of range");
			}
			return indices_[pos];
		}

		/**
		 * @brief Returns the smallest index.
		 */
		[[nodiscard]] constexpr const_reference front() const noexcept
		{
			return indices_.front();
		}

		/**
		 * @brief Returns the largest index.
		 */
		[[nodiscard]] constexpr const_reference back() const noexcept
		{
			return indices_.back();
		}

		// ========================================================================
		// Lookup
		// ========================================================================

		/**
		 * @brief Checks if the given index is present.
		 */
		[[nodiscard]] constexpr bool contains(const value_type index) const noexcept
		{
			return find(index) != end();
		}

		/**
		 * @brief Finds the position of the given index.
		 *
		 * @return Iterator to the index, or end if not found
		 */
		[[nodiscard]] constexpr const_iterator find(const value_type index) const noexcept
		{
			const auto it = std::lower_bound(begin(), end(), index);
			if (it != end() && *it == index)
			{
				return it;
			}
			return end();
		}

		/**
		 * @brief Returns the position of the given index in sorted order.
		 *
		 * @return Position of the index, or NOT_VALID if not found
		 */
		[[nodiscard]] constexpr size_type rank(const value_type index) const noexcept
		{
			const auto it = find(index);
			if (it == end())
			{
				return NOT_VALID;
			}
			return static_cast<size_type>(std::distance(cbegin(), it));
		}

		/**
		 * @brief Returns the index at the given position.
		 *
		 * @return The index at that position, or NOT_VALID if out of range
		 */
		[[nodiscard]] constexpr value_type select(const size_type pos) const noexcept
		{
			if (pos >= size())
			{
				return static_cast<value_type>(NOT_VALID);
			}
			return indices_[pos];
		}

		/**
		 * @brief Counts indices in range [left, right].
		 */
		[[nodiscard]] constexpr size_type count(const value_type left, const value_type right) const noexcept
		{
			if (left > right || empty())
			{
				return 0;
			}
			const auto lower = std::lower_bound(begin(), end(), left);
			const auto upper = std::upper_bound(lower, end(), right);
			return static_cast<size_type>(std::distance(lower, upper));
		}

		// ========================================================================
		// Modifiers
		// ========================================================================

		/**
		 * @brief Inserts an index into the set.
		 *
		 * @return true if inserted, false if already present
		 */
		constexpr bool insert(const value_type index)
		{
			const auto it = std::lower_bound(begin(), end(), index);
			if (it != end() && *it == index)
			{
				return false;
			}
			indices_.insert(it, index);
			return true;
		}

		/**
		 * @brief Removes an index from the set.
		 *
		 * @return true if removed, false if not present
		 */
		constexpr bool erase(const value_type index)
		{
			const auto it = std::lower_bound(begin(), end(), index);
			if (it == end() || *it != index)
			{
				return false;
			}
			indices_.erase(it);
			return true;
		}

		/**
		 * @brief Clears all indices.
		 */
		constexpr void clear() noexcept
		{
			indices_.clear();
		}

		/**
		 * @brief Inserts multiple indices efficiently using sort-and-merge.
		 *
		 * @return Number of indices actually inserted, excluding duplicates
		 */
		template<std::ranges::input_range Range>
		constexpr size_type insert_range(Range&& range)
			requires std::convertible_to<std::ranges::range_value_t<Range>, value_type>
		{
			std::vector<value_type> newIndices(std::ranges::begin(range), std::ranges::end(range));
			if (newIndices.empty())
			{
				return 0;
			}

			std::ranges::sort(newIndices);
			const auto uniqueEnd = std::ranges::unique(newIndices);
			newIndices.erase(uniqueEnd.begin(), uniqueEnd.end());

			std::vector<value_type> merged;
			merged.reserve(size() + newIndices.size());
			std::ranges::set_union(indices_, newIndices, std::back_inserter(merged));

			const size_type insertedCount = merged.size() - size();
			indices_                      = std::move(merged);
			return insertedCount;
		}

		/**
		 * @brief Inserts multiple indices from an initializer list.
		 */
		constexpr size_type insert_range(std::initializer_list<value_type> indices)
		{
			return insert_range(std::ranges::subrange(indices.begin(), indices.end()));
		}

		// ========================================================================
		// Set Operations
		// ========================================================================

		/**
		 * @brief Computes the union with another IndexVector.
		 */
		[[nodiscard]] constexpr IndexVector union_with(const IndexVector& other) const
		{
			IndexVector result;
			result.reserve(size() + other.size());
			std::ranges::set_union(*this, other, std::back_inserter(result.indices_));
			return result;
		}

		/**
		 * @brief Computes the intersection with another IndexVector.
		 */
		[[nodiscard]] constexpr IndexVector intersect(const IndexVector& other) const
		{
			IndexVector result;
			result.reserve(std::min(size(), other.size()));
			std::ranges::set_intersection(*this, other, std::back_inserter(result.indices_));
			return result;
		}

		/**
		 * @brief Computes the difference: elements in this but not in `other`.
		 */
		[[nodiscard]] constexpr IndexVector minus(const IndexVector& other) const
		{
			IndexVector result;
			result.reserve(size());
			std::ranges::set_difference(*this, other, std::back_inserter(result.indices_));
			return result;
		}

		// ========================================================================
		// Comparison
		// ========================================================================

		[[nodiscard]] constexpr bool operator==(const IndexVector& other) const noexcept
		{
			return indices_ == other.indices_;
		}

		[[nodiscard]] constexpr auto operator<=>(const IndexVector& other) const noexcept
		{
			return std::lexicographical_compare_three_way(begin(), end(), other.begin(), other.end());
		}

		// ========================================================================
		// Underlying Access
		// ========================================================================

		/**
		 * @brief Returns the underlying container.
		 */
		[[nodiscard]] constexpr const container_type& data() const noexcept
		{
			return indices_;
		}

	private:
		container_type indices_;
	};

	// ========================================================================
	// InplaceIndexVector - Fixed-capacity sparse index set using array storage
	// ========================================================================

	/**
	 * @brief A fixed-capacity sparse index set backed by a sorted array.
	 *
	 * @details Provides O(log n) lookup, O(n) insertion/removal,
	 *          and O(1) iteration over set indices. No heap allocation.
	 *
	 * @tparam N Maximum number of indices
	 * @tparam Index The index type
	 */
	template<std::size_t N, std::integral Index = std::size_t>
	class InplaceIndexVector
	{
	public:
		// ========================================================================
		// Type Definitions
		// ========================================================================

		using container_type  = std::array<Index, N>;
		using value_type      = Index;
		using size_type       = std::size_t;
		using difference_type = std::ptrdiff_t;
		using reference       = container_type::reference;
		using const_reference = container_type::const_reference;
		using pointer         = container_type::pointer;
		using const_pointer   = container_type::const_pointer;
		using iterator        = container_type::iterator;
		using const_iterator  = container_type::const_iterator;

		static constexpr size_type NOT_VALID = static_cast<size_type>(-1);
		static constexpr size_type CAPACITY  = N;

		// ========================================================================
		// Construction
		// ========================================================================

		/**
		 * @brief Default constructor.
		 */
		constexpr InplaceIndexVector() = default;

		/**
		 * @brief Constructs from an initializer list.
		 *
		 * @param init Initializer list of indices. Excess elements are ignored.
		 */
		constexpr InplaceIndexVector(std::initializer_list<value_type> init)
		{
			for (const auto& value: init)
			{
				if (!insert(value))
				{
					break; // Full
				}
			}
		}

		/**
		 * @brief Constructs from a range.
		 *
		 * @param first Iterator to the beginning
		 * @param last Iterator to the end
		 */
		template<std::input_iterator It>
		constexpr InplaceIndexVector(It first, It last)
		{
			for (; first != last; ++first)
			{
				if (!insert(*first))
				{
					break; // Full
				}
			}
		}

		// ========================================================================
		// Iterators
		// ========================================================================

		[[nodiscard]] constexpr iterator begin() noexcept
		{
			return indices_.begin();
		}

		[[nodiscard]] constexpr const_iterator begin() const noexcept
		{
			return indices_.begin();
		}

		[[nodiscard]] constexpr const_iterator cbegin() const noexcept
		{
			return indices_.cbegin();
		}

		[[nodiscard]] constexpr iterator end() noexcept
		{
			return indices_.begin() + static_cast<difference_type>(size_);
		}

		[[nodiscard]] constexpr const_iterator end() const noexcept
		{
			return indices_.begin() + static_cast<difference_type>(size_);
		}

		[[nodiscard]] constexpr const_iterator cend() const noexcept
		{
			return indices_.cbegin() + static_cast<difference_type>(size_);
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

		[[nodiscard]] static constexpr size_type capacity() noexcept
		{
			return N;
		}

		[[nodiscard]] constexpr bool full() const noexcept
		{
			return size_ >= N;
		}

		// ========================================================================
		// Element Access
		// ========================================================================

		/**
		 * @brief Returns the index at the given position in the sorted order.
		 */
		[[nodiscard]] constexpr const_reference operator[](const size_type pos) const noexcept
		{
			return indices_[pos];
		}

		/**
		 * @brief Returns the index at the given position with bounds checking.
		 */
		[[nodiscard]] constexpr const_reference at(const size_type pos) const
		{
			if (pos >= size_)
			{
				throw std::out_of_range("InplaceIndexVector::at: position out of range");
			}
			return indices_[pos];
		}

		/**
		 * @brief Returns the smallest index.
		 */
		[[nodiscard]] constexpr const_reference front() const noexcept
		{
			return indices_[0];
		}

		/**
		 * @brief Returns the largest index.
		 */
		[[nodiscard]] constexpr const_reference back() const noexcept
		{
			return indices_[size_ - 1];
		}

		// ========================================================================
		// Lookup
		// ========================================================================

		/**
		 * @brief Checks if the given index is present.
		 */
		[[nodiscard]] constexpr bool contains(const value_type index) const noexcept
		{
			return find(index) != end();
		}

		/**
		 * @brief Finds the position of the given index.
		 *
		 * @return Iterator to the index, or end if not found
		 */
		[[nodiscard]] constexpr const_iterator find(const value_type index) const noexcept
		{
			const auto it = std::lower_bound(begin(), end(), index);
			if (it != end() && *it == index)
			{
				return it;
			}
			return end();
		}

		/**
		 * @brief Returns the position of the given index in sorted order.
		 *
		 * @return Position of the index, or NOT_VALID if not found
		 */
		[[nodiscard]] constexpr size_type rank(const value_type index) const noexcept
		{
			const auto it = find(index);
			if (it == end())
			{
				return NOT_VALID;
			}
			return static_cast<size_type>(it - cbegin());
		}

		/**
		 * @brief Returns the index at the given position.
		 *
		 * @return The index at that position, or NOT_VALID if out of range
		 */
		[[nodiscard]] constexpr value_type select(const size_type pos) const noexcept
		{
			if (pos >= size_)
			{
				return static_cast<value_type>(NOT_VALID);
			}
			return indices_[pos];
		}

		/**
		 * @brief Counts indices in range [left, right].
		 */
		[[nodiscard]] constexpr size_type count(const value_type left, const value_type right) const noexcept
		{
			if (left > right || empty())
			{
				return 0;
			}
			const auto lower = std::lower_bound(begin(), end(), left);
			const auto upper = std::upper_bound(lower, end(), right);
			return static_cast<size_type>(upper - lower);
		}

		// ========================================================================
		// Modifiers
		// ========================================================================

		/**
		 * @brief Inserts an index into the set.
		 *
		 * @return true if inserted, false if already present or full
		 */
		constexpr bool insert(const value_type index)
		{
			const auto it = std::lower_bound(begin(), end(), index);
			if (it != end() && *it == index)
			{
				return false; // Already exists
			}

			if (size_ >= N)
			{
				return false; // Full
			}

			const auto insertPos = static_cast<size_type>(it - cbegin());
			auto insertIt        = indices_.begin() + static_cast<difference_type>(insertPos);
			std::move_backward(
				insertIt,
				indices_.begin() + static_cast<difference_type>(size_),
				indices_.begin() + static_cast<difference_type>(size_ + 1));
			*insertIt = index;
			++size_;
			return true;
		}

		/**
		 * @brief Removes an index from the set.
		 *
		 * @return true if removed, false if not present
		 */
		constexpr bool erase(const value_type index)
		{
			const auto it = std::lower_bound(begin(), end(), index);
			if (it == end() || *it != index)
			{
				return false;
			}

			const auto erasePos = static_cast<size_type>(it - cbegin());
			auto eraseIt        = indices_.begin() + static_cast<difference_type>(erasePos);
			std::move(eraseIt + 1, indices_.begin() + static_cast<difference_type>(size_), eraseIt);
			--size_;
			return true;
		}

		/**
		 * @brief Clears all indices.
		 */
		constexpr void clear() noexcept
		{
			size_ = 0;
		}

		// ========================================================================
		// Comparison
		// ========================================================================

		[[nodiscard]] constexpr bool operator==(const InplaceIndexVector& other) const noexcept
		{
			if (size_ != other.size_)
			{
				return false;
			}
			return std::equal(begin(), end(), other.begin());
		}

		[[nodiscard]] constexpr auto operator<=>(const InplaceIndexVector& other) const noexcept
		{
			return std::lexicographical_compare_three_way(begin(), end(), other.begin(), other.end());
		}

		// ========================================================================
		// Underlying Access
		// ========================================================================

		/**
		 * @brief Returns the underlying array.
		 */
		[[nodiscard]] constexpr const container_type& data() const noexcept
		{
			return indices_;
		}

	private:
		container_type indices_{};
		size_type size_ = 0;
	};
}
