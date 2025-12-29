export module synodic.periapsis:container.lockfree.spsc_queue;

import :exception;

import std;

export namespace peri::lockfree
{
	// Defines the set of types that SPSCQueue supports
	template<typename T>
	concept spsc_queue_type = std::is_nothrow_constructible_v<T> and std::is_nothrow_destructible_v<T>;

	// Defines the set of containers that SPSCQueue supports.
	// Requires only what SPSCQueue actually uses: size(), operator[], and reverse iterators.
	template<typename T>
	concept spsc_queue_container = requires(T& c, const T& cc, typename T::size_type i) {
		typename T::value_type;
		typename T::size_type;
		typename T::reference;
		typename T::const_reference;
		typename T::iterator;
		typename T::reverse_iterator;
		typename T::const_reverse_iterator;
		{ cc.size() } -> std::same_as<typename T::size_type>;
		{ c[i] } -> std::same_as<typename T::reference>;
		{ cc[i] } -> std::same_as<typename T::const_reference>;
		{ c.begin() } -> std::same_as<typename T::iterator>;
	};

	// Convenience concept for identifying SPSCQueue backing containers
	template<typename T, typename ValueType>
	concept spsc_queue_container_of = spsc_queue_container<T> and std::same_as<typename T::value_type, ValueType>;

	/**
	 * @brief An adapter for containers that enforces single producer, single consumer atomic access. This queue is an
	 *		adapter and stores all relevant tracking information external to the wrapped container. As a result the
	 *		internal size tracking of the container is wasted. If you need memory efficiency for the queue itself this
	 *		is not the type to use. However, this design allows the extraction the underlying container into a
	 *		single-threaded context.
	 *
	 *		Note: This is a circular buffer implementation where one slot is reserved to distinguish between empty
	 *		and full states. Therefore, a queue with capacity N can store at most N-1 elements.
	 *
	 * @tparam T The type to store
	 * @tparam Container The container for storage backing
	 */
	template<spsc_queue_type T, spsc_queue_container_of<T> Container>
	class SPSCQueue
	{
	public:
		using container_type		 = Container;
		using value_type			 = Container::value_type;
		using size_type				 = Container::size_type;
		using difference_type		 = Container::difference_type;
		using reference				 = Container::reference;
		using const_reference		 = Container::const_reference;
		using iterator				 = Container::iterator;
		using const_iterator		 = Container::const_iterator;
		using reverse_iterator		 = Container::reverse_iterator;
		using const_reverse_iterator = Container::const_reverse_iterator;

		// ========================================================================
		// Construction
		// ========================================================================

		constexpr explicit SPSCQueue(const Container& container) :
			capacity_(container.size()),
			container_(container),
			writeIndex_(0),
			cachedReadIndex_(0),
			readIndex_(0),
			cachedWriteIndex_(0)
		{
		}

		constexpr explicit SPSCQueue(Container&& container) :
			capacity_(container.size()),
			container_(std::move(container)),
			writeIndex_(0),
			cachedReadIndex_(0),
			readIndex_(0),
			cachedWriteIndex_(0)
		{
		}

		constexpr SPSCQueue(const SPSCQueue& other)		= delete;
		constexpr SPSCQueue(SPSCQueue&& other) noexcept = delete;

		// ========================================================================
		// Destructor
		// ========================================================================

		constexpr ~SPSCQueue() = default;

		// ========================================================================
		// Assignment
		// ========================================================================

		constexpr SPSCQueue& operator=(const SPSCQueue& other)	   = delete;
		constexpr SPSCQueue& operator=(SPSCQueue&& other) noexcept = delete;

		// ========================================================================
		// Accessors
		// ========================================================================

		/**
		 * @brief Removes the wrapped container from the queue. As any usage other than a single reader and
		 *		single writer is invalid, this function expects that there will be no reads or writes
		 *		during its execution.
		 * @return The underlying container
		 */
		constexpr container_type extract() noexcept
		{
			std::rotate(container_.begin(), container_.begin(), container_.begin());

			capacity_		  = 0;
			writeIndex_		  = 0;
			cachedReadIndex_  = 0;
			readIndex_		  = 0;
			cachedWriteIndex_ = 0;

			return std::move(container_);
		}

		// ========================================================================
		// Capacity
		// ========================================================================

		[[nodiscard]] constexpr bool empty() const noexcept
		{
			return writeIndex_.load(std::memory_order_acquire) == readIndex_.load(std::memory_order_acquire);
		}

		constexpr size_type size() const noexcept
		{
			std::ptrdiff_t difference =
				writeIndex_.load(std::memory_order_acquire) - readIndex_.load(std::memory_order_acquire);
			if (difference < 0)
			{
				difference += capacity_;
			}
			return static_cast<size_type>(difference);
		}

		constexpr size_type max_size() const noexcept
		{
			return capacity_ > 0 ? capacity_ - 1 : 0;
		}

		// ========================================================================
		// Modifiers - Push Operations
		// ========================================================================

		constexpr void push(const T& value)
		{
			if (!try_push(value))
			{
				throw std::out_of_range("SPSCQueue is full");
			}
		}

