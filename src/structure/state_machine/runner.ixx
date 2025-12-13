export module synodic.periapsis:structure.state_machine.runner;

import :structure.graph.visitor;
import :structure.state_machine.concepts;
import :structure.state_machine.state_machine;

import std;

export namespace peri::state_machine
{
	// ============================================================================
	// Re-export AdvanceResult from graph for unified API
	// ============================================================================

	using peri::graph::AdvanceResult;

	// ============================================================================
	// Process Result (FSM-specific, richer than AdvanceResult)
	// ============================================================================

	/**
	 * @brief Result of processing an event in the state machine runner.
	 *        Maps to AdvanceResult but adds FSM-specific states.
	 */
	enum class ProcessResult
	{
		Accepted,  // Transition was taken successfully (= AdvanceResult::Moved)
		Rejected,  // No valid transition for this event (= AdvanceResult::NoEdge)
		GuardFailed,  // Transition exists but guard returned false (= AdvanceResult::Blocked)
		NoCurrentState,	 // Runner has no current state (= AdvanceResult::InvalidState)
		InFinalState  // State machine is in a final state (FSM-specific)
	};

	/// Convert AdvanceResult to ProcessResult
	[[nodiscard]] constexpr auto to_process_result(AdvanceResult result) noexcept -> ProcessResult
	{
		switch (result)
		{
			case AdvanceResult::Moved :
				return ProcessResult::Accepted;
			case AdvanceResult::NoEdge :
				return ProcessResult::Rejected;
			case AdvanceResult::Blocked :
				return ProcessResult::GuardFailed;
			case AdvanceResult::InvalidState :
				return ProcessResult::NoCurrentState;
		}
		return ProcessResult::Rejected;
	}

	// ============================================================================
	// Runner Class - FSM-specific Walker
	// ============================================================================

	/**
	 * @brief Execution state for a state machine, implementing the visitor concept.
	 *
	 *        Runner tracks the current position in a state machine and handles
	 *        event processing, guards, and actions. It satisfies the graph::visitor
	 *        and graph::event_visitor concepts, allowing FSMs to work with graph
	 *        algorithms.
	 *
	 *        Relationship to graph::Visitor:
	 *        - Both satisfy the visitor concept
	 *        - Runner adds FSM semantics (initial/final states, guards, actions)
	 *        - Runner uses event-based edge selection
	 *        - Graph algorithms work on both via the visitor concept
	 *
	 * @tparam StateMachineType The state machine type to traverse
	 */
	template<typename StateMachineType>
	class Runner
	{
	public:
		// ========================================================================
		// Type Definitions (walker interface)
		// ========================================================================

		using state_machine_type	= std::remove_cvref_t<StateMachineType>;
		using event_type			= typename state_machine_type::event_type;
		using state_descriptor		= typename state_machine_type::state_descriptor;
		using transition_descriptor = typename state_machine_type::transition_descriptor;

		// Visitor-compatible aliases
		using vertex_descriptor = state_descriptor;
		using edge_descriptor	= transition_descriptor;

	private:
		// Pointer to state machine (const or non-const based on StateMachineType template param)
		// Using pointer instead of reference_wrapper to enable constexpr traversal
		std::conditional_t<
			std::is_const_v<std::remove_reference_t<StateMachineType>>,
			const state_machine_type*,
			state_machine_type*>
			machine_ {nullptr};

		std::optional<state_descriptor> current_state_ {};

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		/// Construct a runner for a state machine
		constexpr explicit Runner(StateMachineType& machine) :
			machine_(&machine)
		{
		}

		/// Construct and start from initial state
		constexpr Runner(StateMachineType& machine, std::nullptr_t) :
			machine_(&machine)
		{
			start();
		}

		/// Construct and start from specific state
		constexpr Runner(StateMachineType& machine, state_descriptor initial) :
			machine_(&machine),
			current_state_(initial)
		{
		}

		// ========================================================================
		// Visitor Interface - Position Query
		// ========================================================================

		/// Get current position (visitor interface)
		[[nodiscard]] constexpr auto position() const noexcept -> std::optional<vertex_descriptor>
		{
			return current_state_;
		}

		/// Check if positioned (visitor interface)
		[[nodiscard]] constexpr auto is_positioned() const noexcept -> bool
		{
			return current_state_.has_value();
		}

