/**
 * @brief Unit tests for state machine
 */

#include <gtest/gtest.h>

import std;
import synodic.periapsis;

namespace
{
	struct StateData
	{
		int value = 0;

		constexpr bool operator==(const StateData&) const = default;
	};

	struct TransitionData
	{
		int priority = 0;

		constexpr bool operator==(const TransitionData&) const = default;
	};

	// Event types for typed state machine
	enum class Event
	{
		Start,
		Stop,
		Pause,
		Resume,
		Error
	};

	// ============================================================================
	// Static Storage Tests (compile-time state machines)
	// ============================================================================

	TEST(StateMachineStaticStorage, ConstexprConstruction)
	{
		using namespace peri::state_machine;
		using namespace peri::state_machine::storage;

		// Verify constexpr construction works
		constexpr auto make_sm = []() constexpr
		{
			StaticStorage<int, void, void, 4, 8> storage;
			auto s0 = storage.add_state();
			auto s1 = storage.add_state();
			auto s2 = storage.add_state();
			storage.add_transition(s0, s1, 1);
			storage.add_transition(s1, s2, 2);
			storage.add_transition(s2, s0, 3);
			storage.set_initial_state(s0);
			storage.set_final_state(s2);
			return storage;
		};

		constexpr auto storage = make_sm();

		static_assert(storage.num_states() == 3);
		static_assert(storage.num_transitions() == 3);
		static_assert(storage.initial_state().has_value());
		static_assert(*storage.initial_state() == 0);
		static_assert(storage.is_final_state(2));

		EXPECT_EQ(storage.num_states(), 3);
		EXPECT_EQ(storage.num_transitions(), 3);
	}

	TEST(StateMachineStaticStorage, StateProperties)
	{
		using namespace peri::state_machine::storage;

		StaticStorage<int, StateData, void, 4, 4> storage;
		auto s0 = storage.add_state(StateData {10});
		auto s1 = storage.add_state(StateData {20});

		EXPECT_EQ(storage.state_data(s0).value, 10);
		EXPECT_EQ(storage.state_data(s1).value, 20);
	}

	TEST(StateMachineStaticStorage, TransitionLookup)
	{
		using namespace peri::state_machine::storage;

		constexpr auto make_sm = []() constexpr
		{
			StaticStorage<int, void, void, 4, 8> storage;
			auto s0 = storage.add_state();
			auto s1 = storage.add_state();
			auto s2 = storage.add_state();

			storage.add_transition(s0, s1, 1);
			storage.add_transition(s0, s2, 2);
			storage.add_transition(s1, s2, 3);

			return storage;
		};

		constexpr auto storage = make_sm();

		// Test transition lookup
		constexpr auto t1	  = storage.find_transition(0, 1);
		constexpr auto t2	  = storage.find_transition(0, 2);
		constexpr auto t_none = storage.find_transition(0, 99);

		static_assert(t1.has_value());
		static_assert(t2.has_value());
		static_assert(!t_none.has_value());

		// Verify targets
		static_assert(storage.target(*t1) == 1);
		static_assert(storage.target(*t2) == 2);

		EXPECT_TRUE(t1.has_value());
		EXPECT_EQ(storage.target(*t1), 1);
	}

	TEST(StateMachineStaticStorage, InitialFinalStates)
	{
		using namespace peri::state_machine::storage;

		StaticStorage<int, void, void, 4, 4> storage;
		auto s0 = storage.add_state();
		auto s1 = storage.add_state();
		auto s2 = storage.add_state();

		storage.set_initial_state(s0);
		storage.set_final_state(s1);
		storage.set_final_state(s2);

		EXPECT_EQ(storage.initial_state(), s0);
		EXPECT_TRUE(storage.is_final_state(s1));
		EXPECT_TRUE(storage.is_final_state(s2));
		EXPECT_FALSE(storage.is_final_state(s0));
	}

	// ============================================================================
	// State Machine Class Tests
	// ============================================================================

