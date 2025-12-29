export module synodic.periapsis:structure.automata.runner;

import :structure.graph.visitor;
import :structure.automata.concepts;
import :structure.automata.state_machine;

import std;

export namespace peri::automata
{
	using graph::AdvanceResult;

	// ============================================================================
	// Process Result
	// ============================================================================

	/**
	 * @brief Result of processing an event in the state machine runner.
	 */
	enum class ProcessResult
	{
		ACCEPTED,		   // Transition was taken successfully
		REJECTED,		   // No valid transition for this event
		GUARD_FAILED,	   // Transition exists but guard returned false
		NO_CURRENT_STATE,  // Runner has no current state
		IN_FINAL_STATE	   // State machine is in a final state
	};

	/**
	 * @brief Convert AdvanceResult to ProcessResult
	 */
	[[nodiscard]] constexpr auto ToProcessResult(const AdvanceResult result) noexcept -> ProcessResult
	{
		switch (result)
		{
			case AdvanceResult::MOVED :
				return ProcessResult::ACCEPTED;
			case AdvanceResult::NO_EDGE :
				return ProcessResult::REJECTED;
			case AdvanceResult::BLOCKED :
				return ProcessResult::GUARD_FAILED;
			case AdvanceResult::INVALID_STATE :
				return ProcessResult::NO_CURRENT_STATE;
		}
		return ProcessResult::REJECTED;
	}

	// ============================================================================
	// Runner traversal
	// ============================================================================

	/**
	 * @brief Execution state for a state machine.
	 *
	 * @tparam StateMachineType The state machine type to traverse
	 */
	template<typename StateMachineType>
	class Runner
	{
	public:
		// ========================================================================
		// Type Definitions
		// ========================================================================

		using StateMachineT        = std::remove_cvref_t<StateMachineType>;
		using EventType            = StateMachineT::EventType;
		using StateDescriptor      = StateMachineT::StateDescriptor;
		using TransitionDescriptor = StateMachineT::TransitionDescriptor;

		// Visitor-compatible aliases
		using VertexDescriptor = StateDescriptor;
		using EdgeDescriptor   = TransitionDescriptor;

	private:
		std::conditional_t<
			std::is_const_v<std::remove_reference_t<StateMachineType>>,
			std::reference_wrapper<const StateMachineT>,
			std::reference_wrapper<StateMachineT>>
			machine_;

		std::optional<StateDescriptor> currentState_ {};

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		/**
		 * @brief Construct a runner for a state machine
		 */
		constexpr explicit Runner(StateMachineType& machine) :
			machine_(machine)
		{
		}

		/**
		 * @brief Construct and start from initial state
		 */
		constexpr Runner(StateMachineType& machine, std::nullptr_t) :
			machine_(machine)
		{
			Start();
		}

		/**
		 * @brief Construct and start from specific state
		 */
		constexpr Runner(StateMachineType& machine, StateDescriptor initial) :
			machine_(machine),
			currentState_(initial)
		{
		}

		// ========================================================================
		// Visitor Interface - Position Query
		// ========================================================================

		/**
		 * @brief Get current position
		 */
		[[nodiscard]] constexpr auto Position() const noexcept -> std::optional<VertexDescriptor>
		{
			return currentState_;
		}

		/**
		 * @brief Check if positioned
		 */
		[[nodiscard]] constexpr auto IsPositioned() const noexcept -> bool
		{
			return currentState_.has_value();
		}

		// ========================================================================
		// Visitor Interface - Position Control
		// ========================================================================

		/**
		 * @brief Move to a specific state
		 */
		constexpr void MoveTo(VertexDescriptor v)
		{
			currentState_ = v;
		}

		/**
		 * @brief Clear position
		 */
		constexpr void Reset()
		{
			currentState_ = std::nullopt;
		}

		// ========================================================================
		// Visitor Interface - Traversal
		// ========================================================================

		/**
		 * @brief Advance by event
		 */
		[[nodiscard]] constexpr auto Advance(const EventType& event) -> AdvanceResult
		{
			if (!currentState_)
			{
				return AdvanceResult::INVALID_STATE;
			}

			auto trans = machine_.get().FindTransition(*currentState_, event);
			if (!trans)
			{
				return AdvanceResult::NO_EDGE;
			}

			currentState_ = machine_.get().Target(*trans);
			return AdvanceResult::MOVED;
		}

		/**
		 * @brief Check if advance would succeed
		 */
		[[nodiscard]] constexpr auto CanAdvance(const EventType& event) const noexcept -> bool
		{
			if (!currentState_)
			{
				return false;
			}
			return machine_.get().FindTransition(*currentState_, event).has_value();
		}

		/**
		 * @brief Peek at the transition that would be taken
		 */
		[[nodiscard]] constexpr auto Peek(const EventType& event) const noexcept -> std::optional<EdgeDescriptor>
		{
			if (!currentState_)
			{
				return std::nullopt;
			}
			return machine_.get().FindTransition(*currentState_, event);
		}

		// ========================================================================
		// FSM-Specific State Query
		// ========================================================================

		/**
		 * @brief Get the current state
		 */
		[[nodiscard]] constexpr auto CurrentState() const noexcept -> std::optional<StateDescriptor>
		{
			return currentState_;
		}

		/**
		 * @brief Check if the runner is in an active state (FSM alias for IsPositioned())
		 */
		[[nodiscard]] constexpr auto IsRunning() const noexcept -> bool
		{
			return currentState_.has_value();
		}