		// ========================================================================
		// Visitor Interface - Position Control
		// ========================================================================

		/// Move to a specific state (visitor interface)
		constexpr void move_to(vertex_descriptor v)
		{
			current_state_ = v;
		}

		/// Clear position (visitor interface)
		constexpr void reset()
		{
			current_state_ = std::nullopt;
		}

		// ========================================================================
		// Visitor Interface - Traversal
		// ========================================================================

		/// Advance by event (event_visitor interface)
		[[nodiscard]] constexpr auto advance(const event_type& event) -> AdvanceResult
		{
			if (!current_state_)
			{
				return AdvanceResult::InvalidState;
			}

			auto trans = machine_->find_transition(*current_state_, event);
			if (!trans)
			{
				return AdvanceResult::NoEdge;
			}

			current_state_ = machine_->target(*trans);
			return AdvanceResult::Moved;
		}

		/// Check if advance would succeed (event_visitor interface)
		[[nodiscard]] constexpr auto can_advance(const event_type& event) const noexcept -> bool
		{
			if (!current_state_)
			{
				return false;
			}
			return machine_->find_transition(*current_state_, event).has_value();
		}

		/// Peek at the transition that would be taken
		[[nodiscard]] constexpr auto peek(const event_type& event) const noexcept -> std::optional<edge_descriptor>
		{
			if (!current_state_)
			{
				return std::nullopt;
			}
			return machine_->find_transition(*current_state_, event);
		}

		// ========================================================================
		// FSM-Specific State Query
		// ========================================================================

		/// Get the current state (FSM alias for position())
		[[nodiscard]] constexpr auto current_state() const noexcept -> std::optional<state_descriptor>
		{
			return current_state_;
		}

		/// Check if the runner is in an active state (FSM alias for is_positioned())
		[[nodiscard]] constexpr auto is_running() const noexcept -> bool
		{
			return current_state_.has_value();
		}

		/// Check if the runner is in a final state (FSM-specific)
		[[nodiscard]] constexpr auto is_in_final_state() const noexcept -> bool
		{
			return current_state_ && machine_->is_final_state(*current_state_);
		}

		/// Get Proxy to the underlying state machine
		[[nodiscard]] constexpr auto machine() const noexcept -> StateMachineType&
		{
			return *machine_;
		}

		/// Check if runner has a valid state machine
		[[nodiscard]] constexpr auto has_machine() const noexcept -> bool
		{
			return machine_ != nullptr;
		}

		// ========================================================================
		// FSM-Specific Lifecycle
		// ========================================================================

		/// Start from the machine's initial state
		constexpr auto start() -> bool
		{
			auto init = machine_->initial_state();
			if (init)
			{
				current_state_ = *init;
				return true;
			}
			return false;
		}

		/// Start from a specific state (FSM alias for move_to())
		constexpr void start(state_descriptor s)
		{
			current_state_ = s;
		}

		// ========================================================================
		// FSM Event Processing (with guards/actions)
		// ========================================================================

		/// Process an event - find matching transition and take it
		/// Returns FSM-specific ProcessResult (richer than AdvanceResult)
		[[nodiscard]] constexpr auto process(const event_type& event) -> ProcessResult
		{
			if (!current_state_)
			{
				return ProcessResult::NoCurrentState;
			}

			if (machine_->is_final_state(*current_state_))
			{
				return ProcessResult::InFinalState;
			}

			auto trans = machine_->find_transition(*current_state_, event);
			if (!trans)
			{
				return ProcessResult::Rejected;
			}

			current_state_ = machine_->target(*trans);
			return ProcessResult::Accepted;
		}

		/// Process an event with a guard predicate
		template<typename Guard>
			requires std::is_invocable_r_v<bool, Guard, transition_descriptor>
		[[nodiscard]] constexpr auto process(const event_type& event, Guard&& guard) -> ProcessResult
		{
			if (!current_state_)
			{
				return ProcessResult::NoCurrentState;
			}

			if (machine_->is_final_state(*current_state_))
			{
				return ProcessResult::InFinalState;
			}

			auto trans = machine_->find_transition(*current_state_, event);
			if (!trans)
			{
				return ProcessResult::Rejected;
			}

			if (!std::invoke(std::forward<Guard>(guard), *trans))
			{
				return ProcessResult::GuardFailed;
			}

			current_state_ = machine_->target(*trans);
			return ProcessResult::Accepted;
		}

