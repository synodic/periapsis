export module synodic.periapsis:structure.automata.storage.static_storage;

import :structure.automata.concepts;

import std;

export namespace peri::automata::storage
{
	/**
	 * @brief Property type for states, tracking initial and final flags with user data
	 *
	 * @tparam StateData User-defined data stored per state, use void for none
	 */
	template<typename StateData>
	struct StateProperty
	{
		[[no_unique_address]] StateData data{};
		bool isInitial = false;
		bool isFinal   = false;

		constexpr bool operator==(const StateProperty&) const = default;
	};

	template<>
	struct StateProperty<void>
	{
		bool isInitial = false;
		bool isFinal   = false;

		constexpr bool operator==(const StateProperty&) const = default;
	};

	/**
	 * @brief Property type for transitions, storing event and optional guard and action indices
	 *
	 * @tparam Event The event type that triggers this transition
	 * @tparam TransitionData Additional user-defined transition data, use void for none
	 */
	template<typename Event, typename TransitionData = void>
	struct TransitionProperty
	{
		Event event{};
		[[no_unique_address]] TransitionData data{};

		constexpr bool operator==(const TransitionProperty&) const = default;
	};

	template<typename Event>
	struct TransitionProperty<Event, void>
	{
		Event event{};

		constexpr bool operator==(const TransitionProperty&) const = default;
	};

	/**
	 * @brief A constexpr-capable state machine storage using fixed-size arrays.
	 *
	 * @tparam Event Type used to trigger transitions
	 * @tparam StateData Type stored per state, use void for none
	 * @tparam TransitionData Type stored per transition beyond the event, use void for none
	 * @tparam MaxStates Maximum number of states
	 * @tparam MaxTransitions Maximum number of transitions
	 * @tparam IndexType Unsigned integer type for indices, default std::size_t
	 */
	template<
		typename Event,
		typename StateData,
		typename TransitionData,
		std::size_t MaxStates,
		std::size_t MaxTransitions,
		std::unsigned_integral IndexType = std::size_t>
	class StaticStorage
	{
	public:
		// ========================================================================
		// Type Definitions
		// ========================================================================

		using EventType              = Event;
		using StateDescriptor        = IndexType;
		using StatesSizeType         = IndexType;
		using TransitionsSizeType    = IndexType;
		using DegreeSizeType         = IndexType;
		using StatePropertyType      = StateProperty<StateData>;
		using TransitionPropertyType = TransitionProperty<Event, TransitionData>;

		// Tagged wrapper to distinguish transition descriptors from state descriptors
		// Both are IndexType but we need separate overloads for operator[]
		struct TransitionDescriptor
		{
			IndexType value;

			constexpr TransitionDescriptor() = default;

			constexpr explicit(false) TransitionDescriptor(IndexType v) :
				value(v)
			{
			}

			constexpr explicit(false) operator IndexType() const noexcept
			{
				return value;
			}

			// Increment/decrement for use with std::views::iota
			constexpr auto& operator++() noexcept
			{
				++value;
				return *this;
			}

			constexpr auto operator++(int) noexcept
			{
				auto tmp = *this;
				++value;
				return tmp;
			}

			constexpr auto& operator--() noexcept
			{
				--value;
				return *this;
			}

			constexpr auto operator--(int) noexcept
			{
				auto tmp = *this;
				--value;
				return tmp;
			}

			// Difference for iota
			constexpr auto operator-(const TransitionDescriptor& other) const noexcept
			{
				return static_cast<std::ptrdiff_t>(value) - static_cast<std::ptrdiff_t>(other.value);
			}

			constexpr bool operator==(const TransitionDescriptor&) const noexcept  = default;
			constexpr auto operator<=>(const TransitionDescriptor&) const noexcept = default;
		};

		// Compile-time properties
		static constexpr bool HAS_STABLE_STATE_DESCRIPTORS      = true;
		static constexpr bool HAS_STABLE_TRANSITION_DESCRIPTORS = true;

		static constexpr StatesSizeType MAX_STATES           = static_cast<StatesSizeType>(MaxStates);
		static constexpr TransitionsSizeType MAX_TRANSITIONS = static_cast<TransitionsSizeType>(MaxTransitions);

		// Rebind to different property types
		template<typename E, typename SD, typename TD>
		using Rebind = StaticStorage<E, SD, TD, MaxStates, MaxTransitions, IndexType>;

