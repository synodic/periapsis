export module synodic.periapsis:structure.automata.state_machine;

import :structure.automata.concepts;
import :structure.automata.storage.static_storage;
import :structure.automata.storage.dynamic_storage;

import std;

export namespace peri::automata
{
	// ============================================================================
	// Storage Selection Helper
	// ============================================================================

	namespace detail
	{
		/**
		 * @brief Sentinel value indicating dynamic storage
		 */
		inline constexpr std::size_t DYNAMIC_EXTENT = std::numeric_limits<std::size_t>::max();

		/**
		 * @brief Select storage type based on size parameters
		 */
		template<
			typename Event,
			typename StateData,
			typename TransitionData,
			std::size_t MaxStates,
			std::size_t MaxTransitions>
		struct StorageSelector
		{
			using Type = std::conditional_t<
				MaxStates == DYNAMIC_EXTENT || MaxTransitions == DYNAMIC_EXTENT,
				storage::DynamicStorage<Event, StateData, TransitionData>,
				storage::StaticStorage<Event, StateData, TransitionData, MaxStates, MaxTransitions>>;
		};

		template<
			typename Event,
			typename StateData,
			typename TransitionData,
			std::size_t MaxStates,
			std::size_t MaxTransitions>
		using StorageSelectorT =
		StorageSelector<Event, StateData, TransitionData, MaxStates, MaxTransitions>::Type;
	}

	// ============================================================================
	// StateMachine Class
	// ============================================================================

	/**
	 * @brief A state machine data structure with automatic storage selection.
	 *
	 *        This class represents the STRUCTURE of a state machine only - states,
	 *        transitions, and their topology. Execution state is managed separately by a Runner.
	 *
	 * @tparam Event Type used to trigger transitions
	 * @tparam StateData Type stored per state (void for none)
	 * @tparam TransitionData Type stored per transition (void for none)
	 * @tparam MaxStates Maximum states (dynamic_extent for growable)
	 * @tparam MaxTransitions Maximum transitions (dynamic_extent for growable)
	 */
	template<
		typename Event,
		typename StateData = void,
		typename TransitionData = void,
		std::size_t MaxStates = detail::DYNAMIC_EXTENT,
		std::size_t MaxTransitions = detail::DYNAMIC_EXTENT>
	class StateMachine
	{
	public:
		// ========================================================================
		// Type Definitions
		// ========================================================================

		using StorageType = detail::StorageSelectorT<Event, StateData, TransitionData, MaxStates, MaxTransitions>;
		using EventType = Event;
		using StateDescriptor = StorageType::StateDescriptor;
		using TransitionDescriptor = StorageType::TransitionDescriptor;
		using StatesSizeType = StorageType::StatesSizeType;
		using TransitionsSizeType = StorageType::TransitionsSizeType;
		using DegreeSizeType = StorageType::DegreeSizeType;
		using StateDataType = StateData;
		using TransitionDataType = TransitionData;

		// Compile-time properties

		static constexpr bool IS_DYNAMIC = MaxStates == detail::DYNAMIC_EXTENT;

		static constexpr std::size_t MAX_STATES      = MaxStates;
		static constexpr std::size_t MAX_TRANSITIONS = MaxTransitions;

	private:
		StorageType storage_;

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		constexpr StateMachine() = default;

		/**
		 * @brief Construct with explicit storage
		 */
		constexpr explicit StateMachine(StorageType storage) :
			storage_(std::move(storage))
		{
		}

		/**
		 * @brief Seed from a fixed-size state machine
		 */
		template<std::size_t OtherMaxS, std::size_t OtherMaxT>
			requires IS_DYNAMIC and (OtherMaxS != detail::DYNAMIC_EXTENT)
		explicit(false) StateMachine(const StateMachine<Event, StateData, TransitionData, OtherMaxS, OtherMaxT>& source)
		{
			// Copy all states
			for (auto s: source.States())
			{
				if constexpr (not std::same_as<StateData, void>)
				{
					AddState(source[s]);
				}
				else
				{
					AddState();
				}
			}

			// Copy initial and final state markers
			if (auto init = source.InitialState())
			{
				SetInitialState(*init);
			}
			for (auto s: source.States())
			{
				if (source.IsFinalState(s))
				{
					SetFinalState(s, true);
				}
			}

			// Copy all transitions
			for (auto t: source.Transitions())
			{
				auto src = source.Source(t);
				auto tgt = source.Target(t);
				auto evt = source.TransitionEvent(t);
				if constexpr (not std::same_as<TransitionData, void>)
				{
					AddTransition(src, tgt, evt, source[t]);
				}
				else
				{
					AddTransition(src, tgt, evt);
				}
			}
		}

		// ========================================================================
		// State Operations
		// ========================================================================

		[[nodiscard]] constexpr auto NumStates() const noexcept -> StatesSizeType
		{
			return storage_.NumStates();
		}

		[[nodiscard]] constexpr auto States() const noexcept
		{
			return storage_.States();
		}

		constexpr auto AddState() -> StateDescriptor
		{
			return storage_.AddState();
		}

		template<typename... Args>
			requires(not std::same_as<StateData, void>) and std::constructible_from<StateData, Args...>
		constexpr auto AddState(Args&&... args) -> StateDescriptor
		{
			return storage_.AddState(std::forward<Args>(args)...);
		}

		constexpr void RemoveState(StateDescriptor s)
			requires mutable_state_storage<StorageType>
		{
			storage_.RemoveState(s);
		}

		constexpr void ClearStates()
			requires mutable_state_storage<StorageType>
		{
			storage_.ClearStates();
		}

