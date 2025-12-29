/**
 * @brief Unit tests for SPSC queue
 */

#include <gtest/gtest.h>

import synodic.periapsis;
import std;

namespace
{
	// Type aliases for testing
	using Array				= std::array<int, 3>;
	using Deque				= std::deque<int>;
	using ForwardList		= std::forward_list<int>;
	using List				= std::list<int>;
	using Map				= std::map<int, int>;
	using MultiMap			= std::multimap<int, int>;
	using Set				= std::set<int>;
	using MultiSet			= std::multiset<int>;
	using String			= std::basic_string<int>;
	using UnorderedMap		= std::unordered_map<int, int>;
	using UnorderedMultiMap = std::unordered_multimap<int, int>;
	using UnorderedSet		= std::unordered_set<int>;
	using UnorderedMultiSet = std::unordered_multiset<int>;
	using Vector			= std::vector<int>;

	template<typename T>
	using Queue = peri::lockfree::SPSCQueue<int, T>;

	TEST(SPSCQueue, StorageType)
	{
		// Verifies that containers with size(), operator[], and reverse iterators satisfy the concept
		static_assert(peri::lockfree::spsc_queue_container<Array>);
		static_assert(peri::lockfree::spsc_queue_container<Deque>);
		static_assert(peri::lockfree::spsc_queue_container<String>);
		static_assert(peri::lockfree::spsc_queue_container<Vector>);

		// Containers without operator[] or reverse iterators do not satisfy the concept
		static_assert(not peri::lockfree::spsc_queue_container<ForwardList>);
		static_assert(not peri::lockfree::spsc_queue_container<List>);
		static_assert(not peri::lockfree::spsc_queue_container<Map>);
		static_assert(not peri::lockfree::spsc_queue_container<MultiMap>);
		static_assert(not peri::lockfree::spsc_queue_container<MultiSet>);
		static_assert(not peri::lockfree::spsc_queue_container<Set>);
		static_assert(not peri::lockfree::spsc_queue_container<UnorderedMap>);
		static_assert(not peri::lockfree::spsc_queue_container<UnorderedMultiMap>);
		static_assert(not peri::lockfree::spsc_queue_container<UnorderedMultiSet>);
		static_assert(not peri::lockfree::spsc_queue_container<UnorderedSet>);
	}

	// ========================================================================
	// Construction Tests
	// ========================================================================

	TEST(SPSCQueue, ConstructFromVector)
	{
		Vector container(5);
		Queue<Vector> queue(container);
		EXPECT_EQ(queue.max_size(), 4);
		EXPECT_TRUE(queue.empty());
		EXPECT_EQ(queue.size(), 0);
	}

	TEST(SPSCQueue, ConstructFromVectorMoveSemantics)
	{
		Vector container(3);
		Queue<Vector> queue(std::move(container));
		EXPECT_EQ(queue.max_size(), 2);
		EXPECT_TRUE(queue.empty());
	}

	TEST(SPSCQueue, ConstructFromDeque)
	{
		Deque container;
		container.resize(7);
		Queue<Deque> queue(container);
		EXPECT_EQ(queue.max_size(), 6);
	}

	TEST(SPSCQueue, DeleteCopyConstructor)
	{
		static_assert(!std::copy_constructible<Queue<Vector>>);
	}

	TEST(SPSCQueue, DeleteMoveConstructor)
	{
		static_assert(!std::move_constructible<Queue<Vector>>);
	}

	// ========================================================================
	// Capacity Tests
	// ========================================================================

	TEST(SPSCQueue, EmptyQueue)
	{
		Vector container(5);
		Queue<Vector> queue(container);
		EXPECT_TRUE(queue.empty());
	}

	TEST(SPSCQueue, CapacityOperations)
	{
		Vector container(10);
		Queue<Vector> queue(container);
		EXPECT_EQ(queue.max_size(), 9);
		EXPECT_EQ(queue.size(), 0);
	}

	// ========================================================================
	// Push Operations Tests
	// ========================================================================

	TEST(SPSCQueue, TryPushConstReference)
	{
		Vector container(3);
		Queue<Vector> queue(container);

		int value = 42;
		EXPECT_TRUE(queue.try_push(value));
		EXPECT_FALSE(queue.empty());
		EXPECT_EQ(queue.size(), 1);
	}

	TEST(SPSCQueue, TryPushRValueReference)
	{
		Vector container(3);
		Queue<Vector> queue(container);

		EXPECT_TRUE(queue.try_push(99));
		EXPECT_EQ(queue.size(), 1);
	}

	TEST(SPSCQueue, PushConstReference)
	{
		Vector container(3);
		Queue<Vector> queue(container);

		int value = 42;
		EXPECT_NO_THROW(queue.push(value));
		EXPECT_EQ(queue.size(), 1);
	}

	TEST(SPSCQueue, PushRValueReference)
	{
		Vector container(3);
		Queue<Vector> queue(container);

		EXPECT_NO_THROW(queue.push(99));
		EXPECT_EQ(queue.size(), 1);
	}

	TEST(SPSCQueue, TryPushToFullQueue)
	{
		Vector container(3);
		Queue<Vector> queue(container);

		// Can store up to max_size() which is 2
		EXPECT_TRUE(queue.try_push(1));
		EXPECT_TRUE(queue.try_push(2));
		EXPECT_FALSE(queue.try_push(3));
		EXPECT_EQ(queue.size(), 2);
	}

	TEST(SPSCQueue, PushToFullQueueThrows)
	{
		Vector container(2);
		Queue<Vector> queue(container);

		// max_size() is 1
		queue.push(1);
		EXPECT_THROW(queue.push(2), std::out_of_range);
	}

