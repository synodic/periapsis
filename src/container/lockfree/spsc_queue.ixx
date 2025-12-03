export module synodic.library:container.lockfree.spsc_queue;

import :exception.not_implemented;
import :type.concepts.container;
import :type.concepts.sequence_container;

import std;

export namespace synodic::lockfree
{
	// Defines the set of types that SPSCQueue supports
	template<typename T>
	concept spsc_queue_type = std::is_nothrow_constructible_v<T> and std::is_nothrow_destructible_v<T>;

	// Defines the set of containers that SPSCQueue supports
	template<typename T>
	concept spsc_queue_container = inplace_constructing_sequence_container<T>;

	// Convenience concept for identifying SPSCQueue backing containers
	template<typename T, typename ValueType>
	concept spsc_queue_container_of = container_of<T, ValueType> && spsc_queue_container<T>;

	/**
	 * @brief An adapter for containers that enforces single producer, single consumer atomic access. This queue is an
	 *		adapter and stores all relevant tracking information external to the wrapped container. As a result the
	 *		internal size tracking of the container is wasted. If you need memory efficiency for the queue itself this
	 *		is not the type to use. However, this design allows the extraction the underlying container into a
	 *		single-threaded context.
	 * @tparam T The type to store
	 * @tparam Container The container for storage backing
	 */
	template<spsc_queue_type T, spsc_queue_container_of<T> Container>
	class SPSCQueue
	{
	public:
		using container_type		 = Container;
		using value_type			 = typename Container::value_type;
		using size_type				 = typename Container::size_type;
		using difference_type		 = typename Container::difference_type;
		using reference				 = typename Container::reference;
		using const_reference		 = typename Container::const_reference;
		using iterator				 = typename Container::iterator;
		using const_iterator		 = typename Container::const_iterator;
		using reverse_iterator		 = typename Container::reverse_iterator;
		using const_reverse_iterator = typename Container::const_reverse_iterator;

		// Construction

		constexpr explicit SPSCQueue(const Container& container);
		constexpr explicit SPSCQueue(Container&& container);
		constexpr SPSCQueue(const SPSCQueue& other)		= delete;
		constexpr SPSCQueue(SPSCQueue&& other) noexcept = delete;

		// TODO: Implement range construction
		// template<typename InputIt>
		// constexpr SPSCQueue(InputIt first, InputIt last);

		// Destructor

		// TODO: Guard against invalid usage
		constexpr ~SPSCQueue() = default;

		// Assignment

		constexpr SPSCQueue& operator=(const SPSCQueue& other)	   = delete;
		constexpr SPSCQueue& operator=(SPSCQueue&& other) noexcept = delete;

		// Accessors

		/**
		 * @brief Removes the wrapped container from the queue. As any usage other than a single reader and
		 *		single writer is invalid, this function expects that there will be no reads or writes
		 *		during its execution. TODO: Guard against invalid usage
		 * @return The underlying container
		 */
		constexpr container_type extract() noexcept;

		// Capacity

		[[nodiscard]] constexpr bool empty() const noexcept;
		constexpr size_type size() const noexcept;
		constexpr size_type max_size() const noexcept;

		// Modifiers

		constexpr void push(const T& value);
		constexpr void push(T&& value);

		constexpr bool try_push(const T& value);
		constexpr bool try_push(T&& value);

		/**
		 * @brief
		 */
		constexpr std::optional<T> pop() noexcept;

		template<typename... Args>
		constexpr void emplace(Args&&... args);

		template<typename... Args>
		constexpr bool try_emplace(Args&&... args);

		// Comparison

		constexpr auto operator<=>(const SPSCQueue&) const = default;

	private:
		static constexpr size_t cacheLineSize = std::hardware_destructive_interference_size;

		// Tracks capacity external to the container to guarantee alignment. Placed by front
		// to get bundled with container_
		alignas(cacheLineSize) size_type capacity_;

		alignas(cacheLineSize) Container container_;

