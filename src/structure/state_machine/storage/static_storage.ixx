export module synodic.library:structure.state_machine.storage.static_storage;

import :structure.state_machine.concepts;

import std;

export namespace synodic::state_machine::storage
{
	/**
	 * @brief Property type for states, tracking initial/final flags and user data
	 *
	 * @tparam StateData User-defined data stored per state (use void for none)
	 */
	template<typename StateData>
	struct StateProperty
	{
		[[no_unique_address]] StateData data {};
		bool is_initial = false;
		bool is_final	= false;

		constexpr bool operator==(const StateProperty&) const = default;
	};

	template<>
	struct StateProperty<void>
	{
		bool is_initial = false;
		bool is_final	= false;

		constexpr bool operator==(const StateProperty&) const = default;
	};

	/**
	 * @brief Property type for transitions, storing event and optional guard/action indices
	 *
	 * @tparam Event The event type that triggers this transition
	 * @tparam TransitionData Additional user-defined transition data (use void for none)
	 */
	template<typename Event, typename TransitionData = void>
	struct TransitionProperty
	{
		Event event {};
		[[no_unique_address]] TransitionData data {};

		constexpr bool operator==(const TransitionProperty&) const = default;
	};

	template<typename Event>
	struct TransitionProperty<Event, void>
	{
		Event event {};

		constexpr bool operator==(const TransitionProperty&) const = default;
	};