	private:
		// CSR-style storage for O(1) state lookup and cache-friendly transition iteration
		std::array<TransitionsSizeType, MaxStates + 1> rowOffsets_{};
		std::array<StateDescriptor, MaxTransitions> targets_{};

		// Property storage
		std::array<StatePropertyType, MaxStates> stateProperties_{};
		std::array<TransitionPropertyType, MaxTransitions> transitionProperties_{};

		StatesSizeType numStates_{0};
		TransitionsSizeType numTransitions_{0};

		// Initial state
		std::optional<StateDescriptor> initialState_{};

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		constexpr StaticStorage() = default;

		// ========================================================================
		// State Operations
		// ========================================================================

		[[nodiscard]] constexpr auto NumStates() const noexcept -> StatesSizeType
		{
			return numStates_;
		}

		[[nodiscard]] constexpr auto States() const noexcept
		{
			return std::views::iota(StateDescriptor{0}, numStates_);
		}

		constexpr auto AddState() -> StateDescriptor
		{
			if (numStates_ >= MAX_STATES)
			{
				// In constexpr context, this will cause a compile error
			}
			auto s             = numStates_++;
			rowOffsets_[s + 1] = rowOffsets_[s]; // Initialize with no transitions
			return s;
		}

		template<typename... Args>
			requires std::constructible_from<StateData, Args...>
		constexpr auto AddState(Args&&... args) -> StateDescriptor
		{
			auto s                   = AddState();
			stateProperties_[s].data = StateData(std::forward<Args>(args)...);
			return s;
		}

		// ========================================================================
		// Transition Operations
		// ========================================================================

		constexpr auto AddTransition(
			StateDescriptor source,
			StateDescriptor target,
			const Event& event)
			-> std::pair<TransitionDescriptor, bool>
		{
			if (numTransitions_ >= MAX_TRANSITIONS)
			{
				return {TransitionDescriptor{}, false};
			}

			// In CSR format, we need to insert at the position where this source's transitions end
			auto insertPos = rowOffsets_[source + 1];

			// Shift all transitions after insertPos to make room
			for (auto i = numTransitions_; i > insertPos; --i)
			{
				targets_[i]              = targets_[i - 1];
				transitionProperties_[i] = transitionProperties_[i - 1];
			}

			// Insert the new transition
			targets_[insertPos]                    = target;
			transitionProperties_[insertPos].event = event;
			++numTransitions_;

			// Update row offsets for all states after source
			for (auto s = source + 1; s <= numStates_; ++s)
			{
				++rowOffsets_[s];
			}

			return {TransitionDescriptor{insertPos}, true};
		}

		template<typename... Args>
			requires std::constructible_from<TransitionData, Args...>
		constexpr auto
			AddTransition(
				StateDescriptor source,
				StateDescriptor target,
				const Event& event,
				Args&&... args)
				-> std::pair<TransitionDescriptor, bool>
		{
			auto [trans, inserted] = AddTransition(source, target, event);
			if (inserted)
			{
				transitionProperties_[trans.value].data = TransitionData(std::forward<Args>(args)...);
			}
			return {trans, inserted};
		}

		[[nodiscard]] constexpr auto NumTransitions() const noexcept -> TransitionsSizeType
		{
			return numTransitions_;
		}

		[[nodiscard]] constexpr auto Transitions() const noexcept
		{
			return std::views::iota(TransitionDescriptor{0}, TransitionDescriptor{numTransitions_});
		}

		// ========================================================================
		// Incidence: Out-Transition Traversal
		// ========================================================================

		[[nodiscard]] constexpr auto OutDegree(StateDescriptor s) const noexcept -> DegreeSizeType
		{
			return rowOffsets_[s + 1] - rowOffsets_[s];
		}

		[[nodiscard]] constexpr auto OutTransitions(StateDescriptor s) const noexcept
		{
			return std::views::iota(
				TransitionDescriptor{rowOffsets_[s]},
				TransitionDescriptor{rowOffsets_[s + 1]});
		}

		[[nodiscard]] constexpr auto Source(TransitionDescriptor t) const noexcept -> StateDescriptor
		{
			// Binary search to find which state owns this transition
			auto it = std::ranges::upper_bound(rowOffsets_ | std::views::take(numStates_ + 1), t.value);
			return static_cast<StateDescriptor>(std::distance(rowOffsets_.begin(), it) - 1);
		}

		[[nodiscard]] constexpr auto Target(TransitionDescriptor t) const noexcept -> StateDescriptor
		{
			return targets_[t.value];
		}