		/// Process an event with guard and action
		template<typename Guard, typename Action>
			requires std::is_invocable_r_v<bool, Guard, transition_descriptor> and
					 std::is_invocable_v<Action, transition_descriptor>
		[[nodiscard]] constexpr auto process(const event_type& event, Guard&& guard, Action&& action) -> ProcessResult
		{
			if (!current_state_)
			{
				return ProcessResult::NoCurrentState;
			}

			if (machine_->is_final_state(*current_state_))
			{
				return ProcessResult::InFinalState;
			}

			auto trans = machine_->find_transition(*current_state_, event);
			if (!trans)
			{
				return ProcessResult::Rejected;
			}

			if (!std::invoke(std::forward<Guard>(guard), *trans))
			{
				return ProcessResult::GuardFailed;
			}

			current_state_ = machine_->target(*trans);
			std::invoke(std::forward<Action>(action), *trans);
			return ProcessResult::Accepted;
		}

		/// Process with action only (no guard)
		template<typename Action>
			requires std::is_invocable_v<Action, transition_descriptor>
		[[nodiscard]] constexpr auto process_with_action(const event_type& event, Action&& action) -> ProcessResult
		{
			return process(
				event,
				[](auto)
				{
					return true;
				},
				std::forward<Action>(action));
		}

		// ========================================================================
		// FSM Query Operations
		// ========================================================================

		/// Check if an event can be processed (FSM alias for can_advance())
		[[nodiscard]] constexpr auto can_process(const event_type& event) const noexcept -> bool
		{
			if (!current_state_)
			{
				return false;
			}
			return machine_->find_transition(*current_state_, event).has_value();
		}

		/// Get all available events from current state
		[[nodiscard]] constexpr auto available_events() const noexcept
		{
			if (current_state_)
			{
				return machine_->out_transitions(*current_state_) | std::views::transform(
																		[this](transition_descriptor t)
																		{
																			return machine_->transition_event(t);
																		});
			}
			// Return empty range
			return machine_->out_transitions(state_descriptor {0}) |
				   std::views::filter(
					   [](auto)
					   {
						   return false;
					   }) |
				   std::views::transform(
					   [this](transition_descriptor t)
					   {
						   return machine_->transition_event(t);
					   });
		}

		/// Get all available transitions from current state (visitor-compatible out_edges)
		[[nodiscard]] constexpr auto available_transitions() const noexcept
		{
			if (current_state_)
			{
				return machine_->out_transitions(*current_state_);
			}
			// Return empty range
			return machine_->out_transitions(state_descriptor {0}) | std::views::filter(
																		 [](auto)
																		 {
																			 return false;
																		 });
		}

		/// Alias for visitor compatibility (out_edges equivalent)
		[[nodiscard]] constexpr auto out_edges() const noexcept
		{
			return available_transitions();
		}
	};

	// ============================================================================
	// Deduction Guides
	// ============================================================================

	template<typename SM>
	Runner(SM&) -> Runner<SM>;

	template<typename SM>
	Runner(SM&, std::nullptr_t) -> Runner<SM>;

	template<typename SM, typename SD>
	Runner(SM&, SD) -> Runner<SM>;

	// ============================================================================
	// Factory Functions
	// ============================================================================

	/// Create a runner for a state machine
	template<typename StateMachineType>
	[[nodiscard]] constexpr auto make_runner(StateMachineType& machine) -> Runner<StateMachineType>
	{
		return Runner<StateMachineType>(machine);
	}

	/// Create a runner and start from initial state
	template<typename StateMachineType>
	[[nodiscard]] constexpr auto make_started_runner(StateMachineType& machine) -> Runner<StateMachineType>
	{
		Runner<StateMachineType> runner(machine);
		runner.start();
		return runner;
	}

	/// Create a runner starting from a specific state
	template<typename StateMachineType>
	[[nodiscard]] constexpr auto
		make_runner_at(StateMachineType& machine, typename StateMachineType::state_descriptor state)
			-> Runner<StateMachineType>
	{
		return Runner<StateMachineType>(machine, state);
	}
}