	TEST(SPSCQueue, MultiplePushOperations)
	{
		Vector container(6);
		Queue<Vector> queue(container);

		// max_size() is 5
		for (int i = 0; i < 5; ++i)
		{
			EXPECT_TRUE(queue.try_push(i));
		}
		EXPECT_EQ(queue.size(), 5);
		EXPECT_FALSE(queue.try_push(99));
	}

	// ========================================================================
	// Pop Operations Tests
	// ========================================================================

	TEST(SPSCQueue, PopFromEmptyQueue)
	{
		Vector container(3);
		Queue<Vector> queue(container);

		auto result = queue.pop();
		EXPECT_FALSE(result.has_value());
	}

	TEST(SPSCQueue, PopSingleElement)
	{
		Vector container(3);
		Queue<Vector> queue(container);

		queue.push(42);
		auto result = queue.pop();

		EXPECT_TRUE(result.has_value());
		EXPECT_EQ(result.value(), 42);
		EXPECT_TRUE(queue.empty());
	}

	TEST(SPSCQueue, PopMultipleElements)
	{
		Vector container(6);
		Queue<Vector> queue(container);

		// max_size() is 5
		for (int i = 0; i < 5; ++i)
		{
			queue.push(i);
		}

		for (int i = 0; i < 5; ++i)
		{
			auto result = queue.pop();
			EXPECT_TRUE(result.has_value());
			EXPECT_EQ(result.value(), i);
		}
		EXPECT_TRUE(queue.empty());
	}

	TEST(SPSCQueue, PopAfterPushWrapAround)
	{
		Vector container(4);
		Queue<Vector> queue(container);

		// max_size() is 3
		queue.push(1);
		queue.push(2);
		queue.push(3);

		// Pop one to make space
		auto result1 = queue.pop();
		EXPECT_EQ(result1.value(), 1);

		// Push again (tests wrap-around)
		queue.push(4);

		// Pop remaining
		EXPECT_EQ(queue.pop().value(), 2);
		EXPECT_EQ(queue.pop().value(), 3);
		EXPECT_EQ(queue.pop().value(), 4);
	}

	// ========================================================================
	// Emplace Operations Tests
	// ========================================================================

	TEST(SPSCQueue, TryEmplace)
	{
		Vector container(3);
		Queue<Vector> queue(container);

		EXPECT_TRUE(queue.try_emplace(42));
		EXPECT_EQ(queue.size(), 1);
		EXPECT_EQ(queue.pop().value(), 42);
	}

	TEST(SPSCQueue, Emplace)
	{
		Vector container(3);
		Queue<Vector> queue(container);

		EXPECT_NO_THROW(queue.emplace(99));
		EXPECT_EQ(queue.size(), 1);
		EXPECT_EQ(queue.pop().value(), 99);
	}

	TEST(SPSCQueue, TryEmplaceToFullQueue)
	{
		Vector container(2);
		Queue<Vector> queue(container);

		// max_size() is 1
		EXPECT_TRUE(queue.try_emplace(1));
		EXPECT_FALSE(queue.try_emplace(2));
	}

	TEST(SPSCQueue, EmplaceToFullQueueThrows)
	{
		Vector container(2);
		Queue<Vector> queue(container);

		// max_size() is 1
		queue.emplace(1);
		EXPECT_THROW(queue.emplace(2), std::out_of_range);
	}

	// ========================================================================
	// Extract Tests
	// ========================================================================

	TEST(SPSCQueue, ExtractContainer)
	{
		Vector container(5);
		Queue<Vector> queue(container);

		queue.push(1);
		queue.push(2);

		auto extracted = queue.extract();
		EXPECT_EQ(extracted.size(), 5);
		EXPECT_EQ(queue.max_size(), 0);
	}

	TEST(SPSCQueue, ExtractResetsQueue)
	{
		Vector container(3);
		Queue<Vector> queue(container);

		queue.push(42);
		auto extracted = queue.extract();

		EXPECT_EQ(queue.max_size(), 0);
		EXPECT_EQ(queue.size(), 0);
		EXPECT_TRUE(queue.empty());
	}

	// ========================================================================
	// Mixed Operations Tests
	// ========================================================================

	TEST(SPSCQueue, PushPopInterleaved)
	{
		Vector container(4);
		Queue<Vector> queue(container);

		queue.push(1);
		queue.push(2);

		EXPECT_EQ(queue.pop().value(), 1);

		queue.push(3);
		queue.push(4);

		EXPECT_EQ(queue.pop().value(), 2);
		EXPECT_EQ(queue.pop().value(), 3);
		EXPECT_EQ(queue.pop().value(), 4);
		EXPECT_TRUE(queue.empty());
	}

	TEST(SPSCQueue, MixedPushEmplace)
	{
		Vector container(5);
		Queue<Vector> queue(container);

		// max_size() is 4
		queue.push(1);
		queue.emplace(2);
		queue.push(3);
		queue.emplace(4);

		EXPECT_EQ(queue.size(), 4);
		EXPECT_EQ(queue.pop().value(), 1);
		EXPECT_EQ(queue.pop().value(), 2);
		EXPECT_EQ(queue.pop().value(), 3);
		EXPECT_EQ(queue.pop().value(), 4);
	}

	TEST(SPSCQueue, LargeSequentialOperations)
	{
		Vector container(101);
		Queue<Vector> queue(container);

		// max_size() is 100
		for (int i = 0; i < 100; ++i)
		{
			EXPECT_TRUE(queue.try_push(i));
		}
		EXPECT_EQ(queue.size(), 100);

		// Pop 100 elements
		for (int i = 0; i < 100; ++i)
		{
			auto result = queue.pop();
			EXPECT_TRUE(result.has_value());
			EXPECT_EQ(result.value(), i);
		}
		EXPECT_TRUE(queue.empty());
	}
}
