export module synodic.library:container.fixed.ring_buffer;

import std;

export namespace synodic
{
	/**
	 * @brief A constexpr-capable, fixed-capacity ring buffer (circular buffer).
	 *        Provides the container interface required by std::queue and std::stack adapters.
	 *
	 *        Key features:
	 *        - Fully constexpr (usable at compile-time)
	 *        - O(1) push/pop at both ends
	 *        - Zero heap allocation (uses std::array internally)
	 *        - Compatible with std::queue<T, FixedRingBuffer<T, N>>
	 *
	 * @tparam T The element type
	 * @tparam Capacity Maximum number of elements
	 */
	template<typename T, std::size_t Capacity>
		requires(Capacity > 0)
	class FixedRingBuffer
	{
	public:
		// ========================================================================
		// Container Type Definitions
		// ========================================================================

		using value_type	  = T;
		using size_type		  = std::size_t;
		using difference_type = std::ptrdiff_t;
		using reference		  = T&;
		using const_reference = const T&;
		using pointer		  = T*;
		using const_pointer	  = const T*;

		// ========================================================================
		// Construction
		// ========================================================================

		constexpr FixedRingBuffer() = default;

		constexpr FixedRingBuffer(std::initializer_list<T> init)
		{
			for (const auto& value: init)
			{
				push_back(value);
			}
		}

		// ========================================================================
		// Capacity
		// ========================================================================

		[[nodiscard]] constexpr bool empty() const noexcept
		{
			return size_ == 0;
		}

		[[nodiscard]] constexpr size_type size() const noexcept
		{
			return size_;
		}

		[[nodiscard]] static constexpr size_type max_size() noexcept
		{
			return Capacity;
		}

		[[nodiscard]] static constexpr size_type capacity() noexcept
		{
			return Capacity;
		}

		[[nodiscard]] constexpr bool full() const noexcept
		{
			return size_ == Capacity;
		}

		// ========================================================================
		// Element Access
		// ========================================================================

		[[nodiscard]] constexpr reference front()
		{
			return data_[head_];
		}

		[[nodiscard]] constexpr const_reference front() const
		{
			return data_[head_];
		}

		[[nodiscard]] constexpr reference back()
		{
			return data_[prev_index(tail_)];
		}

		[[nodiscard]] constexpr const_reference back() const
		{
			return data_[prev_index(tail_)];
		}

		[[nodiscard]] constexpr reference operator[](size_type pos)
		{
			return data_[(head_ + pos) % Capacity];
		}

		[[nodiscard]] constexpr const_reference operator[](size_type pos) const
		{
			return data_[(head_ + pos) % Capacity];
		}

		// ========================================================================
		// Modifiers - Back Operations (for queue-like usage)
		// ========================================================================

		constexpr void push_back(const T& value)
		{
			data_[tail_] = value;
			tail_		 = next_index(tail_);
			++size_;
		}

		constexpr void push_back(T&& value)
		{
			data_[tail_] = std::move(value);
			tail_		 = next_index(tail_);
			++size_;
		}

		template<typename... Args>
		constexpr reference emplace_back(Args&&... args)
		{
			data_[tail_] = T(std::forward<Args>(args)...);
			auto& ref	 = data_[tail_];
			tail_		 = next_index(tail_);
			++size_;
			return ref;
		}

		constexpr void pop_back()
		{
			tail_ = prev_index(tail_);
			--size_;
		}

		// ========================================================================
		// Modifiers - Front Operations (for deque-like usage)
		// ========================================================================

		constexpr void push_front(const T& value)
		{
			head_		 = prev_index(head_);
			data_[head_] = value;
			++size_;
		}

		constexpr void push_front(T&& value)
		{
			head_		 = prev_index(head_);
			data_[head_] = std::move(value);
			++size_;
		}

		template<typename... Args>
		constexpr reference emplace_front(Args&&... args)
		{
			head_		 = prev_index(head_);
			data_[head_] = T(std::forward<Args>(args)...);
			++size_;
			return data_[head_];
		}

		constexpr void pop_front()
		{
			head_ = next_index(head_);
			--size_;
		}

		// ========================================================================
		// Modifiers - General
		// ========================================================================

		constexpr void clear() noexcept
		{
			head_ = 0;
			tail_ = 0;
			size_ = 0;
		}

		constexpr void swap(FixedRingBuffer& other) noexcept(std::is_nothrow_swappable_v<T>)
		{
			using std::swap;
			swap(data_, other.data_);
			swap(head_, other.head_);
			swap(tail_, other.tail_);
			swap(size_, other.size_);
		}

		// ========================================================================
		// Comparison
		// ========================================================================

		constexpr bool operator==(const FixedRingBuffer& other) const
		{
			if (size_ != other.size_)
			{
				return false;
			}
			for (size_type i = 0; i < size_; ++i)
			{
				if ((*this)[i] != other[i])
				{
					return false;
				}
			}
			return true;
		}

		constexpr auto operator<=>(const FixedRingBuffer& other) const
		{
			return std::lexicographical_compare_three_way(
				data_.begin(),
				data_.begin() + size_,
				other.data_.begin(),
				other.data_.begin() + other.size_);
		}

	private:
		std::array<T, Capacity> data_ {};
		size_type head_ {0};  // Index of first element
		size_type tail_ {0};  // Index one past last element
		size_type size_ {0};  // Current number of elements

		[[nodiscard]] constexpr size_type next_index(size_type idx) const noexcept
		{
			return (idx + 1) % Capacity;
		}

		[[nodiscard]] constexpr size_type prev_index(size_type idx) const noexcept
		{
			return (idx + Capacity - 1) % Capacity;
		}
	};

	// ============================================================================
	// Non-Member Functions
	// ============================================================================

	template<typename T, std::size_t Capacity>
	constexpr void swap(FixedRingBuffer<T, Capacity>& lhs, FixedRingBuffer<T, Capacity>& rhs)
		noexcept(noexcept(lhs.swap(rhs)))
	{
		lhs.swap(rhs);
	}

	// ============================================================================
	// Convenience Aliases for Adapter Usage
	// ============================================================================

	/**
	 * @brief A constexpr-capable queue using FixedRingBuffer as the underlying container.
	 *        Example: FixedQueue<int, 64> queue;
	 */
	template<typename T, std::size_t Capacity>
	using FixedQueue = std::queue<T, FixedRingBuffer<T, Capacity>>;

	/**
	 * @brief A constexpr-capable stack using FixedRingBuffer as the underlying container.
	 *        Example: FixedStack<int, 64> stack;
	 */
	template<typename T, std::size_t Capacity>
	using FixedStack = std::stack<T, FixedRingBuffer<T, Capacity>>;
}
