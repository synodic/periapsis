export module synodic.library:structure.state_machine.state_machine;

import :structure.state_machine.concepts;
import :structure.state_machine.storage.static_storage;
import :structure.state_machine.storage.dynamic_storage;

import std;

export namespace synodic::state_machine
{
	// ============================================================================
	// Storage Selection Helper
	// ============================================================================

	namespace detail
	{
		/// Sentinel value indicating dynamic/growable storage
		inline constexpr std::size_t dynamic_extent = std::numeric_limits<std::size_t>::max();

		/// Select storage type based on size parameters
		template<
			typename Event,
			typename StateData,
			typename TransitionData,
			std::size_t MaxStates,
			std::size_t MaxTransitions>
		struct storage_selector
		{
			using type = std::conditional_t<
				(MaxStates == dynamic_extent || MaxTransitions == dynamic_extent),
				storage::DynamicStorage<Event, StateData, TransitionData>,
				storage::StaticStorage<Event, StateData, TransitionData, MaxStates, MaxTransitions>>;
		};

		template<
			typename Event,
			typename StateData,
			typename TransitionData,
			std::size_t MaxStates,
			std::size_t MaxTransitions>
		using storage_selector_t =
			typename storage_selector<Event, StateData, TransitionData, MaxStates, MaxTransitions>::type;
	}

	// ============================================================================
	// StateMachine Class (Structure Only)
	// ============================================================================

	/**
	 * @brief A state machine data structure with automatic storage selection.
	 *
	 *        This class represents the STRUCTURE of a state machine only - states,
	 *        transitions, and their topology. Execution state (current position)
	 *        is managed separately by a Runner.
	 *
	 *        - Fixed size (MaxStates, MaxTransitions specified): Uses constexpr-capable storage
	 *        - Dynamic size (defaults): Uses growable storage
	 *
	 *        A dynamic state machine can be seeded from a constexpr one via constructor.
	 *
	 * @tparam Event Type used to trigger transitions
	 * @tparam StateData Type stored per state (void for none)
	 * @tparam TransitionData Type stored per transition (void for none)
	 * @tparam MaxStates Maximum states (dynamic_extent for growable)
	 * @tparam MaxTransitions Maximum transitions (dynamic_extent for growable)
	 */
	template<
		typename Event,
		typename StateData		   = void,
		typename TransitionData	   = void,
		std::size_t MaxStates	   = detail::dynamic_extent,
		std::size_t MaxTransitions = detail::dynamic_extent>
	class StateMachine
	{
	public:
		// ========================================================================
		// Type Definitions
		// ========================================================================

		using storage_type = detail::storage_selector_t<Event, StateData, TransitionData, MaxStates, MaxTransitions>;
		using event_type   = Event;
		using state_descriptor		= typename storage_type::state_descriptor;
		using transition_descriptor = typename storage_type::transition_descriptor;
		using states_size_type		= typename storage_type::states_size_type;
		using transitions_size_type = typename storage_type::transitions_size_type;
		using degree_size_type		= typename storage_type::degree_size_type;
		using state_data_type		= StateData;
		using transition_data_type	= TransitionData;

		// Compile-time properties
		static constexpr bool is_constexpr_capable = storage_type::is_constexpr_capable;
		static constexpr bool is_dynamic		   = (MaxStates == detail::dynamic_extent);

		static constexpr std::size_t max_states		 = MaxStates;
		static constexpr std::size_t max_transitions = MaxTransitions;

	private:
		storage_type storage_;

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		constexpr StateMachine() = default;

		/// Construct with explicit storage
		constexpr explicit StateMachine(storage_type storage) :
			storage_(std::move(storage))
		{
		}

		/// Seed from a fixed-size state machine (enables constexpr -> dynamic extension)
		template<std::size_t OtherMaxS, std::size_t OtherMaxT>
			requires(is_dynamic) and (OtherMaxS != detail::dynamic_extent)
		StateMachine(const StateMachine<Event, StateData, TransitionData, OtherMaxS, OtherMaxT>& source)
		{
			// Copy all states
			for (auto s: source.states())
			{
				if constexpr (not std::same_as<StateData, void>)
				{
					add_state(source[s]);
				}
				else
				{
					add_state();
				}
			}

			// Copy initial/final state markers
			if (auto init = source.initial_state())
			{
				set_initial_state(*init);
			}
			for (auto s: source.states())
			{
				if (source.is_final_state(s))
				{
					set_final_state(s, true);
				}
			}

			// Copy all transitions
			for (auto t: source.transitions())
			{
				auto src = source.source(t);
				auto tgt = source.target(t);
				auto evt = source.transition_event(t);
				if constexpr (not std::same_as<TransitionData, void>)
				{
					add_transition(src, tgt, evt, source[t]);
				}
				else
				{
					add_transition(src, tgt, evt);
				}
			}
		}

		// ========================================================================
		// State Operations
		// ========================================================================

		[[nodiscard]] constexpr auto num_states() const noexcept -> states_size_type
		{
			return storage_.num_states();
		}

		[[nodiscard]] constexpr auto states() const noexcept
		{
			return storage_.states();
		}

		constexpr auto add_state() -> state_descriptor
		{
			return storage_.add_state();
		}

		template<typename... Args>
			requires(not std::same_as<StateData, void>) and std::constructible_from<StateData, Args...>
		constexpr auto add_state(Args&&... args) -> state_descriptor
		{
			return storage_.add_state(std::forward<Args>(args)...);
		}

