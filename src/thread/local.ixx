export module synodic.periapsis:thread.local;

import std;

export namespace peri
{
	/**
	 * @brief Thread-local storage wrapper allowing non-static member thread-local data.
	 *		Each thread lazily constructs its own instance of T on first access.
	 * @tparam T The type to store per-thread.
	 */
	template<typename T>
		requires std::move_constructible<T>
	class ThreadLocal
	{
	public:
		using Factory = std::move_only_function<T()>;

		/**
		 * @brief Default constructor. Each thread will default-construct T on first access.
		 */
		constexpr ThreadLocal()
			requires std::default_initializable<T>
			:
			factory_(
				[]
				{
					return T{};
				})
		{
		}

		/**
		 * @brief Construct with a factory function for lazy per-thread initialization.
		 * @param factory Invoked once per thread on first access to construct T.
		 */
		explicit constexpr ThreadLocal(Factory factory) noexcept :
			factory_(std::move(factory))
		{
		}

		/**
		 * @brief Construct with a prototype value. Each thread gets a copy.
		 * @param prototype The value to copy for each thread's instance.
		 */
		explicit constexpr ThreadLocal(const T& prototype)
			requires std::copy_constructible<T>
			:
			factory_(
				[prototype]
				{
					return prototype;
				})
		{
		}

		~ThreadLocal()
		{
			Storage().erase(this);
		}

		ThreadLocal(const ThreadLocal&)            = delete;
		ThreadLocal& operator=(const ThreadLocal&) = delete;

		ThreadLocal(ThreadLocal&& other) noexcept :
			factory_(std::move(other.factory_))
		{
			// Transfer any existing value from other to this
			auto& map = Storage();
			if (auto it = map.find(&other); it != map.end())
			{
				map.emplace(this, std::move(it->second));
				map.erase(it);
			}
		}

		ThreadLocal& operator=(ThreadLocal&& other) noexcept
		{
			if (this != &other)
			{
				factory_ = std::move(other.factory_);

				// Transfer any existing value from other to this
				auto& map = Storage();
				map.erase(this); // Remove our current value if any
				if (auto it = map.find(&other); it != map.end())
				{
					map.emplace(this, std::move(it->second));
					map.erase(it);
				}
			}
			return *this;
		}

		/**
		 * @brief Access the thread-local value, constructing it on first access.
		 * @return Reference to this thread's instance of T.
		 */
		[[nodiscard]] auto Get(this auto&& self) -> decltype(auto)
		{
			auto& map       = Storage();
			const void* key = std::addressof(self);
			auto it         = map.find(key);

			if (it == map.end()) [[unlikely]]
			{
				it = map.emplace(key, self.factory_()).first;
			}

			return (it->second);
		}

		/**
		 * @brief Check if the current thread has an initialized value.
		 * @return true if this thread has already accessed/constructed its value.
		 */
		[[nodiscard]] bool HasValue() const noexcept
		{
			return Storage().contains(this);
		}

		/**
		 * @brief Reset this thread's value, destroying the current and constructing a new one.
		 */
		void Reset()
		{
			Storage().erase(this);
		}

		/**
		 * @brief Reset this thread's value to a specific new value.
		 * @param value The new value for this thread.
		 */
		void Reset(T value)
		{
			Storage().insert_or_assign(this, std::move(value));
		}

		[[nodiscard]] auto operator->(this auto&& self) noexcept -> decltype(auto)
		{
			return std::addressof(self.Get());
		}

		[[nodiscard]] auto operator*(this auto&& self) noexcept -> decltype(auto)
		{
			return self.Get();
		}

	private:
		/**
		 * @brief Returns the shared thread-local storage for all ThreadLocal<T> instances.
		 * @details All member functions access storage through this single point to ensure
		 *          they operate on the same thread_local map.
		 */
		[[nodiscard]] static auto& Storage() noexcept
		{
			// TODO: Make this a flat_map
			thread_local std::unordered_map<const void*, T> instance;
			return instance;
		}

		mutable Factory factory_;
	};

	/**
	 * @brief Deduction guide for factory functions.
	 */
	template<typename F>
		requires std::invocable<F>
	ThreadLocal(F) -> ThreadLocal<std::invoke_result_t<F>>;
}