	TEST(StateMachine, ConstexprStateMachine)
	{
		using namespace peri::state_machine;

		// Build a complete constexpr state machine
		constexpr auto make_turnstile = []() constexpr
		{
			SimpleStaticStateMachine<2, 4> sm;

			// States: Locked (0), Unlocked (1)
			auto locked	  = sm.add_state();
			auto unlocked = sm.add_state();

			// Events: 0 = coin, 1 = push
			sm.add_transition(locked, unlocked, 0);	   // coin unlocks
			sm.add_transition(unlocked, locked, 1);	   // push locks
			sm.add_transition(locked, locked, 1);	   // push while locked stays locked
			sm.add_transition(unlocked, unlocked, 0);  // coin while unlocked stays unlocked

			sm.set_initial_state(locked);

			return sm;
		};

		constexpr auto turnstile = make_turnstile();

		static_assert(turnstile.num_states() == 2);
		static_assert(turnstile.num_transitions() == 4);
		static_assert(turnstile.initial_state() == 0);

		EXPECT_EQ(turnstile.num_states(), 2);
		EXPECT_EQ(turnstile.num_transitions(), 4);
	}

	TEST(StateMachine, ConstexprTraversal)
	{
		using namespace peri::state_machine;

		// Build and traverse a state machine entirely at compile-time
		constexpr auto traverse_turnstile = []() constexpr
		{
			SimpleStaticStateMachine<2, 4> sm;

			auto locked	  = sm.add_state();
			auto unlocked = sm.add_state();

			// Events: 0 = coin, 1 = push
			sm.add_transition(locked, unlocked, 0);	   // coin unlocks
			sm.add_transition(unlocked, locked, 1);	   // push locks
			sm.add_transition(locked, locked, 1);	   // push while locked stays locked
			sm.add_transition(unlocked, unlocked, 0);  // coin while unlocked stays unlocked

			sm.set_initial_state(locked);

			// Create runner and traverse at compile-time
			peri::state_machine::Runner runner {sm};
			runner.start();

			// Initial state should be locked (0)
			auto state0 = runner.current_state();
			if (state0 != locked)
			{
				return false;
			}

			// Insert coin -> should unlock
			auto r1		= runner.process(0);
			auto state1 = runner.current_state();
			if (r1 != ProcessResult::Accepted || state1 != unlocked)
			{
				return false;
			}

			// Push -> should lock
			auto r2		= runner.process(1);
			auto state2 = runner.current_state();
			if (r2 != ProcessResult::Accepted || state2 != locked)
			{
				return false;
			}

			// Push while locked -> stays locked
			auto r3		= runner.process(1);
			auto state3 = runner.current_state();
			if (r3 != ProcessResult::Accepted || state3 != locked)
			{
				return false;
			}

			// Invalid event -> rejected
			auto r4 = runner.process(99);
			if (r4 != ProcessResult::Rejected)
			{
				return false;
			}

			return true;
		};

		// This static_assert proves the entire traversal happens at compile-time
		static_assert(traverse_turnstile(), "Constexpr traversal failed");

		EXPECT_TRUE(traverse_turnstile());
	}

	TEST(StateMachine, RuntimeEventProcessing)
	{
		using namespace peri::state_machine;

		// Create a simple traffic light state machine
		SimpleStaticStateMachine<3, 3> sm;

		auto red	= sm.add_state();
		auto yellow = sm.add_state();
		auto green	= sm.add_state();

		// Events: 1 = next
		sm.add_transition(red, green, 1);
		sm.add_transition(green, yellow, 1);
		sm.add_transition(yellow, red, 1);

		sm.set_initial_state(red);

		// Use Runner for execution
		peri::state_machine::Runner runner {sm};
		runner.start();

		EXPECT_EQ(runner.current_state(), red);

		auto result1 = runner.process(1);
		EXPECT_EQ(result1, ProcessResult::Accepted);
		EXPECT_EQ(runner.current_state(), green);

		auto result2 = runner.process(1);
		EXPECT_EQ(result2, ProcessResult::Accepted);
		EXPECT_EQ(runner.current_state(), yellow);

		auto result3 = runner.process(1);
		EXPECT_EQ(result3, ProcessResult::Accepted);
		EXPECT_EQ(runner.current_state(), red);

		// Invalid event should be rejected
		auto result4 = runner.process(99);
		EXPECT_EQ(result4, ProcessResult::Rejected);
		EXPECT_EQ(runner.current_state(), red);	 // State unchanged
	}

