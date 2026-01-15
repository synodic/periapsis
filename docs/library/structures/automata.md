---
title: Automata
description: Constexpr-capable finite state machines
---

# Automata Library

A constexpr-capable, concept-based automata library supporting compile-time definition and runtime execution.

## Overview

The automata library is designed around three key principles:

1. **Unified Type** — `StateMachine<Event, StateData, TransData, MaxS, MaxT>` auto-selects storage
2. **Compile-Time Definition** — Supports constexpr FSMs for embedded systems
3. **Visitor Integration** — Runner satisfies `graph::visitor` concept for algorithm compatibility

## StateMachine

### Synopsis

```cpp
namespace peri::automata {
    template<
        typename Event,
        typename StateData = void,
        typename TransitionData = void,
        std::size_t MaxStates = 0,
        std::size_t MaxTransitions = 0
    >
    class StateMachine;
}
```

### Template Parameters

| Parameter | Description |
|-----------|-------------|
| `Event` | Type used to trigger transitions |
| `StateData` | Data stored per state (`void` for none) |
| `TransitionData` | Data stored per transition (`void` for none) |
| `MaxStates` | Maximum states (0 = dynamic) |
| `MaxTransitions` | Maximum transitions (0 = dynamic) |

### Storage Selection

- **Dynamic**: When `MaxStates = 0` or `MaxTransitions = 0`
- **Fixed**: When both `MaxStates > 0` and `MaxTransitions > 0`

Fixed storage enables full constexpr capability.

## Quick Start

### Dynamic StateMachine

```cpp
import synodic.periapsis;

using namespace peri::automata;

// Dynamic FSM with int events
StateMachine<int> sm;

auto locked = sm.add_state();
auto unlocked = sm.add_state();

sm.add_transition(locked, unlocked, 0);   // 0 = coin
sm.add_transition(unlocked, locked, 1);   // 1 = push

sm.set_initial_state(locked);
```

### Fixed-Size Constexpr StateMachine

```cpp
using namespace peri::automata;

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
static_assert(turnstile.num_transitions() == 4);
```

## StateMachine Operations

### State Operations

| Method | Returns | Description |
|--------|---------|-------------|
| `add_state()` | `state_descriptor` | Add state with default data |
| `add_state(data)` | `state_descriptor` | Add state with data |
| `operator[](s)` | `StateData&` | Access state data |
| `num_states()` | `size_type` | Number of states |
| `states()` | Range | Range of state descriptors |
| `set_initial_state(s)` | `void` | Set the initial state |
| `initial_state()` | `std::optional` | Get initial state |

### Transition Operations

| Method | Returns | Description |
|--------|---------|-------------|
| `add_transition(from, to, event)` | `transition_descriptor` | Add transition |
| `add_transition(from, to, event, data)` | `transition_descriptor` | Add transition with data |
| `operator[](t)` | `TransitionData&` | Access transition data |
| `num_transitions()` | `size_type` | Number of transitions |
| `transitions()` | Range | Range of transition descriptors |
| `source(t)` | `state_descriptor` | Source state of transition |
| `target(t)` | `state_descriptor` | Target state of transition |
| `event(t)` | `Event` | Event that triggers transition |

---

## Runner Class

The `Runner` class separates execution state from the FSM structure.

### Synopsis

```cpp
namespace peri::automata {
    template<typename StateMachine>
    class Runner;
}
```

### Design Philosophy

- **Structure** (`StateMachine`) — Defines states and transitions, can be constexpr/shared
- **Execution** (`Runner`) — Tracks current state, mutable, per-instance

This separation enables:

- One FSM structure, many runners
- Const/constexpr FSM with mutable execution
- Thread-safe sharing of FSM definition

### Construction

```cpp
constexpr auto fsm = make_turnstile();

// Create runner (starts at initial state)
Runner runner{fsm};

// Or specify starting state explicitly
Runner runner{fsm, some_state};
```

### Operations

