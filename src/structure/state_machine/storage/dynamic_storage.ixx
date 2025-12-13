export module synodic.periapsis:structure.state_machine.storage.dynamic_storage;

import :structure.state_machine.concepts;
import :structure.state_machine.storage.static_storage;

import std;

export namespace peri::state_machine::storage
{
	/**
	 * @brief Dynamic state machine storage supporting runtime state/transition add/remove.
	 *        Uses adjacency list layout for efficient traversal with full mutability.
	 *
	 *        Memory layout:
	 *        - State properties stored contiguously in a vector (O(1) access by descriptor)
	 *        - Each state has a separate transition list (out-transitions)
	 *
	 *        Performance characteristics:
	 *        - O(1) state property access
	 *        - O(1) add state (amortized)
	 *        - O(1) add transition (amortized)
	 *        - O(degree) remove transition
	 *        - O(T) remove state (must update transition references)
	 *
	 * @tparam Event Type used to trigger transitions
	 * @tparam StateData Type stored per state (use void for none)
	 * @tparam TransitionData Type stored per transition beyond the event (use void for none)
	 * @tparam StateContainer Container template for state storage
	 * @tparam TransitionContainer Container template for per-state transition lists
	 * @tparam IndexType Unsigned integer type for descriptors
	 */
	template<
		typename Event,
		typename StateData,
		typename TransitionData,
		template<typename...> typename StateContainer	   = std::vector,
		template<typename...> typename TransitionContainer = std::vector,
		std::unsigned_integral IndexType				   = std::size_t>
	class DynamicStorage
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

		// Transition descriptor: (source state, index within source's transition list)
		struct transition_descriptor
		{
			state_descriptor source;
			IndexType transition_index;

			constexpr bool operator==(const transition_descriptor&) const noexcept	= default;
			constexpr auto operator<=>(const transition_descriptor&) const noexcept = default;
		};

		// Compile-time properties
		static constexpr bool is_constexpr_capable				= false;
		static constexpr bool has_stable_state_descriptors		= false;  // Invalidated on remove
		static constexpr bool has_stable_transition_descriptors = false;

	private:
		// Transition storage: target state + property
		struct TransitionData_
		{
			state_descriptor target;
			transition_property_type property;
		};

		// Per-state data: property + out-transition list
		struct StateData_
		{
			state_property_type property;
			TransitionContainer<TransitionData_> out_transitions;
		};

		StateContainer<StateData_> states_;
		transitions_size_type total_transitions_ {0};
		std::optional<state_descriptor> initial_state_ {};

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		DynamicStorage() = default;

		explicit DynamicStorage(states_size_type initial_capacity)
		{
			states_.reserve(initial_capacity);
		}

		// ========================================================================
		// State Operations
		// ========================================================================

		[[nodiscard]] auto num_states() const noexcept -> states_size_type
		{
			return static_cast<states_size_type>(states_.size());
		}

		[[nodiscard]] auto states() const noexcept
		{
			return std::views::iota(state_descriptor {0}, num_states());
		}

		auto add_state() -> state_descriptor
		{
			auto s = static_cast<state_descriptor>(states_.size());
			states_.emplace_back();
			return s;
		}

		template<typename... Args>
			requires std::constructible_from<StateData, Args...>
		auto add_state(Args&&... args) -> state_descriptor
		{
			auto s			   = static_cast<state_descriptor>(states_.size());
			auto& data		   = states_.emplace_back();
			data.property.data = StateData(std::forward<Args>(args)...);
			return s;
		}

		void remove_state(state_descriptor s)
		{
			// Remove all transitions from this state
			total_transitions_ -= static_cast<transitions_size_type>(states_[s].out_transitions.size());
			states_[s].out_transitions.clear();

			// Remove all transitions targeting this state from other states
			for (auto& state_data: states_)
			{
				std::erase_if(
					state_data.out_transitions,
					[s, this](const TransitionData_& t)
					{
						if (t.target == s)
						{
							--total_transitions_;
							return true;
						}
						return false;
					});
			}

			// Handle initial state
			if (initial_state_ && *initial_state_ == s)
			{
				initial_state_ = std::nullopt;
			}

			// Swap with last and pop (stable_state_descriptors = false)
			if (s != states_.size() - 1)
			{
				auto last = static_cast<state_descriptor>(states_.size() - 1);

				// Update all transitions pointing to 'last' to point to 's'
				for (auto& state_data: states_)
				{
					for (auto& trans: state_data.out_transitions)
					{
						if (trans.target == last)
						{
							trans.target = s;
						}
					}
				}

				// Update initial state if it was 'last'
				if (initial_state_ && *initial_state_ == last)
				{
					initial_state_ = s;
				}

				states_[s] = std::move(states_[last]);
			}
			states_.pop_back();
		}