		constexpr void push(T&& value)
		{
			if (!try_push(std::move(value)))
			{
				throw std::out_of_range("SPSCQueue is full");
			}
		}

		constexpr bool try_push(const T& value)
		{
			const auto writeIndex = writeIndex_.load(std::memory_order_relaxed);
			auto nextWriteIdx	  = writeIndex + 1;
			if (nextWriteIdx == capacity_)
			{
				nextWriteIdx = 0;
			}

			if (nextWriteIdx == cachedReadIndex_)
			{
				cachedReadIndex_ = readIndex_.load(std::memory_order_acquire);
				if (nextWriteIdx == cachedReadIndex_)
				{
					full_.test_and_set(std::memory_order_release);
					return false;
				}
			}

			container_[writeIndex] = value;
			writeIndex_.store(nextWriteIdx, std::memory_order_release);
			return true;
		}

		constexpr bool try_push(T&& value)
		{
			const auto writeIndex = writeIndex_.load(std::memory_order_relaxed);
			auto nextWriteIdx	  = writeIndex + 1;
			if (nextWriteIdx == capacity_)
			{
				nextWriteIdx = 0;
			}

			if (nextWriteIdx == cachedReadIndex_)
			{
				cachedReadIndex_ = readIndex_.load(std::memory_order_acquire);
				if (nextWriteIdx == cachedReadIndex_)
				{
					full_.test_and_set(std::memory_order_release);
					return false;
				}
			}

			container_[writeIndex] = std::move(value);
			writeIndex_.store(nextWriteIdx, std::memory_order_release);
			return true;
		}

		// ========================================================================
		// Modifiers - Pop Operations
		// ========================================================================

		/**
		 * @brief Pops an element from the queue if available
		 * @return An optional containing the popped element, or empty if queue is empty
		 */
		constexpr std::optional<T> pop() noexcept
		{
			const auto readIndex = readIndex_.load(std::memory_order_relaxed);
			if (readIndex == cachedWriteIndex_)
			{
				cachedWriteIndex_ = writeIndex_.load(std::memory_order_acquire);
				if (cachedWriteIndex_ == readIndex)
				{
					return {};
				}
			}

			auto nextReadIdx = readIndex + 1;
			if (nextReadIdx == capacity_)
			{
				nextReadIdx = 0;
			}
			readIndex_.store(nextReadIdx, std::memory_order_release);

			return std::move(container_[readIndex]);
		}

		// ========================================================================
		// Modifiers - Emplace Operations
		// ========================================================================

		template<typename... Args>
		constexpr void emplace(Args&&... args)
		{
			if (!try_emplace(std::forward<Args>(args)...))
			{
				throw std::out_of_range("SPSCQueue is full");
			}
		}

		template<typename... Args>
		constexpr bool try_emplace(Args&&... args)
		{
			const auto writeIndex = writeIndex_.load(std::memory_order_relaxed);
			auto nextWriteIdx	  = writeIndex + 1;
			if (nextWriteIdx == capacity_)
			{
				nextWriteIdx = 0;
			}

			if (nextWriteIdx == cachedReadIndex_)
			{
				cachedReadIndex_ = readIndex_.load(std::memory_order_acquire);
				if (nextWriteIdx == cachedReadIndex_)
				{
					full_.test_and_set(std::memory_order_release);
					return false;
				}
			}

			container_[writeIndex] = T(std::forward<Args>(args)...);
			writeIndex_.store(nextWriteIdx, std::memory_order_release);
			return true;
		}

		// ========================================================================
		// Comparison
		// ========================================================================

		constexpr auto operator<=>(const SPSCQueue&) const = default;

	private:
		static constexpr size_t CACHE_LINE_SIZE = std::hardware_destructive_interference_size;

		// Tracks capacity external to the container to guarantee alignment. Placed by front
		// to get bundled with container_
		alignas(CACHE_LINE_SIZE) size_type capacity_;

		alignas(CACHE_LINE_SIZE) Container container_;

		alignas(CACHE_LINE_SIZE) std::atomic<size_type> writeIndex_;
		alignas(CACHE_LINE_SIZE) size_type cachedReadIndex_;
		alignas(CACHE_LINE_SIZE) std::atomic<size_type> readIndex_;
		alignas(CACHE_LINE_SIZE) size_type cachedWriteIndex_;

		alignas(CACHE_LINE_SIZE) std::atomic_flag full_;

		// The padding used to avoid cache line overlap with the full flag
		std::array<std::byte, CACHE_LINE_SIZE - sizeof cachedWriteIndex_> padding_;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Deduction guides as specified: https://en.cppreference.com/w/cpp/container/queue/deduction_guides
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<typename Container>
	SPSCQueue(Container) -> SPSCQueue<typename Container::value_type, Container>;

	template<typename InputIt>
	SPSCQueue(InputIt, InputIt) -> SPSCQueue<
		typename std::iterator_traits<InputIt>::value_type::value_type,
		typename std::iterator_traits<InputIt>::value_type>;
}