		constexpr void remove_state(state_descriptor s)
			requires mutable_state_storage<storage_type>
		{
			storage_.remove_state(s);
		}

		constexpr void clear_states()
			requires mutable_state_storage<storage_type>
		{
			storage_.clear_states();
		}

		// ========================================================================
		// Transition Operations
		// ========================================================================

		[[nodiscard]] constexpr auto num_transitions() const noexcept -> transitions_size_type
		{
			return storage_.num_transitions();
		}

		[[nodiscard]] constexpr auto transitions() const noexcept
		{
			return storage_.transitions();
		}

		constexpr auto add_transition(state_descriptor from, state_descriptor to, const event_type& event)
			-> std::pair<transition_descriptor, bool>
		{
			return storage_.add_transition(from, to, event);
		}

		template<typename... Args>
			requires(not std::same_as<TransitionData, void>) and std::constructible_from<TransitionData, Args...>
		constexpr auto
			add_transition(state_descriptor from, state_descriptor to, const event_type& event, Args&&... args)
				-> std::pair<transition_descriptor, bool>
		{
			return storage_.add_transition(from, to, event, std::forward<Args>(args)...);
		}

		constexpr void remove_transition(transition_descriptor t)
			requires mutable_transition_storage<storage_type>
		{
			storage_.remove_transition(t);
		}

		constexpr void clear_transitions()
			requires mutable_transition_storage<storage_type>
		{
			storage_.clear_transitions();
		}

		// ========================================================================
		// Incidence Queries
		// ========================================================================

		[[nodiscard]] constexpr auto out_transitions(state_descriptor s) const noexcept
		{
			return storage_.out_transitions(s);
		}

		[[nodiscard]] constexpr auto out_degree(state_descriptor s) const noexcept -> degree_size_type
		{
			return storage_.out_degree(s);
		}

		[[nodiscard]] constexpr auto source(transition_descriptor t) const noexcept -> state_descriptor
		{
			return storage_.source(t);
		}

		[[nodiscard]] constexpr auto target(transition_descriptor t) const noexcept -> state_descriptor
		{
			return storage_.target(t);
		}

		[[nodiscard]] constexpr auto transition_event(transition_descriptor t) const noexcept -> const event_type&
		{
			return storage_.transition_event(t);
		}

		// ========================================================================
		// Event Queries
		// ========================================================================

		[[nodiscard]] constexpr auto find_transition(state_descriptor from, const event_type& event) const noexcept
			-> std::optional<transition_descriptor>
		{
			return storage_.find_transition(from, event);
		}

		// ========================================================================
		// Initial/Final State Management
		// ========================================================================

		constexpr void set_initial_state(state_descriptor s)
		{
			storage_.set_initial_state(s);
		}

		[[nodiscard]] constexpr auto initial_state() const noexcept -> std::optional<state_descriptor>
		{
			return storage_.initial_state();
		}

		constexpr void set_final_state(state_descriptor s, bool is_final = true)
		{
			storage_.set_final_state(s, is_final);
		}

		[[nodiscard]] constexpr auto is_final_state(state_descriptor s) const noexcept -> bool
		{
			return storage_.is_final_state(s);
		}

		[[nodiscard]] constexpr auto final_states() const noexcept
		{
			return storage_.final_states();
		}

		// ========================================================================
		// Property Access
		// ========================================================================

		[[nodiscard]] constexpr decltype(auto) operator[](state_descriptor s)
			requires(not std::same_as<StateData, void>)
		{
			return storage_[s];
		}

		[[nodiscard]] constexpr decltype(auto) operator[](state_descriptor s) const
			requires(not std::same_as<StateData, void>)
		{
			return storage_[s];
		}

		[[nodiscard]] constexpr decltype(auto) operator[](transition_descriptor t)
			requires(not std::same_as<TransitionData, void>)
		{
			return storage_[t];
		}

		[[nodiscard]] constexpr decltype(auto) operator[](transition_descriptor t) const
			requires(not std::same_as<TransitionData, void>)
		{
			return storage_[t];
		}

		// ========================================================================
		// Advanced: Direct Storage Access (for power users)
		// ========================================================================

		[[nodiscard]] constexpr auto storage() noexcept -> storage_type&
		{
			return storage_;
		}

		[[nodiscard]] constexpr auto storage() const noexcept -> const storage_type&
		{
			return storage_;
		}
	};

	// ============================================================================
	// Convenience Constants
	// ============================================================================

	/// Sentinel for dynamic/growable state machines
	inline constexpr std::size_t dynamic_extent = detail::dynamic_extent;

	// ============================================================================
	// Convenience Type Aliases
	// ============================================================================

	/// Dynamic state machine (default) - growable at runtime
	template<typename Event, typename StateData = void, typename TransitionData = void>
	using DynamicStateMachine = StateMachine<Event, StateData, TransitionData>;

	/// Fixed-size constexpr-capable state machine
	template<typename Event, typename StateData, typename TransitionData, std::size_t MaxS, std::size_t MaxT>
	using FixedStateMachine = StateMachine<Event, StateData, TransitionData, MaxS, MaxT>;

	/// Simple state machine with integer events and no data
	template<std::size_t MaxStates, std::size_t MaxTransitions>
	using SimpleStaticStateMachine = StateMachine<int, void, void, MaxStates, MaxTransitions>;

	/// Simple dynamic state machine with integer events and no data
	using SimpleDynamicStateMachine = StateMachine<int, void, void>;
}
