# State Machine Library

A constexpr-capable, concept-based state machine library supporting compile-time definition and runtime execution with structure/execution separation.

## Overview

The state machine library is designed around three key principles:

1. **Unified Type** - `StateMachine<Event,StateData,TransData,MaxS,MaxT>` auto-selects storage
2. **Compile-Time Definition** - Supports `constexpr` state machines for embedded systems
3. **Visitor Integration** - Runner satisfies `graph::visitor` concept for algorithm compatibility

## Relationship to Graph Library

State machines are graphs with FSM semantics. The `Runner` class implements the `graph::visitor` concept, enabling:

- **Unified traversal** - Same algorithms work on graphs and FSMs
- **Event-driven navigation** - `Runner` is an `event_visitor` using events as edge selectors
- **Shared vocabulary** - `AdvanceResult` and position tracking work identically

```cpp
// Both satisfy visitor concept
graph::Visitor<Graph>  visitor{graph, start_vertex};
state_machine::Runner  runner{fsm};

// Same traversal interface
visitor.position();    runner.position();
visitor.move_to(v);    runner.move_to(state);
visitor.reset();       runner.reset();
visitor.advance();     runner.advance(event);
```

## Quick Start

### Dynamic State Machine (Default)

```cpp
import synodic.library;

using namespace synodic::state_machine;

// Dynamic state machine - growable at runtime
StateMachine<int> sm;  // int events, no state/transition data

auto locked = sm.add_state();
auto unlocked = sm.add_state();

sm.add_transition(locked, unlocked, 0);   // coin unlocks
sm.add_transition(unlocked, locked, 1);   // push locks

sm.set_initial_state(locked);
```

### Fixed-Size Constexpr State Machine

```cpp
using namespace synodic::state_machine;

constexpr auto make_turnstile() {
    StateMachine<int, void, void, 2, 4> sm;  // Max 2 states, 4 transitions

    auto locked = sm.add_state();
    auto unlocked = sm.add_state();

    sm.add_transition(locked, unlocked, 0);   // coin
    sm.add_transition(unlocked, locked, 1);   // push
    sm.add_transition(locked, locked, 1);     // push while locked
    sm.add_transition(unlocked, unlocked, 0); // coin while unlocked

    sm.set_initial_state(locked);
    return sm;
}

constexpr auto turnstile = make_turnstile();
static_assert(turnstile.num_states() == 2);
```

### Runner: Execution State

The `Runner` class separates execution state from structure:

```cpp
// Structure (can be constexpr, shared, immutable)
constexpr auto fsm = make_turnstile();

// Execution state (mutable, per-instance)
Runner runner{fsm};
runner.start();

auto result = runner.process(0);  // Insert coin
assert(result == ProcessResult::Accepted);
assert(runner.current_state() == 1);  // unlocked

runner.process(1);  // Push through
assert(runner.current_state() == 0);  // locked again
```

### Multiple Runners, One Structure

```cpp
const auto shared_fsm = make_fsm();

// Each runner has independent execution state
Runner runner1{shared_fsm};
Runner runner2{shared_fsm};

runner1.start();
runner2.start();

runner1.process(Event::A);  // Doesn't affect runner2
runner2.process(Event::B);  // Doesn't affect runner1
```

### Seeding: Constexpr to Dynamic

```cpp
// Start with compile-time structure
constexpr auto seed = make_turnstile();

// Seed a dynamic state machine
StateMachine<int> dynamic{seed};

// Add more at runtime
auto new_state = dynamic.add_state();
dynamic.add_transition(1, new_state, 2);
```

## Guards and Actions

```cpp
Runner runner{fsm};
runner.start();

int counter = 0;

auto result = runner.process(
    1,
    // Guard: only allow if condition met
    [&](auto transition) { return counter < 10; },
    // Action: executed after transition
    [&](auto transition) { ++counter; }
);

if (result == ProcessResult::GuardFailed) {
    // Transition exists but guard returned false
}
```

## ProcessResult Enum

```cpp
enum class ProcessResult {
    Accepted,       // Transition taken successfully
    Rejected,       // No valid transition for event
    GuardFailed,    // Guard returned false
    NoCurrentState, // Runner not started
    InFinalState    // In a final state
};
```

## Storage Selection

The unified `StateMachine` type automatically selects storage:

| Template Args | Storage Type | Capabilities |
|--------------|--------------|--------------|
| `StateMachine<E>` | DynamicStorage | Growable |
| `StateMachine<E,S,T,MaxS,MaxT>` | StaticStorage | Constexpr, fixed |

## Type Aliases

```cpp
// Dynamic state machine (default)
DynamicStateMachine<Event, StateData, TransitionData>

// Fixed-size constexpr state machine
FixedStateMachine<Event, StateData, TransitionData, MaxS, MaxT>

// Simple integer-event state machines
SimpleStaticStateMachine<MaxStates, MaxTransitions>
SimpleDynamicStateMachine
```

## API Reference

### StateMachine Class (Structure Only)