		alignas(cacheLineSize) std::atomic<size_type> writeIndex_;
		alignas(cacheLineSize) size_type cachedReadIndex_;
		alignas(cacheLineSize) std::atomic<size_type> readIndex_;
		alignas(cacheLineSize) size_type cachedWriteIndex_;

		alignas(cacheLineSize) std::atomic_flag full_;

		// The padding used to avoid cache line overlap with the full flag
		std::array<std::byte, cacheLineSize - sizeof cachedWriteIndex_> padding_;
	};

	template<spsc_queue_type T, spsc_queue_container_of<T> Container>
	constexpr SPSCQueue<T, Container>::SPSCQueue(const Container& container) :
		capacity_(container.max_size()),
		container_(container),
		writeIndex_(0),
		cachedReadIndex_(0),
		readIndex_(0),
		cachedWriteIndex_(0)
	{
	}

	template<spsc_queue_type T, spsc_queue_container_of<T> Container>
	constexpr SPSCQueue<T, Container>::SPSCQueue(Container&& container) :
		capacity_(container.max_size()),
		container_(std::move(container)),
		writeIndex_(0),
		cachedReadIndex_(0),
		readIndex_(0),
		cachedWriteIndex_(0)
	{
	}

	template<spsc_queue_type T, spsc_queue_container_of<T> Container>
	constexpr typename SPSCQueue<T, Container>::container_type SPSCQueue<T, Container>::extract() noexcept
	{
		std::rotate(container_.begin(), container_.begin(), container_.begin());

		// container_

		// TODO: Mark as full, as if in a constructed state
		capacity_		  = 0;
		writeIndex_		  = 0;
		cachedReadIndex_  = 0;
		readIndex_		  = 0;
		cachedWriteIndex_ = 0;

		return std::move(container_);
	}

	template<spsc_queue_type T, spsc_queue_container_of<T> Container>
	constexpr bool SPSCQueue<T, Container>::empty() const noexcept
	{
		return writeIndex_.load(std::memory_order_acquire) == readIndex_.load(std::memory_order_acquire);
	}

	template<spsc_queue_type T, spsc_queue_container_of<T> Container>
	constexpr typename SPSCQueue<T, Container>::size_type SPSCQueue<T, Container>::size() const noexcept
	{
		std::ptrdiff_t difference =
			writeIndex_.load(std::memory_order_acquire) - readIndex_.load(std::memory_order_acquire);
		if (difference < 0)
		{
			difference += capacity_;
		}
		return static_cast<size_type>(difference);
	}

	template<spsc_queue_type T, spsc_queue_container_of<T> Container>
	constexpr typename SPSCQueue<T, Container>::size_type SPSCQueue<T, Container>::max_size() const noexcept
	{
		return capacity_;
	}

	template<spsc_queue_type T, spsc_queue_container_of<T> Container>
	constexpr void SPSCQueue<T, Container>::push(const T& value)
	{
		throw NotImplementedException();
	}

	template<spsc_queue_type T, spsc_queue_container_of<T> Container>
	constexpr void SPSCQueue<T, Container>::push(T&& value)
	{
		throw NotImplementedException();
	}

	template<spsc_queue_type T, spsc_queue_container_of<T> Container>
	constexpr bool SPSCQueue<T, Container>::try_push(const T& value)
	{
		throw NotImplementedException();
	}

	template<spsc_queue_type T, spsc_queue_container_of<T> Container>
	constexpr bool SPSCQueue<T, Container>::try_push(T&& value)
	{
		throw NotImplementedException();
	}

	template<spsc_queue_type T, spsc_queue_container_of<T> Container>
	constexpr std::optional<T> SPSCQueue<T, Container>::pop() noexcept
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

	template<spsc_queue_type T, spsc_queue_container_of<T> Container>
	template<typename... Args>
	constexpr void SPSCQueue<T, Container>::emplace(Args&&... args)
	{
		throw NotImplementedException();
	}

	template<spsc_queue_type T, spsc_queue_container_of<T> Container>
	template<typename... Args>
	constexpr bool SPSCQueue<T, Container>::try_emplace(Args&&... args)
	{
		throw NotImplementedException();
	}

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