	TEST(StateMachine, FinalStateBehavior)
	{
		using namespace peri::state_machine;

		SimpleStaticStateMachine<3, 2> sm;

		auto start	= sm.add_state();
		auto middle = sm.add_state();
		auto end	= sm.add_state();

		sm.add_transition(start, middle, 1);
		sm.add_transition(middle, end, 2);

		sm.set_initial_state(start);
		sm.set_final_state(end);

		peri::state_machine::Runner runner {sm};
		runner.start();
		runner.process(1);	// start -> middle
		runner.process(2);	// middle -> end

		EXPECT_TRUE(runner.is_in_final_state());

		// Processing events in final state should return InFinalState
		auto result = runner.process(1);
		EXPECT_EQ(result, ProcessResult::InFinalState);
	}

	TEST(StateMachine, GuardFunctions)
	{
		using namespace peri::state_machine;

		SimpleStaticStateMachine<2, 2> sm;

		auto s0 = sm.add_state();
		auto s1 = sm.add_state();

		sm.add_transition(s0, s1, 1);
		sm.add_transition(s1, s0, 1);

		sm.set_initial_state(s0);

		peri::state_machine::Runner runner {sm};
		runner.start();

		int guard_call_count = 0;

		// Guard that always returns false
		auto result1 = runner.process(
			1,
			[&guard_call_count](auto)
			{
				++guard_call_count;
				return false;
			});

		EXPECT_EQ(result1, ProcessResult::GuardFailed);
		EXPECT_EQ(runner.current_state(), s0);	// State unchanged
		EXPECT_EQ(guard_call_count, 1);

		// Guard that returns true
		auto result2 = runner.process(
			1,
			[&guard_call_count](auto)
			{
				++guard_call_count;
				return true;
			});

		EXPECT_EQ(result2, ProcessResult::Accepted);
		EXPECT_EQ(runner.current_state(), s1);
		EXPECT_EQ(guard_call_count, 2);
	}

	TEST(StateMachine, GuardAndAction)
	{
		using namespace peri::state_machine;

		SimpleStaticStateMachine<2, 1> sm;

		auto s0 = sm.add_state();
		auto s1 = sm.add_state();

		sm.add_transition(s0, s1, 1);
		sm.set_initial_state(s0);

		peri::state_machine::Runner runner {sm};
		runner.start();

		bool action_called = false;

		auto result = runner.process(
			1,
			[](auto)
			{
				return true;
			},						// Guard
			[&action_called](auto)	// Action
			{
				action_called = true;
			});

		EXPECT_EQ(result, ProcessResult::Accepted);
		EXPECT_TRUE(action_called);
		EXPECT_EQ(runner.current_state(), s1);
	}

	TEST(StateMachine, CanProcessCheck)
	{
		using namespace peri::state_machine;

		SimpleStaticStateMachine<2, 1> sm;

		auto s0 = sm.add_state();
		auto s1 = sm.add_state();

		sm.add_transition(s0, s1, 1);
		sm.set_initial_state(s0);

		peri::state_machine::Runner runner {sm};
		runner.start();

		EXPECT_TRUE(runner.can_process(1));
		EXPECT_FALSE(runner.can_process(2));

		runner.process(1);

		// From s1, no transitions defined
		EXPECT_FALSE(runner.can_process(1));
	}