```cpp
template<typename Event, typename StateData, typename TransData,
         std::size_t MaxStates, std::size_t MaxTransitions>
class StateMachine {
    // State operations
    constexpr auto add_state() -> state_descriptor;
    constexpr auto add_state(Args&&... data) -> state_descriptor;
    constexpr auto num_states() const -> states_size_type;
    constexpr auto states() const;  // Range

    // Transition operations
    constexpr auto add_transition(from, to, event) -> pair<descriptor, bool>;
    constexpr auto num_transitions() const -> transitions_size_type;
    constexpr auto transitions() const;  // Range

    // Incidence queries
    constexpr auto out_transitions(state) const;
    constexpr auto source(transition) const -> state_descriptor;
    constexpr auto target(transition) const -> state_descriptor;
    constexpr auto transition_event(transition) const -> const Event&;
    constexpr auto find_transition(state, event) const -> optional<transition>;

    // Initial/Final states
    constexpr void set_initial_state(state);
    constexpr auto initial_state() const -> optional<state_descriptor>;
    constexpr void set_final_state(state, bool = true);
    constexpr auto is_final_state(state) const -> bool;

    // Property access
    constexpr auto operator[](state) -> StateData&;
    constexpr auto operator[](transition) -> TransData&;
};
```

### Runner Class (Execution State)

Runner satisfies the `graph::visitor` concept, enabling interoperability with graph algorithms:

```cpp
template<typename StateMachineType>
class Runner {
    // Type aliases
    using vertex_descriptor = state_descriptor;  // Visitor compatibility

    // Construction
    explicit Runner(StateMachineType& machine);
    Runner(StateMachineType& machine, state_descriptor start);

    // State query (original API)
    auto current_state() const -> optional<state_descriptor>;
    auto is_running() const -> bool;
    auto is_in_final_state() const -> bool;

    // Visitor interface (graph::visitor concept)
    auto position() const -> optional<vertex_descriptor>;  // Same as current_state()
    auto is_positioned() const -> bool;                    // Same as is_running()
    void move_to(vertex_descriptor state);                 // Direct state assignment
    void reset();                                          // Clear position

    // Lifecycle
    auto start() -> bool;  // From initial state
    void start(state_descriptor);

    // Event processing
    auto process(event) -> ProcessResult;
    auto process(event, Guard) -> ProcessResult;
    auto process(event, Guard, Action) -> ProcessResult;

    // Advance (visitor-style)
    auto advance(event) -> AdvanceResult;
    auto advance(event, Guard) -> AdvanceResult;

    // Query
    auto can_process(event) const -> bool;
    auto available_events() const;  // Range
    auto available_transitions() const;  // Range

    // Graph access
    auto& machine() const;  // Access underlying state machine
};

// Static assertions
static_assert(graph::visitor<Runner<StateMachine<int>>>);
```

### Visitor Integration

Runner's visitor interface enables generic algorithm usage:

```cpp
// Generic visitor algorithm works with both Visitor and Runner
template<graph::visitor V>
void explore(V& vis) {
    while (vis.is_positioned()) {
        process(vis.position().value());
        // ... advance logic ...
    }
}

// Use with FSM
StateMachine<Event> fsm = make_fsm();
Runner runner{fsm};
runner.start();

// Direct use
explore(runner);
```

## Concepts Hierarchy

```
state_machine_storage (base)
├── state_list_storage           - states(), num_states()
├── transition_list_storage      - transitions(), num_transitions()
├── incidence_storage            - out_transitions(), source(), target()
├── appendable_state_storage     - add_state()
├── appendable_transition_storage - add_transition()
├── mutable_state_storage        - remove_state(), clear_states()
├── mutable_transition_storage   - remove_transition(), clear_transitions()
├── state_property_storage       - operator[](state)
├── transition_property_storage  - operator[](transition)
├── event_queryable_storage      - find_transition(state, event)
├── has_initial_state            - initial_state(), set_initial_state()
└── has_final_states             - is_final_state(), final_states()
```

## Example: Traffic Light Controller

```cpp
import synodic.library;

using namespace synodic::state_machine;

enum class Event { Timer, Emergency, Reset };

struct LightState {
    std::string_view name;
    int duration_ms;
};

constexpr auto make_traffic_light() {
    StateMachine<Event, LightState, void, 4, 7> sm;

    auto red = sm.add_state(LightState{"Red", 30000});
    auto yellow = sm.add_state(LightState{"Yellow", 5000});
    auto green = sm.add_state(LightState{"Green", 25000});
    auto emergency = sm.add_state(LightState{"Emergency", 0});

    sm.add_transition(red, green, Event::Timer);
    sm.add_transition(green, yellow, Event::Timer);
    sm.add_transition(yellow, red, Event::Timer);
    sm.add_transition(red, emergency, Event::Emergency);
    sm.add_transition(green, emergency, Event::Emergency);
    sm.add_transition(yellow, emergency, Event::Emergency);
    sm.add_transition(emergency, red, Event::Reset);

    sm.set_initial_state(red);
    sm.set_final_state(emergency);

    return sm;
}

// Use at compile-time
constexpr auto light = make_traffic_light();
static_assert(light.num_states() == 4);

// Use at runtime with Runner
void run_controller() {
    const auto fsm = make_traffic_light();
    Runner runner{fsm};
    runner.start();

    while (!runner.is_in_final_state()) {
        // Process timer events...
        runner.process(Event::Timer);
    }
}
```

## Constants

```cpp
// Sentinel for dynamic storage selection
inline constexpr std::size_t dynamic_extent;
```
