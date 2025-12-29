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
		START,
		STOP,
		PAUSE,
		RESUME,
		ERROR
	};

	// ============================================================================
	// Static Storage Tests (compile-time state machines)
	// ============================================================================

	TEST(StateMachineStaticStorage, ConstexprConstruction)
	{
		using namespace peri::automata;
		using namespace peri::automata::storage;

		// Verify constexpr construction works
		constexpr auto make_sm = []() constexpr
		{
			StaticStorage<int, void, void, 4, 8> storage;
			auto s0 = storage.AddState();
			auto s1 = storage.AddState();
			auto s2 = storage.AddState();
			storage.AddTransition(s0, s1, 1);
			storage.AddTransition(s1, s2, 2);
			storage.AddTransition(s2, s0, 3);
			storage.SetInitialState(s0);
			storage.SetFinalState(s2);
			return storage;
		};

		constexpr auto storage = make_sm();

		static_assert(storage.NumStates() == 3);
		static_assert(storage.NumTransitions() == 3);
		static_assert(storage.InitialState().has_value());
		static_assert(*storage.InitialState() == 0);
		static_assert(storage.IsFinalState(2));

		EXPECT_EQ(storage.NumStates(), 3);
		EXPECT_EQ(storage.NumTransitions(), 3);
	}

	TEST(StateMachineStaticStorage, StateProperties)
	{
		using namespace peri::automata::storage;

		StaticStorage<int, StateData, void, 4, 4> storage;
		auto s0 = storage.AddState(StateData {10});
		auto s1 = storage.AddState(StateData {20});

			EXPECT_EQ(storage.GetStateData(s0).value, 10);
			EXPECT_EQ(storage.GetStateData(s1).value, 20);
		}

	TEST(StateMachineStaticStorage, TransitionLookup)
	{
		using namespace peri::automata::storage;

		constexpr auto make_sm = []() constexpr
		{
			StaticStorage<int, void, void, 4, 8> storage;
			auto s0 = storage.AddState();
			auto s1 = storage.AddState();
			auto s2 = storage.AddState();

			storage.AddTransition(s0, s1, 1);
			storage.AddTransition(s0, s2, 2);
			storage.AddTransition(s1, s2, 3);

			return storage;
		};

		constexpr auto storage = make_sm();

		// Test transition lookup
		constexpr auto t1	  = storage.FindTransition(0, 1);
		constexpr auto t2	  = storage.FindTransition(0, 2);
		constexpr auto t_none = storage.FindTransition(0, 99);

		static_assert(t1.has_value());
		static_assert(t2.has_value());
		static_assert(!t_none.has_value());

		// Verify targets
		static_assert(storage.Target(*t1) == 1);
		static_assert(storage.Target(*t2) == 2);

		EXPECT_TRUE(t1.has_value());
		EXPECT_EQ(storage.Target(*t1), 1);
	}

	TEST(StateMachineStaticStorage, InitialFinalStates)
	{
		using namespace peri::automata::storage;

		StaticStorage<int, void, void, 4, 4> storage;
		auto s0 = storage.AddState();
		auto s1 = storage.AddState();
		auto s2 = storage.AddState();

		storage.SetInitialState(s0);
		storage.SetFinalState(s1);
		storage.SetFinalState(s2);

		EXPECT_EQ(storage.InitialState(), s0);
		EXPECT_TRUE(storage.IsFinalState(s1));
		EXPECT_TRUE(storage.IsFinalState(s2));
		EXPECT_FALSE(storage.IsFinalState(s0));
	}

	// ============================================================================
	// State Machine Class Tests
	// ============================================================================

	TEST(StateMachine, ConstexprStateMachine)
	{
		using namespace peri::automata;

		// Build a complete constexpr state machine
		constexpr auto make_turnstile = []() constexpr
		{
			SimpleStaticStateMachine<2, 4> sm;

			// States: Locked (0), Unlocked (1)
			auto locked	  = sm.AddState();
			auto unlocked = sm.AddState();

			// Events: 0 = coin, 1 = push
			sm.AddTransition(locked, unlocked, 0);	   // coin unlocks
			sm.AddTransition(unlocked, locked, 1);	   // push locks
			sm.AddTransition(locked, locked, 1);	   // push while locked stays locked
			sm.AddTransition(unlocked, unlocked, 0);  // coin while unlocked stays unlocked

			sm.SetInitialState(locked);

			return sm;
		};

		constexpr auto turnstile = make_turnstile();

		static_assert(turnstile.NumStates() == 2);
		static_assert(turnstile.NumTransitions() == 4);
		static_assert(turnstile.InitialState() == 0);

		EXPECT_EQ(turnstile.NumStates(), 2);
		EXPECT_EQ(turnstile.NumTransitions(), 4);
	}

	TEST(StateMachine, ConstexprTraversal)
	{
		using namespace peri::automata;

		// Build and traverse a state machine entirely at compile-time
		constexpr auto traverse_turnstile = []() constexpr
		{
			SimpleStaticStateMachine<2, 4> sm;

			auto locked	  = sm.AddState();
			auto unlocked = sm.AddState();

			// Events: 0 = coin, 1 = push
			sm.AddTransition(locked, unlocked, 0);	   // coin unlocks
			sm.AddTransition(unlocked, locked, 1);	   // push locks
			sm.AddTransition(locked, locked, 1);	   // push while locked stays locked
			sm.AddTransition(unlocked, unlocked, 0);  // coin while unlocked stays unlocked

			sm.SetInitialState(locked);

			// Create runner and traverse at compile-time
			peri::automata::Runner runner {sm};
			runner.Start();

				// Initial state should be locked (0)
				auto state0 = runner.CurrentState();
				if (state0 != locked)
				{
					return false;
				}

				// Insert coin -> should unlock
				auto r1		= runner.Process(0);
				auto state1 = runner.CurrentState();
				if (r1 != ProcessResult::ACCEPTED || state1 != unlocked)
				{
					return false;
				}

				// Push -> should lock
				auto r2		= runner.Process(1);
				auto state2 = runner.CurrentState();
				if (r2 != ProcessResult::ACCEPTED || state2 != locked)
				{
					return false;
				}

				// Push while locked -> stays locked
				auto r3		= runner.Process(1);
				auto state3 = runner.CurrentState();
				if (r3 != ProcessResult::ACCEPTED || state3 != locked)
				{
					return false;
				}

				// Invalid event -> rejected
				auto r4 = runner.Process(99);
				if (r4 != ProcessResult::REJECTED)
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
		using namespace peri::automata;

		// Create a simple traffic light state machine
		SimpleStaticStateMachine<3, 3> sm;

		auto red	= sm.AddState();
		auto yellow = sm.AddState();
		auto green	= sm.AddState();

		// Events: 1 = next
		sm.AddTransition(red, green, 1);
		sm.AddTransition(green, yellow, 1);
		sm.AddTransition(yellow, red, 1);

		sm.SetInitialState(red);

		// Use Runner for execution
		peri::automata::Runner runner {sm};
		runner.Start();

		EXPECT_EQ(runner.CurrentState(), red);

		auto result1 = runner.Process(1);
		EXPECT_EQ(result1, ProcessResult::ACCEPTED);
		EXPECT_EQ(runner.CurrentState(), green);

		auto result2 = runner.Process(1);
		EXPECT_EQ(result2, ProcessResult::ACCEPTED);
		EXPECT_EQ(runner.CurrentState(), yellow);

		auto result3 = runner.Process(1);
		EXPECT_EQ(result3, ProcessResult::ACCEPTED);
		EXPECT_EQ(runner.CurrentState(), red);

		// Invalid event should be rejected
		auto result4 = runner.Process(99);
		EXPECT_EQ(result4, ProcessResult::REJECTED);
		EXPECT_EQ(runner.CurrentState(), red);	 // State unchanged
	}

	TEST(StateMachine, FinalStateBehavior)
	{
		using namespace peri::automata;

		SimpleStaticStateMachine<3, 2> sm;

		auto start	= sm.AddState();
		auto middle = sm.AddState();
		auto end	= sm.AddState();

		sm.AddTransition(start, middle, 1);
		sm.AddTransition(middle, end, 2);

		sm.SetInitialState(start);
		sm.SetFinalState(end);

		peri::automata::Runner runner {sm};
		runner.Start();
		runner.Process(1);	// start -> middle
		runner.Process(2);	// middle -> end

		EXPECT_TRUE(runner.IsInFinalState());

		// Processing events in final state should return InFinalState
		auto result = runner.Process(1);
		EXPECT_EQ(result, ProcessResult::IN_FINAL_STATE);
	}

	TEST(StateMachine, GuardFunctions)
	{
		using namespace peri::automata;

		SimpleStaticStateMachine<2, 2> sm;

		auto s0 = sm.AddState();
		auto s1 = sm.AddState();

		sm.AddTransition(s0, s1, 1);
		sm.AddTransition(s1, s0, 1);

		sm.SetInitialState(s0);

		peri::automata::Runner runner {sm};
		runner.Start();

		int guard_call_count = 0;

		// Guard that always returns false
		auto result1 = runner.Process(
			1,
			[&guard_call_count](auto)
			{
				++guard_call_count;
				return false;
			});

		EXPECT_EQ(result1, ProcessResult::GUARD_FAILED);
		EXPECT_EQ(runner.CurrentState(), s0);	// State unchanged
		EXPECT_EQ(guard_call_count, 1);

		// Guard that returns true
		auto result2 = runner.Process(
			1,
			[&guard_call_count](auto)
			{
				++guard_call_count;
				return true;
			});

		EXPECT_EQ(result2, ProcessResult::ACCEPTED);
		EXPECT_EQ(runner.CurrentState(), s1);
		EXPECT_EQ(guard_call_count, 2);
	}

	TEST(StateMachine, GuardAndAction)
	{
		using namespace peri::automata;

		SimpleStaticStateMachine<2, 1> sm;

		auto s0 = sm.AddState();
		auto s1 = sm.AddState();

		sm.AddTransition(s0, s1, 1);
		sm.SetInitialState(s0);

		peri::automata::Runner runner {sm};
		runner.Start();

		bool action_called = false;

		auto result = runner.Process(
			1,
			[](auto)
			{
				return true;
			},						// Guard
			[&action_called](auto)	// Action
			{
				action_called = true;
			});

		EXPECT_EQ(result, ProcessResult::ACCEPTED);
		EXPECT_TRUE(action_called);
		EXPECT_EQ(runner.CurrentState(), s1);
	}

	TEST(StateMachine, CanProcessCheck)
	{
		using namespace peri::automata;

		SimpleStaticStateMachine<2, 1> sm;

		auto s0 = sm.AddState();
		auto s1 = sm.AddState();

		sm.AddTransition(s0, s1, 1);
		sm.SetInitialState(s0);

		peri::automata::Runner runner {sm};
		runner.Start();

		EXPECT_TRUE(runner.CanProcess(1));
		EXPECT_FALSE(runner.CanProcess(2));

		runner.Process(1);

		// From s1, no transitions defined
		EXPECT_FALSE(runner.CanProcess(1));
	}

	TEST(StateMachine, NotStartedBehavior)
	{
		using namespace peri::automata;

		SimpleStaticStateMachine<2, 1> sm;

		auto s0 = sm.AddState();
		auto s1 = sm.AddState();

		sm.AddTransition(s0, s1, 1);
		sm.SetInitialState(s0);

		// Create runner but don't call Start()
		peri::automata::Runner runner {sm};
		EXPECT_FALSE(runner.IsRunning());

		auto result = runner.Process(1);
		EXPECT_EQ(result, ProcessResult::NO_CURRENT_STATE);
	}

	// ============================================================================
	// Dynamic Storage Tests (runtime modifications)
	// ============================================================================

	TEST(StateMachineDynamicStorage, BasicConstruction)
	{
		using namespace peri::automata;

		DynamicStateMachine<int> sm;

		auto s0 = sm.AddState();
		auto s1 = sm.AddState();
		auto s2 = sm.AddState();

		sm.AddTransition(s0, s1, 1);
		sm.AddTransition(s1, s2, 2);
		sm.AddTransition(s2, s0, 3);

		EXPECT_EQ(sm.NumStates(), 3);
		EXPECT_EQ(sm.NumTransitions(), 3);
	}

	TEST(StateMachineDynamicStorage, RuntimeStateRemoval)
	{
		using namespace peri::automata;

		DynamicStateMachine<int> sm;

		auto s0 = sm.AddState();
		auto s1 = sm.AddState();
		auto s2 = sm.AddState();

		sm.AddTransition(s0, s1, 1);
		sm.AddTransition(s1, s2, 2);

		EXPECT_EQ(sm.NumStates(), 3);

		sm.RemoveState(s1);

		EXPECT_EQ(sm.NumStates(), 2);
		// Transitions involving s1 should be removed
		EXPECT_EQ(sm.NumTransitions(), 0);
	}

	TEST(StateMachineDynamicStorage, RuntimeTransitionRemoval)
	{
		using namespace peri::automata;

		DynamicStateMachine<int> sm;

		auto s0 = sm.AddState();
		auto s1 = sm.AddState();

		auto [t1, success1] = sm.AddTransition(s0, s1, 1);
		auto [t2, success2] = sm.AddTransition(s0, s1, 2);

		EXPECT_EQ(sm.NumTransitions(), 2);

		sm.RemoveTransition(t1);

		EXPECT_EQ(sm.NumTransitions(), 1);
	}

	TEST(StateMachineDynamicStorage, StateDataAccess)
	{
		using namespace peri::automata;

		DynamicStateMachine<int, StateData> sm;

		auto s0 = sm.AddState(StateData {42});
		auto s1 = sm.AddState(StateData {100});

			EXPECT_EQ(sm.Storage().GetStateData(s0).value, 42);
			EXPECT_EQ(sm.Storage().GetStateData(s1).value, 100);

			// Modify state data
			sm.Storage().GetStateData(s0).value = 999;
			EXPECT_EQ(sm.Storage().GetStateData(s0).value, 999);
		}

		TEST(StateMachineDynamicStorage, TypedEvents)
		{
			using namespace peri::automata;

			DynamicStateMachine<Event, StateData> sm;

			auto idle	 = sm.AddState(StateData {0});
			auto running = sm.AddState(StateData {1});
			auto paused	 = sm.AddState(StateData {2});

			sm.AddTransition(idle, running, Event::START);
			sm.AddTransition(running, paused, Event::PAUSE);
			sm.AddTransition(paused, running, Event::RESUME);
			sm.AddTransition(running, idle, Event::STOP);
			sm.AddTransition(paused, idle, Event::STOP);

			sm.SetInitialState(idle);

			peri::automata::Runner runner {sm};
			runner.Start();

			EXPECT_EQ(runner.CurrentState(), idle);

			runner.Process(Event::START);
			EXPECT_EQ(runner.CurrentState(), running);

			runner.Process(Event::PAUSE);
			EXPECT_EQ(runner.CurrentState(), paused);

			runner.Process(Event::RESUME);
			EXPECT_EQ(runner.CurrentState(), running);

			runner.Process(Event::STOP);
			EXPECT_EQ(runner.CurrentState(), idle);
		}
	}