	TEST(StateMachine, NotStartedBehavior)
	{
		using namespace peri::state_machine;

		SimpleStaticStateMachine<2, 1> sm;

		auto s0 = sm.add_state();
		auto s1 = sm.add_state();

		sm.add_transition(s0, s1, 1);
		sm.set_initial_state(s0);

		// Create runner but don't call start()
		peri::state_machine::Runner runner {sm};
		EXPECT_FALSE(runner.is_running());

		auto result = runner.process(1);
		EXPECT_EQ(result, ProcessResult::NoCurrentState);
	}

	// ============================================================================
	// Dynamic Storage Tests (runtime modifications)
	// ============================================================================

	TEST(StateMachineDynamicStorage, BasicConstruction)
	{
		using namespace peri::state_machine;

		DynamicStateMachine<int> sm;

		auto s0 = sm.add_state();
		auto s1 = sm.add_state();
		auto s2 = sm.add_state();

		sm.add_transition(s0, s1, 1);
		sm.add_transition(s1, s2, 2);
		sm.add_transition(s2, s0, 3);

		EXPECT_EQ(sm.num_states(), 3);
		EXPECT_EQ(sm.num_transitions(), 3);
	}

	TEST(StateMachineDynamicStorage, RuntimeStateRemoval)
	{
		using namespace peri::state_machine;

		DynamicStateMachine<int> sm;

		auto s0 = sm.add_state();
		auto s1 = sm.add_state();
		auto s2 = sm.add_state();

		sm.add_transition(s0, s1, 1);
		sm.add_transition(s1, s2, 2);

		EXPECT_EQ(sm.num_states(), 3);

		sm.remove_state(s1);

		EXPECT_EQ(sm.num_states(), 2);
		// Transitions involving s1 should be removed
		EXPECT_EQ(sm.num_transitions(), 0);
	}

	TEST(StateMachineDynamicStorage, RuntimeTransitionRemoval)
	{
		using namespace peri::state_machine;

		DynamicStateMachine<int> sm;

		auto s0 = sm.add_state();
		auto s1 = sm.add_state();

		auto [t1, success1] = sm.add_transition(s0, s1, 1);
		auto [t2, success2] = sm.add_transition(s0, s1, 2);

		EXPECT_EQ(sm.num_transitions(), 2);

		sm.remove_transition(t1);

		EXPECT_EQ(sm.num_transitions(), 1);
	}

	TEST(StateMachineDynamicStorage, StateDataAccess)
	{
		using namespace peri::state_machine;

		DynamicStateMachine<int, StateData> sm;

		auto s0 = sm.add_state(StateData {42});
		auto s1 = sm.add_state(StateData {100});

		EXPECT_EQ(sm.storage().state_data(s0).value, 42);
		EXPECT_EQ(sm.storage().state_data(s1).value, 100);

		// Modify state data
		sm.storage().state_data(s0).value = 999;
		EXPECT_EQ(sm.storage().state_data(s0).value, 999);
	}

	TEST(StateMachineDynamicStorage, TypedEvents)
	{
		using namespace peri::state_machine;

		DynamicStateMachine<Event, StateData> sm;

		auto idle	 = sm.add_state(StateData {0});
		auto running = sm.add_state(StateData {1});
		auto paused	 = sm.add_state(StateData {2});

		sm.add_transition(idle, running, Event::Start);
		sm.add_transition(running, paused, Event::Pause);
		sm.add_transition(paused, running, Event::Resume);
		sm.add_transition(running, idle, Event::Stop);
		sm.add_transition(paused, idle, Event::Stop);

		sm.set_initial_state(idle);

		peri::state_machine::Runner runner {sm};
		runner.start();

		EXPECT_EQ(runner.current_state(), idle);

		runner.process(Event::Start);
		EXPECT_EQ(runner.current_state(), running);

		runner.process(Event::Pause);
		EXPECT_EQ(runner.current_state(), paused);

		runner.process(Event::Resume);
		EXPECT_EQ(runner.current_state(), running);

		runner.process(Event::Stop);
		EXPECT_EQ(runner.current_state(), idle);
	}
}