		/**
		 * @brief Check if the runner is in a final state
		 */
		[[nodiscard]] constexpr auto IsInFinalState() const noexcept -> bool
		{
			return currentState_ && machine_.get().IsFinalState(*currentState_);
		}

		/**
		 * @brief Get Proxy to the underlying state machine
		 */
		[[nodiscard]] constexpr auto Machine() const noexcept -> StateMachineT&
		{
			return machine_.get();
		}

		// ========================================================================
		// FSM-Specific Lifecycle
		// ========================================================================

		/**
		 * @brief Start from the machine's initial state
		 */
		constexpr auto Start() -> bool
		{
			auto init = machine_.get().InitialState();
			if (init)
			{
				currentState_ = *init;
				return true;
			}
			return false;
		}

		/**
		 * @brief Start from a specific state
		 */
		constexpr void Start(StateDescriptor s)
		{
			currentState_ = s;
		}

		// ========================================================================
		// FSM Event Processing
		// ========================================================================

		/**
		 * @brief Process an event. Find matching transition and take it.
		 */
		[[nodiscard]] constexpr auto Process(const EventType& event) -> ProcessResult
		{
			if (!currentState_)
			{
				return ProcessResult::NO_CURRENT_STATE;
			}

			if (machine_.get().IsFinalState(*currentState_))
			{
				return ProcessResult::IN_FINAL_STATE;
			}

			auto trans = machine_.get().FindTransition(*currentState_, event);
			if (!trans)
			{
				return ProcessResult::REJECTED;
			}

			currentState_ = machine_.get().Target(*trans);
			return ProcessResult::ACCEPTED;
		}

		/**
		 * @brief Process an event with a guard predicate
		 */
		template<typename Guard>
			requires std::is_invocable_r_v<bool, Guard, TransitionDescriptor>
		[[nodiscard]] constexpr auto Process(const EventType& event, Guard&& guard) -> ProcessResult
		{
			if (!currentState_)
			{
				return ProcessResult::NO_CURRENT_STATE;
			}

			if (machine_.get().IsFinalState(*currentState_))
			{
				return ProcessResult::IN_FINAL_STATE;
			}

			auto trans = machine_.get().FindTransition(*currentState_, event);
			if (!trans)
			{
				return ProcessResult::REJECTED;
			}

			if (!std::invoke(std::forward<Guard>(guard), *trans))
			{
				return ProcessResult::GUARD_FAILED;
			}

			currentState_ = machine_.get().Target(*trans);
			return ProcessResult::ACCEPTED;
		}

		/**
		 * @brief Process an event with guard and action
		 */
		template<typename Guard, typename Action>
			requires std::is_invocable_r_v<bool, Guard, TransitionDescriptor> and
					 std::is_invocable_v<Action, TransitionDescriptor>
		[[nodiscard]] constexpr auto Process(const EventType& event, Guard&& guard, Action&& action) -> ProcessResult
		{
			if (!currentState_)
			{
				return ProcessResult::NO_CURRENT_STATE;
			}

			if (machine_.get().IsFinalState(*currentState_))
			{
				return ProcessResult::IN_FINAL_STATE;
			}

			auto trans = machine_.get().FindTransition(*currentState_, event);
			if (!trans)
			{
				return ProcessResult::REJECTED;
			}

			if (!std::invoke(std::forward<Guard>(guard), *trans))
			{
				return ProcessResult::GUARD_FAILED;
			}

			currentState_ = machine_.get().Target(*trans);
			std::invoke(std::forward<Action>(action), *trans);
			return ProcessResult::ACCEPTED;
		}

		/**
		 * @brief Process with action only
		 */
		template<typename Action>
			requires std::is_invocable_v<Action, TransitionDescriptor>
		[[nodiscard]] constexpr auto ProcessWithAction(const EventType& event, Action&& action) -> ProcessResult
		{
			return Process(
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

		/**
		 * @brief Check if an event can be processed
		 */
		[[nodiscard]] constexpr auto CanProcess(const EventType& event) const noexcept -> bool
		{
			if (!currentState_)
			{
				return false;
			}
			return machine_.get().FindTransition(*currentState_, event).has_value();
		}

		/**
		 * @brief Get all available events from current state
		 */
		[[nodiscard]] constexpr auto AvailableEvents() const noexcept
		{
			if (currentState_)
			{
				return machine_.get().OutTransitions(*currentState_) | std::views::transform(
																		  [this](TransitionDescriptor t)
																		  {
																			  return machine_.get().TransitionEvent(t);
																		  });
			}
			// Return empty range
			return machine_.get().OutTransitions(StateDescriptor {0}) |
				   std::views::filter(
					   [](auto)
					   {
						   return false;
					   }) |
				   std::views::transform(
					   [this](TransitionDescriptor t)
					   {
						   return machine_.get().TransitionEvent(t);
					   });
		}

		/**
		 * @brief Get all available transitions from current state
		 */
		[[nodiscard]] constexpr auto AvailableTransitions() const noexcept
		{
			if (currentState_)
			{
				return machine_.get().OutTransitions(*currentState_);
			}
			// Return empty range
			return machine_.get().OutTransitions(StateDescriptor {0}) | std::views::filter(
																		   [](auto)
																		   {
																			   return false;
																		   });
		}

		/**
		 * @brief Alias for visitor compatibility
		 */
		[[nodiscard]] constexpr auto OutEdges() const noexcept
		{
			return AvailableTransitions();
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
}
