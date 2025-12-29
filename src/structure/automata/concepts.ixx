export module synodic.periapsis:structure.automata.concepts;

import std;

export namespace peri::automata
{
	// ============================================================================
	// Core State Machine Types
	// ============================================================================

	/**
	 * @brief A state descriptor provides O(1) access to state data
	 *        States are represented as graph vertices
	 */
	template<typename D>
	concept state_descriptor = std::regular<D> and std::totally_ordered<D>;

	/**
	 * @brief A transition descriptor provides O(1) access to transition data
	 *        Transitions are represented as graph edges
	 */
	template<typename D>
	concept transition_descriptor = std::regular<D>;

	/**
	 * @brief An event type that can trigger state transitions
	 *        Events must be equality-comparable for matching
	 */
	template<typename E>
	concept event_type = std::regular<E>;

	// ============================================================================
	// Guard and Action Concepts
	// ============================================================================

	/**
	 * @brief A guard is a predicate that determines if a transition can be taken
	 *        Guards can optionally receive context data
	 */
	template<typename G>
	concept guard = std::is_invocable_r_v<bool, G>;

	/**
	 * @brief A guard that receives context data
	 */
	template<typename G, typename Context>
	concept guard_with_context = std::is_invocable_r_v<bool, G, const Context&>;

	/**
	 * @brief An action is executed when a transition is taken
	 */
	template<typename A>
	concept action = std::is_invocable_v<A>;

	/**
	 * @brief An action that receives context data
	 */
	template<typename A, typename Context>
	concept action_with_context = std::is_invocable_v<A, Context&>;

	// ============================================================================
	// State Machine Storage Concepts
	// ============================================================================

	/**
	 * @brief Minimal requirements for state machine storage backend
	 *        Storage is responsible for holding states, transitions, and their relationships
	 */
	template<typename S>
	concept state_machine_storage = requires
	{
		typename S::StateDescriptor;
		typename S::TransitionDescriptor;
		typename S::StatesSizeType;
		typename S::TransitionsSizeType;
		typename S::EventType;

		requires state_descriptor<typename S::StateDescriptor>;
		requires transition_descriptor<typename S::TransitionDescriptor>;
		requires std::unsigned_integral<typename S::StatesSizeType>;
		requires std::unsigned_integral<typename S::TransitionsSizeType>;
		requires event_type<typename S::EventType>;
	};

	/**
	 * @brief Storage that provides state enumeration
	 */
	template<typename S>
	concept state_list_storage = state_machine_storage<S> and requires(const S& storage)
	{
		{ storage.States() } -> std::ranges::input_range;
		{ storage.NumStates() } -> std::same_as<typename S::StatesSizeType>;
	};

	/**
	 * @brief Storage that provides transition enumeration
	 */
	template<typename S>
	concept transition_list_storage = state_machine_storage<S> and requires(const S& storage)
	{
		{ storage.Transitions() } -> std::ranges::input_range;
		{ storage.NumTransitions() } -> std::same_as<typename S::TransitionsSizeType>;
	};

	/**
	 * @brief Storage supporting outgoing transition traversal from a state
	 */
	template<typename S>
	concept incidence_storage =
		state_machine_storage<S> and
		requires(const S& storage, typename S::StateDescriptor s, typename S::TransitionDescriptor t)
		{
			{ storage.OutTransitions(s) } -> std::ranges::input_range;
			{ storage.OutDegree(s) } -> std::unsigned_integral;
			{ storage.Source(t) } -> std::same_as<typename S::StateDescriptor>;
			{ storage.Target(t) } -> std::same_as<typename S::StateDescriptor>;
		};

	/**
	 * @brief Storage supporting state addition
	 */
	template<typename S>
	concept appendable_state_storage = state_machine_storage<S> and requires(S& storage)
	{
		{ storage.AddState() } -> std::same_as<typename S::StateDescriptor>;
	};

	/**
	 * @brief Storage supporting transition addition
	 */
	template<typename S>
	concept appendable_transition_storage = state_machine_storage<S> and requires(
	                                        S& storage,
	                                        typename S::StateDescriptor from,
	                                        typename S::StateDescriptor to,
	                                        typename S::EventType event)
	                                        {
		                                        {
			                                        storage.AddTransition(from, to, event)
		                                        } -> std::same_as<std::pair<typename S::TransitionDescriptor, bool>>;
	                                        };