		// ========================================================================
		// Event-Based Transition Lookup
		// ========================================================================

		[[nodiscard]] constexpr auto FindTransition(
			StateDescriptor s,
			const Event& event) const noexcept
			-> std::optional<TransitionDescriptor>
		{
			for (IndexType t = rowOffsets_[s]; t < rowOffsets_[s + 1]; ++t)
			{
				if (transitionProperties_[t].event == event)
				{
					return TransitionDescriptor{t};
				}
			}
			return std::nullopt;
		}

		// ========================================================================
		// Property Access
		// ========================================================================

		[[nodiscard]] constexpr auto operator[](StateDescriptor s) -> StatePropertyType&
		{
			return stateProperties_[s];
		}

		[[nodiscard]] constexpr auto operator[](StateDescriptor s) const -> const StatePropertyType&
		{
			return stateProperties_[s];
		}

		[[nodiscard]] constexpr auto operator[](TransitionDescriptor t) -> TransitionPropertyType&
		{
			return transitionProperties_[t.value];
		}

		[[nodiscard]] constexpr auto operator[](TransitionDescriptor t) const -> const TransitionPropertyType&
		{
			return transitionProperties_[t.value];
		}

		// ========================================================================
		// Initial and Final State Management
		// ========================================================================

		constexpr void SetInitialState(StateDescriptor s)
		{
			// Clear previous initial state
			if (initialState_)
			{
				stateProperties_[*initialState_].isInitial = false;
			}
			initialState_                 = s;
			stateProperties_[s].isInitial = true;
		}

		[[nodiscard]] constexpr auto InitialState() const noexcept -> std::optional<StateDescriptor>
		{
			return initialState_;
		}

		constexpr void SetFinalState(StateDescriptor s, bool isFinal = true)
		{
			stateProperties_[s].isFinal = isFinal;
		}

		[[nodiscard]] constexpr auto IsFinalState(StateDescriptor s) const noexcept -> bool
		{
			return stateProperties_[s].isFinal;
		}

		[[nodiscard]] constexpr auto FinalStates() const noexcept
		{
			return States() | std::views::filter(
				       [this](StateDescriptor s)
				       {
					       return stateProperties_[s].isFinal;
				       });
		}

		// ========================================================================
		// State Data Access
		// ========================================================================

		template<typename S = StateData>
			requires(not std::same_as<S, void>)
		[[nodiscard]] constexpr auto GetStateData(StateDescriptor s) -> S&
		{
			return stateProperties_[s].data;
		}

		template<typename S = StateData>
			requires(not std::same_as<S, void>)
		[[nodiscard]] constexpr auto GetStateData(StateDescriptor s) const -> const S&
		{
			return stateProperties_[s].data;
		}

		[[nodiscard]] constexpr auto TransitionEvent(TransitionDescriptor t) const noexcept -> const Event&
		{
			return transitionProperties_[t.value].event;
		}

		template<typename T = TransitionData>
			requires(not std::same_as<T, void>)
		[[nodiscard]] constexpr auto GetTransitionData(TransitionDescriptor t) -> T&
		{
			return transitionProperties_[t.value].data;
		}

		template<typename T = TransitionData>
			requires(not std::same_as<T, void>)
		[[nodiscard]] constexpr auto GetTransitionData(TransitionDescriptor t) const -> const T&
		{
			return transitionProperties_[t.value].data;
		}
	};

	// ============================================================================
	// Convenience Aliases
	// ============================================================================

	/**
	 * @brief Static storage with integral events and no additional data
	 */
	template<std::size_t MaxStates, std::size_t MaxTransitions, std::unsigned_integral IndexType = std::size_t>
	using StaticTopology = StaticStorage<int, void, void, MaxStates, MaxTransitions, IndexType>;

	/**
	 * @brief Static storage with typed events and state data
	 */
	template<
		typename Event,
		typename StateData,
		std::size_t MaxStates,
		std::size_t MaxTransitions,
		std::unsigned_integral IndexType = std::size_t>
	using StaticStateStorage = StaticStorage<Event, StateData, void, MaxStates, MaxTransitions, IndexType>;

	/**
	 * @brief Static storage with typed events and transition data
	 */
	template<
		typename Event,
		typename TransitionData,
		std::size_t MaxStates,
		std::size_t MaxTransitions,
		std::unsigned_integral IndexType = std::size_t>
	using StaticTransitionStorage = StaticStorage<Event, void, TransitionData, MaxStates, MaxTransitions, IndexType>;
}