		void clear_states()
		{
			states_.clear();
			total_transitions_ = 0;
			initial_state_	   = std::nullopt;
		}

		// ========================================================================
		// Transition Operations
		// ========================================================================

		[[nodiscard]] auto num_transitions() const noexcept -> transitions_size_type
		{
			return total_transitions_;
		}

		auto add_transition(state_descriptor source, state_descriptor target, const Event& event)
			-> std::pair<transition_descriptor, bool>
		{
			auto trans_index = static_cast<IndexType>(states_[source].out_transitions.size());
			states_[source].out_transitions.push_back(TransitionData_ {target, transition_property_type {event}});

			++total_transitions_;
			return {
				transition_descriptor {source, trans_index},
				true
			};
		}

		template<typename... Args>
			requires std::constructible_from<TransitionData, Args...>
		auto add_transition(state_descriptor source, state_descriptor target, const Event& event, Args&&... args)
			-> std::pair<transition_descriptor, bool>
		{
			auto trans_index = static_cast<IndexType>(states_[source].out_transitions.size());
			auto prop		 = transition_property_type {event};
			prop.data		 = TransitionData(std::forward<Args>(args)...);
			states_[source].out_transitions.push_back(TransitionData_ {target, std::move(prop)});

			++total_transitions_;
			return {
				transition_descriptor {source, trans_index},
				true
			};
		}

		void remove_transition(transition_descriptor t)
		{
			auto& out = states_[t.source].out_transitions;
			if (t.transition_index < out.size())
			{
				out.erase(out.begin() + t.transition_index);
				--total_transitions_;
			}
		}

		void remove_transition(state_descriptor source, state_descriptor target, const Event& event)
		{
			auto& out = states_[source].out_transitions;
			auto it	  = std::ranges::find_if(
				  out,
				  [target, &event](const TransitionData_& t)
				  {
					  return t.target == target && t.property.event == event;
				  });

			if (it != out.end())
			{
				out.erase(it);
				--total_transitions_;
			}
		}

		void clear_transitions()
		{
			for (auto& s: states_)
			{
				s.out_transitions.clear();
			}
			total_transitions_ = 0;
		}

		// ========================================================================
		// Transition Enumeration
		// ========================================================================

		class TransitionIterator
		{
			const DynamicStorage* storage_;
			state_descriptor current_state_;
			IndexType current_trans_;

			void advance_to_valid()
			{
				while (current_state_ < storage_->num_states())
				{
					if (current_trans_ < storage_->states_[current_state_].out_transitions.size())
					{
						return;
					}
					++current_state_;
					current_trans_ = 0;
				}
			}

		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type		= transition_descriptor;
			using difference_type	= std::ptrdiff_t;
			using pointer			= const transition_descriptor*;
			using reference			= transition_descriptor;

			TransitionIterator() = default;

			TransitionIterator(const DynamicStorage* s, state_descriptor state, IndexType trans) :
				storage_(s),
				current_state_(state),
				current_trans_(trans)
			{
				advance_to_valid();
			}

			auto operator*() const -> transition_descriptor
			{
				return {current_state_, current_trans_};
			}

			auto operator++() -> TransitionIterator&
			{
				++current_trans_;
				advance_to_valid();
				return *this;
			}

			auto operator++(int) -> TransitionIterator
			{
				auto tmp = *this;
				++(*this);
				return tmp;
			}

			bool operator==(const TransitionIterator& other) const
			{
				return current_state_ == other.current_state_ && current_trans_ == other.current_trans_;
			}
		};

		[[nodiscard]] auto transitions() const noexcept
		{
			return std::ranges::subrange(
				TransitionIterator {this, state_descriptor {0}, IndexType {0}},
				TransitionIterator {this, num_states(), IndexType {0}});
		}

		// ========================================================================
		// Incidence (Out-Transition Traversal)
		// ========================================================================

		[[nodiscard]] auto out_degree(state_descriptor s) const noexcept -> degree_size_type
		{
			return static_cast<degree_size_type>(states_[s].out_transitions.size());
		}

		[[nodiscard]] auto out_transitions(state_descriptor s) const noexcept
		{
			return std::views::iota(IndexType {0}, static_cast<IndexType>(states_[s].out_transitions.size())) |
				   std::views::transform(
					   [s](IndexType i)
					   {
						   return transition_descriptor {s, i};
					   });
		}