	/**
	 * @brief Storage supporting state removal
	 */
	template<typename S>
	concept mutable_state_storage =
		appendable_state_storage<S> and requires(S& storage, typename S::StateDescriptor s)
		{
			{ storage.RemoveState(s) };
			{ storage.ClearStates() };
		};

	/**
	 * @brief Storage supporting transition removal
	 */
	template<typename S>
	concept mutable_transition_storage =
		appendable_transition_storage<S> and requires(S& storage, typename S::TransitionDescriptor t)
		{
			{ storage.RemoveTransition(t) };
			{ storage.ClearTransitions() };
		};

	/**
	 * @brief Fully mutable storage
	 */
	template<typename S>
	concept mutable_storage = mutable_state_storage<S> and mutable_transition_storage<S>;

	/**
	 * @brief Append-only storage
	 */
	template<typename S>
	concept appendable_storage = appendable_state_storage<S> and appendable_transition_storage<S>;

	// ============================================================================
	// Property Access Concepts
	// ============================================================================

	/**
	 * @brief Storage providing O(1) state property access
	 */
	template<typename S>
	concept state_property_storage = state_machine_storage<S> and requires { typename S::StatePropertyType; } and
	                                 requires(S& storage, const S& constStorage, typename S::StateDescriptor s)
	                                 {
		                                 { storage[s] } -> std::same_as<typename S::StatePropertyType&>;
		                                 { constStorage[s] } -> std::same_as<const typename S::StatePropertyType&>;
	                                 };

	/**
	 * @brief Storage providing O(1) transition property access
	 */
	template<typename S>
	concept transition_property_storage = state_machine_storage<S> and requires
	                                      {
		                                      typename S::TransitionPropertyType;
	                                      } and requires(
	                                      S& storage,
	                                      const S& constStorage,
	                                      typename S::TransitionDescriptor t)
	                                      {
		                                      { storage[t] } -> std::same_as<typename S::TransitionPropertyType&>;
		                                      {
			                                      constStorage[t]
		                                      } -> std::same_as<const typename S::TransitionPropertyType&>;
	                                      };

	// ============================================================================
	// Event Matching Concepts
	// ============================================================================
	/**
	 * @brief Storage that supports event-based transition lookup
	 */
	template<typename S>
	concept event_queryable_storage =
		state_machine_storage<S> and incidence_storage<S> and
		requires(const S& storage, typename S::StateDescriptor s, typename S::EventType event)
		{
			{ storage.FindTransition(s, event) } -> std::same_as<std::optional<typename S::TransitionDescriptor>>;
		};

	// ============================================================================
	// Initial/Final State Concepts
	// ============================================================================

	/**
	 * @brief Storage that tracks initial state
	 */
	template<typename S>
	concept has_initial_state = state_machine_storage<S> and requires(const S& storage)
	                            {
		                            {
			                            storage.InitialState()
		                            } -> std::same_as<std::optional<typename S::StateDescriptor>>;
	                            } and requires(S& storage, typename S::StateDescriptor s)
	                            {
		                            { storage.SetInitialState(s) };
	                            };

	/**
	 * @brief Storage that tracks final states
	 */
	template<typename S>
	concept has_final_states = state_machine_storage<S> and requires(const S& storage, typename S::StateDescriptor s)
	                           {
		                           { storage.IsFinalState(s) } -> std::same_as<bool>;
		                           { storage.FinalStates() } -> std::ranges::input_range;
	                           } and requires(S& storage, typename S::StateDescriptor s, bool isFinal)
	                           {
		                           { storage.SetFinalState(s, isFinal) };
	                           };

	// ============================================================================
	// Combined Concepts
	// ============================================================================

	/**
	 * @brief A complete state machine storage with all traversal capabilities
	 */
	template<typename S>
	concept traversable_storage = state_list_storage<S> and transition_list_storage<S> and incidence_storage<S>;

	/**
	 * @brief A fully-featured state machine storage
	 */
	template<typename S>
	concept complete_storage = traversable_storage<S> and appendable_storage<S> and event_queryable_storage<S> and
	                           has_initial_state<S> and has_final_states<S>;
}
