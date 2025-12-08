export module synodic.library:structure.state_machine.concepts;

import std;

export namespace synodic::state_machine
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
	concept state_machine_storage = requires {
		typename S::state_descriptor;
		typename S::transition_descriptor;
		typename S::states_size_type;
		typename S::transitions_size_type;
		typename S::event_type;

		requires state_descriptor<typename S::state_descriptor>;
		requires transition_descriptor<typename S::transition_descriptor>;
		requires std::unsigned_integral<typename S::states_size_type>;
		requires std::unsigned_integral<typename S::transitions_size_type>;
		requires event_type<typename S::event_type>;
	};

	/**
	 * @brief Storage that provides state enumeration
	 */
	template<typename S>
	concept state_list_storage = state_machine_storage<S> and requires(const S& storage) {
		{ storage.states() } -> std::ranges::input_range;
		{ storage.num_states() } -> std::same_as<typename S::states_size_type>;
	};

	/**
	 * @brief Storage that provides transition enumeration
	 */
	template<typename S>
	concept transition_list_storage = state_machine_storage<S> and requires(const S& storage) {
		{ storage.transitions() } -> std::ranges::input_range;
		{ storage.num_transitions() } -> std::same_as<typename S::transitions_size_type>;
	};

	/**
	 * @brief Storage supporting outgoing transition traversal from a state
	 */
	template<typename S>
	concept incidence_storage =
		state_machine_storage<S> and
		requires(const S& storage, typename S::state_descriptor s, typename S::transition_descriptor t) {
			{ storage.out_transitions(s) } -> std::ranges::input_range;
			{ storage.out_degree(s) } -> std::unsigned_integral;
			{ storage.source(t) } -> std::same_as<typename S::state_descriptor>;
			{ storage.target(t) } -> std::same_as<typename S::state_descriptor>;
		};

	/**
	 * @brief Storage supporting state addition
	 */
	template<typename S>
	concept appendable_state_storage = state_machine_storage<S> and requires(S& storage) {
		{ storage.add_state() } -> std::same_as<typename S::state_descriptor>;
	};

	/**
	 * @brief Storage supporting transition addition
	 */
	template<typename S>
	concept appendable_transition_storage = state_machine_storage<S> and requires(
																			 S& storage,
																			 typename S::state_descriptor from,
																			 typename S::state_descriptor to,
																			 typename S::event_type event) {
		{ storage.add_transition(from, to, event) } -> std::same_as<std::pair<typename S::transition_descriptor, bool>>;
	};

	/**
	 * @brief Storage supporting state removal
	 */
	template<typename S>
	concept mutable_state_storage =
		appendable_state_storage<S> and requires(S& storage, typename S::state_descriptor s) {
			{ storage.remove_state(s) };
			{ storage.clear_states() };
		};

	/**
	 * @brief Storage supporting transition removal
	 */
	template<typename S>
	concept mutable_transition_storage =
		appendable_transition_storage<S> and requires(S& storage, typename S::transition_descriptor t) {
			{ storage.remove_transition(t) };
			{ storage.clear_transitions() };
		};

	/**
	 * @brief Fully mutable storage (add and remove states/transitions)
	 */
	template<typename S>
	concept mutable_storage = mutable_state_storage<S> and mutable_transition_storage<S>;

	/**
	 * @brief Append-only storage (add but no remove - like StaticStorage)
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
	concept state_property_storage = state_machine_storage<S> and requires { typename S::state_property_type; } and
									 requires(S& storage, const S& constStorage, typename S::state_descriptor s) {
										 { storage[s] } -> std::same_as<typename S::state_property_type&>;
										 { constStorage[s] } -> std::same_as<const typename S::state_property_type&>;
									 };

	/**
	 * @brief Storage providing O(1) transition property access
	 */
	template<typename S>
	concept transition_property_storage = state_machine_storage<S> and requires {
		typename S::transition_property_type;
	} and requires(S& storage, const S& constStorage, typename S::transition_descriptor t) {
		{ storage[t] } -> std::same_as<typename S::transition_property_type&>;
		{ constStorage[t] } -> std::same_as<const typename S::transition_property_type&>;
	};

	// ============================================================================
	// Compile-Time State Machine Concepts
	// ============================================================================

	/**
	 * @brief Storage that can be constructed and manipulated at compile-time
	 */
	template<typename S>
	concept constexpr_storage = state_machine_storage<S> and requires { requires S::is_constexpr_capable; };

	// ============================================================================
	// Event Matching Concepts
	// ============================================================================

	/**
	 * @brief Storage that supports event-based transition lookup
	 */
	template<typename S>
	concept event_queryable_storage =
		state_machine_storage<S> and incidence_storage<S> and
		requires(const S& storage, typename S::state_descriptor s, typename S::event_type event) {
			{ storage.find_transition(s, event) } -> std::same_as<std::optional<typename S::transition_descriptor>>;
		};

	// ============================================================================
	// Initial/Final State Concepts
	// ============================================================================

	/**
	 * @brief Storage that tracks initial state
	 */
	template<typename S>
	concept has_initial_state = state_machine_storage<S> and requires(const S& storage) {
		{ storage.initial_state() } -> std::same_as<std::optional<typename S::state_descriptor>>;
	} and requires(S& storage, typename S::state_descriptor s) {
		{ storage.set_initial_state(s) };
	};

	/**
	 * @brief Storage that tracks final/accepting states
	 */
	template<typename S>
	concept has_final_states = state_machine_storage<S> and requires(const S& storage, typename S::state_descriptor s) {
		{ storage.is_final_state(s) } -> std::same_as<bool>;
		{ storage.final_states() } -> std::ranges::input_range;
	} and requires(S& storage, typename S::state_descriptor s, bool is_final) {
		{ storage.set_final_state(s, is_final) };
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