		[[nodiscard]] auto source(transition_descriptor t) const noexcept -> state_descriptor
		{
			return t.source;
		}

		[[nodiscard]] auto target(transition_descriptor t) const noexcept -> state_descriptor
		{
			return states_[t.source].out_transitions[t.transition_index].target;
		}

		// ========================================================================
		// Event-Based Transition Lookup
		// ========================================================================

		[[nodiscard]] auto find_transition(state_descriptor s, const Event& event) const noexcept
			-> std::optional<transition_descriptor>
		{
			const auto& out = states_[s].out_transitions;
			for (IndexType i = 0; i < out.size(); ++i)
			{
				if (out[i].property.event == event)
				{
					return transition_descriptor {s, i};
				}
			}
			return std::nullopt;
		}

		// ========================================================================
		// Property Access
		// ========================================================================

		[[nodiscard]] auto operator[](state_descriptor s) -> state_property_type&
		{
			return states_[s].property;
		}

		[[nodiscard]] auto operator[](state_descriptor s) const -> const state_property_type&
		{
			return states_[s].property;
		}

		[[nodiscard]] auto operator[](transition_descriptor t) -> transition_property_type&
		{
			return states_[t.source].out_transitions[t.transition_index].property;
		}

		[[nodiscard]] auto operator[](transition_descriptor t) const -> const transition_property_type&
		{
			return states_[t.source].out_transitions[t.transition_index].property;
		}

		// ========================================================================
		// Initial/Final State Management
		// ========================================================================

		void set_initial_state(state_descriptor s)
		{
			if (initial_state_)
			{
				states_[*initial_state_].property.is_initial = false;
			}
			initial_state_				   = s;
			states_[s].property.is_initial = true;
		}

		[[nodiscard]] auto initial_state() const noexcept -> std::optional<state_descriptor>
		{
			return initial_state_;
		}

		void set_final_state(state_descriptor s, bool is_final = true)
		{
			states_[s].property.is_final = is_final;
		}

		[[nodiscard]] auto is_final_state(state_descriptor s) const noexcept -> bool
		{
			return states_[s].property.is_final;
		}

		[[nodiscard]] auto final_states() const noexcept
		{
			return states() | std::views::filter(
								  [this](state_descriptor s)
								  {
									  return states_[s].property.is_final;
								  });
		}

		// ========================================================================
		// State Data Access (convenience methods)
		// ========================================================================

		template<typename S = StateData>
			requires(not std::same_as<S, void>)
		[[nodiscard]] auto state_data(state_descriptor s) -> S&
		{
			return states_[s].property.data;
		}

		template<typename S = StateData>
			requires(not std::same_as<S, void>)
		[[nodiscard]] auto state_data(state_descriptor s) const -> const S&
		{
			return states_[s].property.data;
		}

		[[nodiscard]] auto transition_event(transition_descriptor t) const noexcept -> const Event&
		{
			return states_[t.source].out_transitions[t.transition_index].property.event;
		}

		template<typename T = TransitionData>
			requires(not std::same_as<T, void>)
		[[nodiscard]] auto transition_data(transition_descriptor t) -> T&
		{
			return states_[t.source].out_transitions[t.transition_index].property.data;
		}

		template<typename T = TransitionData>
			requires(not std::same_as<T, void>)
		[[nodiscard]] auto transition_data(transition_descriptor t) const -> const T&
		{
			return states_[t.source].out_transitions[t.transition_index].property.data;
		}
	};

	// ============================================================================
	// Convenience Aliases
	// ============================================================================

	/**
	 * @brief Dynamic storage with integral events and no additional data
	 */
	template<std::unsigned_integral IndexType = std::size_t>
	using DynamicTopology = DynamicStorage<int, void, void, std::vector, std::vector, IndexType>;

	/**
	 * @brief Dynamic storage with typed events and state data
	 */
	template<typename Event, typename StateData, std::unsigned_integral IndexType = std::size_t>
	using DynamicStateStorage = DynamicStorage<Event, StateData, void, std::vector, std::vector, IndexType>;

	/**
	 * @brief Dynamic storage with typed events and transition data
	 */
	template<typename Event, typename TransitionData, std::unsigned_integral IndexType = std::size_t>
	using DynamicTransitionStorage = DynamicStorage<Event, void, TransitionData, std::vector, std::vector, IndexType>;
}