| Method | Returns | Description |
|--------|---------|-------------|
| `start()` | `void` | Move to initial state |
| `current_state()` | `state_descriptor` | Current state |
| `process(event)` | `ProcessResult` | Process event |
| `position()` | `std::optional<state_descriptor>` | Current state (visitor API) |
| `is_positioned()` | `bool` | Has current state |
| `move_to(state)` | `void` | Jump to state |
| `reset()` | `void` | Clear position |

### Process Results

```cpp
enum class ProcessResult {
    Accepted,   // Transition taken
    Rejected,   // No matching transition
    NoPosition  // Runner not positioned
};
```

### Example

```cpp
import synodic.periapsis;

using namespace peri::automata;

constexpr auto fsm = make_turnstile();

Runner runner{fsm};
runner.start();  // Move to initial state (locked)

auto result = runner.process(0);  // Insert coin
// result == ProcessResult::Accepted
// runner.current_state() == unlocked

runner.process(1);  // Push through
// runner.current_state() == locked
```

### Multiple Runners, One Structure

```cpp
constexpr auto fsm = make_game_state_machine();

// Multiple game sessions sharing the same FSM definition
std::vector<Runner<decltype(fsm)>> sessions;

for (int i = 0; i < num_players; ++i) {
    sessions.emplace_back(fsm);
    sessions.back().start();
}

// Each runner tracks its own state independently
sessions[0].process(StartGameEvent{});
sessions[1].process(PauseEvent{});
```

---

## Visitor Concept Integration

The `Runner` class satisfies the `graph::visitor` concept, enabling unified algorithms:

```cpp
// Both satisfy visitor concept
graph::Visitor visitor{graph, start_vertex};
automata::Runner runner{fsm};

// Same interface
visitor.position();    runner.position();
visitor.move_to(v);    runner.move_to(state);
visitor.reset();       runner.reset();
```

This enables generic algorithms that work with both:

```cpp
template<graph::visitor V>
void dump_path(V& vis, auto&& on_node) {
    while (vis.is_positioned()) {
        on_node(vis.position().value());
        vis.advance(FirstEdgeSelector{});
    }
}

// Use with graph
dump_path(graph_visitor, [](auto v) { ... });

// Use with FSM
dump_path(runner, [](auto s) { ... });
```

---

## State Machine with Data

### State Data

Store data per state:

```cpp
struct StateData {
    std::string_view name;
    int value;
};

StateMachine<int, StateData> sm;
auto s1 = sm.add_state(StateData{"idle", 0});
auto s2 = sm.add_state(StateData{"running", 1});

// Access state data
std::cout << sm[s1].name;  // "idle"
```

### Transition Data

Store data per transition:

```cpp
struct TransData {
    std::string_view action;
};

StateMachine<int, void, TransData> sm;
auto s1 = sm.add_state();
auto s2 = sm.add_state();
auto t = sm.add_transition(s1, s2, 0, TransData{"start"});

// Access transition data
std::cout << sm[t].action;  // "start"
```

---

## Constexpr FSM Example

Complete compile-time FSM:

```cpp
import synodic.periapsis;

using namespace peri::automata;

enum class Event { COIN, PUSH };

constexpr auto create_turnstile() {
    StateMachine<Event, void, void, 2, 4> sm;

    auto locked = sm.add_state();
    auto unlocked = sm.add_state();

    sm.add_transition(locked, unlocked, Event::COIN);
    sm.add_transition(unlocked, locked, Event::PUSH);
    sm.add_transition(locked, locked, Event::PUSH);
    sm.add_transition(unlocked, unlocked, Event::COIN);

    sm.set_initial_state(locked);
    return sm;
}

// FSM evaluated at compile-time
constexpr auto turnstile = create_turnstile();

// Validate at compile-time
static_assert(turnstile.num_states() == 2);
static_assert(turnstile.num_transitions() == 4);
static_assert(turnstile.initial_state().has_value());

// Use at runtime
int main() {
    Runner runner{turnstile};
    runner.start();
    runner.process(Event::COIN);  // locked -> unlocked
    runner.process(Event::PUSH);  // unlocked -> locked
}
```