	/**
	 * @brief A constexpr-capable state machine storage using fixed-size arrays.
	 *        Ideal for compile-time state machine construction where topology is known at compile-time.
	 *        Uses adjacency list style layout for efficient transition lookup.
	 *
	 * @tparam Event Type used to trigger transitions
	 * @tparam StateData Type stored per state (use void for none)
	 * @tparam TransitionData Type stored per transition beyond the event (use void for none)
	 * @tparam MaxStates Maximum number of states
	 * @tparam MaxTransitions Maximum number of transitions
	 * @tparam IndexType Unsigned integer type for indices (default: std::size_t)
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

		using event_type			   = Event;
		using state_descriptor		   = IndexType;
		using states_size_type		   = IndexType;
		using transitions_size_type	   = IndexType;
		using degree_size_type		   = IndexType;
		using state_property_type	   = StateProperty<StateData>;
		using transition_property_type = TransitionProperty<Event, TransitionData>;

		// Tagged wrapper to distinguish transition descriptors from state descriptors
		// (both are IndexType, but we need separate overloads for operator[])
		struct transition_descriptor
		{
			IndexType value;

			constexpr transition_descriptor() = default;

			constexpr explicit(false) transition_descriptor(IndexType v) :
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
			constexpr auto operator-(const transition_descriptor& other) const noexcept
			{
				return static_cast<std::ptrdiff_t>(value) - static_cast<std::ptrdiff_t>(other.value);
			}

			constexpr bool operator==(const transition_descriptor&) const noexcept	= default;
			constexpr auto operator<=>(const transition_descriptor&) const noexcept = default;
		};

		// Compile-time properties
		static constexpr bool is_constexpr_capable				= true;
		static constexpr bool has_stable_state_descriptors		= true;
		static constexpr bool has_stable_transition_descriptors = true;

		static constexpr states_size_type max_states		   = static_cast<states_size_type>(MaxStates);
		static constexpr transitions_size_type max_transitions = static_cast<transitions_size_type>(MaxTransitions);

		// Rebind to different property types
		template<typename E, typename SD, typename TD>
		using rebind = StaticStorage<E, SD, TD, MaxStates, MaxTransitions, IndexType>;

	private:
		// CSR-style storage for O(1) state lookup and cache-friendly transition iteration
		std::array<transitions_size_type, MaxStates + 1> row_offsets_ {};
		std::array<state_descriptor, MaxTransitions> targets_ {};

		// Property storage
		std::array<state_property_type, MaxStates> state_properties_ {};
		std::array<transition_property_type, MaxTransitions> transition_properties_ {};

		states_size_type num_states_ {0};
		transitions_size_type num_transitions_ {0};

		// Initial state (optional)
		std::optional<state_descriptor> initial_state_ {};

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		constexpr StaticStorage() = default;

		// ========================================================================
		// State Operations
		// ========================================================================

		[[nodiscard]] constexpr auto num_states() const noexcept -> states_size_type
		{
			return num_states_;
		}

		[[nodiscard]] constexpr auto states() const noexcept
		{
			return std::views::iota(state_descriptor {0}, num_states_);
		}

		constexpr auto add_state() -> state_descriptor
		{
			if (num_states_ >= max_states)
			{
				// In constexpr context, this will cause a compile error
			}
			auto s				= num_states_++;
			row_offsets_[s + 1] = row_offsets_[s];	// Initialize with no transitions
			return s;
		}

		template<typename... Args>
			requires std::constructible_from<StateData, Args...>
		constexpr auto add_state(Args&&... args) -> state_descriptor
		{
			auto s					  = add_state();
			state_properties_[s].data = StateData(std::forward<Args>(args)...);
			return s;
		}

		// ========================================================================
		// Transition Operations
		// ========================================================================

		constexpr auto add_transition(state_descriptor source, state_descriptor target, const Event& event)
			-> std::pair<transition_descriptor, bool>
		{
			if (num_transitions_ >= max_transitions)
			{
				return {transition_descriptor {}, false};
			}

			// In CSR format, we need to insert at the position where this source's transitions end
			auto insert_pos = row_offsets_[source + 1];

			// Shift all transitions after insert_pos to make room
			for (auto i = num_transitions_; i > insert_pos; --i)
			{
				targets_[i]				  = targets_[i - 1];
				transition_properties_[i] = transition_properties_[i - 1];
			}

			// Insert the new transition
			targets_[insert_pos]					 = target;
			transition_properties_[insert_pos].event = event;
			++num_transitions_;

			// Update row offsets for all states after source
			for (auto s = source + 1; s <= num_states_; ++s)
			{
				++row_offsets_[s];
			}

			return {transition_descriptor {insert_pos}, true};
		}

		template<typename... Args>
			requires std::constructible_from<TransitionData, Args...>
		constexpr auto
			add_transition(state_descriptor source, state_descriptor target, const Event& event, Args&&... args)
				-> std::pair<transition_descriptor, bool>
		{
			auto [trans, inserted] = add_transition(source, target, event);
			if (inserted)
			{
				transition_properties_[trans.value].data = TransitionData(std::forward<Args>(args)...);
			}
			return {trans, inserted};
		}

		[[nodiscard]] constexpr auto num_transitions() const noexcept -> transitions_size_type
		{
			return num_transitions_;
		}

		[[nodiscard]] constexpr auto transitions() const noexcept
		{
			return std::views::iota(transition_descriptor {0}, transition_descriptor {num_transitions_});
		}

		// ========================================================================
		// Incidence (Out-Transition Traversal)
		// ========================================================================

		[[nodiscard]] constexpr auto out_degree(state_descriptor s) const noexcept -> degree_size_type
		{
			return row_offsets_[s + 1] - row_offsets_[s];
		}

		[[nodiscard]] constexpr auto out_transitions(state_descriptor s) const noexcept
		{
			return std::views::iota(
				transition_descriptor {row_offsets_[s]},
				transition_descriptor {row_offsets_[s + 1]});
		}

		[[nodiscard]] constexpr auto source(transition_descriptor t) const noexcept -> state_descriptor
		{
			// Binary search to find which state owns this transition
			auto it = std::ranges::upper_bound(row_offsets_ | std::views::take(num_states_ + 1), t.value);
			return static_cast<state_descriptor>(std::distance(row_offsets_.begin(), it) - 1);
		}

		[[nodiscard]] constexpr auto target(transition_descriptor t) const noexcept -> state_descriptor
		{
			return targets_[t.value];
		}

		// ========================================================================
		// Event-Based Transition Lookup
		// ========================================================================

		[[nodiscard]] constexpr auto find_transition(state_descriptor s, const Event& event) const noexcept
			-> std::optional<transition_descriptor>
		{
			for (IndexType t = row_offsets_[s]; t < row_offsets_[s + 1]; ++t)
			{
				if (transition_properties_[t].event == event)
				{
					return transition_descriptor {t};
				}
			}
			return std::nullopt;
		}

		// ========================================================================
		// Property Access
		// ========================================================================

		[[nodiscard]] constexpr auto operator[](state_descriptor s) -> state_property_type&
		{
			return state_properties_[s];
		}

		[[nodiscard]] constexpr auto operator[](state_descriptor s) const -> const state_property_type&
		{
			return state_properties_[s];
		}

		[[nodiscard]] constexpr auto operator[](transition_descriptor t) -> transition_property_type&
		{
			return transition_properties_[t.value];
		}

		[[nodiscard]] constexpr auto operator[](transition_descriptor t) const -> const transition_property_type&
		{
			return transition_properties_[t.value];
		}

		// ========================================================================
		// Initial/Final State Management
		// ========================================================================

		constexpr void set_initial_state(state_descriptor s)
		{
			// Clear previous initial state
			if (initial_state_)
			{
				state_properties_[*initial_state_].is_initial = false;
			}
			initial_state_					= s;
			state_properties_[s].is_initial = true;
		}

		[[nodiscard]] constexpr auto initial_state() const noexcept -> std::optional<state_descriptor>
		{
			return initial_state_;
		}

		constexpr void set_final_state(state_descriptor s, bool is_final = true)
		{
			state_properties_[s].is_final = is_final;
		}

		[[nodiscard]] constexpr auto is_final_state(state_descriptor s) const noexcept -> bool
		{
			return state_properties_[s].is_final;
		}

		[[nodiscard]] constexpr auto final_states() const noexcept
		{
			return states() | std::views::filter(
								  [this](state_descriptor s)
								  {
									  return state_properties_[s].is_final;
								  });
		}

		// ========================================================================
		// State Data Access (convenience methods)
		// ========================================================================

		template<typename S = StateData>
			requires(not std::same_as<S, void>)
		[[nodiscard]] constexpr auto state_data(state_descriptor s) -> S&
		{
			return state_properties_[s].data;
		}

		template<typename S = StateData>
			requires(not std::same_as<S, void>)
		[[nodiscard]] constexpr auto state_data(state_descriptor s) const -> const S&
		{
			return state_properties_[s].data;
		}

		[[nodiscard]] constexpr auto transition_event(transition_descriptor t) const noexcept -> const Event&
		{
			return transition_properties_[t.value].event;
		}

		template<typename T = TransitionData>
			requires(not std::same_as<T, void>)
		[[nodiscard]] constexpr auto transition_data(transition_descriptor t) -> T&
		{
			return transition_properties_[t.value].data;
		}

		template<typename T = TransitionData>
			requires(not std::same_as<T, void>)
		[[nodiscard]] constexpr auto transition_data(transition_descriptor t) const -> const T&
		{
			return transition_properties_[t.value].data;
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