		// ========================================================================
		// Transition Operations
		// ========================================================================

		[[nodiscard]] constexpr auto NumTransitions() const noexcept -> TransitionsSizeType
		{
			return storage_.NumTransitions();
		}

		[[nodiscard]] constexpr auto Transitions() const noexcept
		{
			return storage_.Transitions();
		}

		constexpr auto AddTransition(
			StateDescriptor from,
			StateDescriptor to,
			const EventType& event)
			-> std::pair<TransitionDescriptor, bool>
		{
			return storage_.AddTransition(from, to, event);
		}

		template<typename... Args>
			requires(not std::same_as<TransitionData, void>) and std::constructible_from<TransitionData, Args...>
		constexpr auto
			AddTransition(
				StateDescriptor from,
				StateDescriptor to,
				const EventType& event,
				Args&&... args)
				-> std::pair<TransitionDescriptor, bool>
		{
			return storage_.AddTransition(from, to, event, std::forward<Args>(args)...);
		}

		constexpr void RemoveTransition(TransitionDescriptor t)
			requires mutable_transition_storage<StorageType>
		{
			storage_.RemoveTransition(t);
		}

		constexpr void ClearTransitions()
			requires mutable_transition_storage<StorageType>
		{
			storage_.ClearTransitions();
		}

		// ========================================================================
		// Incidence Queries
		// ========================================================================

		[[nodiscard]] constexpr auto OutTransitions(StateDescriptor s) const noexcept
		{
			return storage_.OutTransitions(s);
		}

		[[nodiscard]] constexpr auto OutDegree(StateDescriptor s) const noexcept -> DegreeSizeType
		{
			return storage_.OutDegree(s);
		}

		[[nodiscard]] constexpr auto Source(TransitionDescriptor t) const noexcept -> StateDescriptor
		{
			return storage_.Source(t);
		}

		[[nodiscard]] constexpr auto Target(TransitionDescriptor t) const noexcept -> StateDescriptor
		{
			return storage_.Target(t);
		}

		[[nodiscard]] constexpr auto TransitionEvent(TransitionDescriptor t) const noexcept -> const EventType&
		{
			return storage_.TransitionEvent(t);
		}

		// ========================================================================
		// Event Queries
		// ========================================================================

		[[nodiscard]] constexpr auto FindTransition(
			StateDescriptor from,
			const EventType& event) const noexcept
			-> std::optional<TransitionDescriptor>
		{
			return storage_.FindTransition(from, event);
		}

		// ========================================================================
		// Initial/Final State Management
		// ========================================================================

		constexpr void SetInitialState(StateDescriptor s)
		{
			storage_.SetInitialState(s);
		}

		[[nodiscard]] constexpr auto InitialState() const noexcept -> std::optional<StateDescriptor>
		{
			return storage_.InitialState();
		}

		constexpr void SetFinalState(StateDescriptor s, bool isFinal = true)
		{
			storage_.SetFinalState(s, isFinal);
		}

		[[nodiscard]] constexpr auto IsFinalState(StateDescriptor s) const noexcept -> bool
		{
			return storage_.IsFinalState(s);
		}

		[[nodiscard]] constexpr auto FinalStates() const noexcept
		{
			return storage_.FinalStates();
		}

		// ========================================================================
		// Property Access
		// ========================================================================

		[[nodiscard]] constexpr decltype(auto) operator[](StateDescriptor s)
			requires(not std::same_as<StateData, void>)
		{
			return storage_[s];
		}

		[[nodiscard]] constexpr decltype(auto) operator[](StateDescriptor s) const
			requires(not std::same_as<StateData, void>)
		{
			return storage_[s];
		}

		[[nodiscard]] constexpr decltype(auto) operator[](TransitionDescriptor t)
			requires(not std::same_as<TransitionData, void>)
		{
			return storage_[t];
		}

		[[nodiscard]] constexpr decltype(auto) operator[](TransitionDescriptor t) const
			requires(not std::same_as<TransitionData, void>)
		{
			return storage_[t];
		}

		// ========================================================================
		// Direct Storage Access
		// ========================================================================

		[[nodiscard]] constexpr auto Storage() noexcept -> StorageType&
		{
			return storage_;
		}

		[[nodiscard]] constexpr auto Storage() const noexcept -> const StorageType&
		{
			return storage_;
		}
	};

	// ============================================================================
	// Convenience Constants
	// ============================================================================

	/**
	 * @brief Sentinel for dynamic state machines
	 */
	inline constexpr std::size_t DYNAMIC_EXTENT = detail::DYNAMIC_EXTENT;

	// ============================================================================
	// Convenience Type Aliases
	// ============================================================================

	/**
	 * @brief Dynamic state machine
	 */
	template<typename Event, typename StateData = void, typename TransitionData = void>
	using DynamicStateMachine = StateMachine<Event, StateData, TransitionData>;

	/**
	 * @brief Fixed-size constexpr-capable state machine
	 */
	template<typename Event, typename StateData, typename TransitionData, std::size_t MaxS, std::size_t MaxT>
	using FixedStateMachine = StateMachine<Event, StateData, TransitionData, MaxS, MaxT>;

	/**
	 * @brief Simple state machine with integer events and no data
	 */
	template<std::size_t MaxStates, std::size_t MaxTransitions>
	using SimpleStaticStateMachine = StateMachine<int, void, void, MaxStates, MaxTransitions>;

	/**
	 * @brief Simple dynamic state machine with integer events and no data
	 */
	using SimpleDynamicStateMachine = StateMachine<int, void, void>;
}
