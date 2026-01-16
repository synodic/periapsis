---
title: Automata
description: Constexpr-capable finite state machines
---

# Automata

Finite state machines (automata) in Periapsis are designed for compile-time definition and runtime execution.

## StateMachine

## Runner

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
