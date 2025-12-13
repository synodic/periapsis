/**
 * @brief Unit tests for fixed ring buffer and related containers
 */

#include <gtest/gtest.h>

import std;
import synodic.periapsis;

namespace
{
	using namespace peri;

	TEST(FixedRingBuffer, DefaultConstruction)
	{
		FixedRingBuffer<int, 8> buffer;
		EXPECT_TRUE(buffer.empty());
		EXPECT_EQ(buffer.size(), 0);
		EXPECT_EQ(buffer.capacity(), 8);
	}

	TEST(FixedRingBuffer, PushBackPopFront)
	{
		FixedRingBuffer<int, 8> buffer;
		buffer.push_back(1);
		buffer.push_back(2);
		buffer.push_back(3);

		EXPECT_EQ(buffer.size(), 3);
		EXPECT_EQ(buffer.front(), 1);
		EXPECT_EQ(buffer.back(), 3);

		buffer.pop_front();
		EXPECT_EQ(buffer.front(), 2);
		EXPECT_EQ(buffer.size(), 2);
	}

	TEST(FixedRingBuffer, PushFrontPopBack)
	{
		FixedRingBuffer<int, 8> buffer;
		buffer.push_front(1);
		buffer.push_front(2);
		buffer.push_front(3);

		EXPECT_EQ(buffer.size(), 3);
		EXPECT_EQ(buffer.front(), 3);
		EXPECT_EQ(buffer.back(), 1);

		buffer.pop_back();
		EXPECT_EQ(buffer.back(), 2);
		EXPECT_EQ(buffer.size(), 2);
	}

	TEST(FixedRingBuffer, WrapAround)
	{
		FixedRingBuffer<int, 4> buffer;
		buffer.push_back(1);
		buffer.push_back(2);
		buffer.push_back(3);
		buffer.pop_front();	 // Remove 1
		buffer.pop_front();	 // Remove 2
		buffer.push_back(4);
		buffer.push_back(5);  // Should wrap around

		EXPECT_EQ(buffer.size(), 3);
		EXPECT_EQ(buffer.front(), 3);
		EXPECT_EQ(buffer.back(), 5);
	}

	TEST(FixedRingBuffer, ConstexprUsage)
	{
		constexpr auto test = []() constexpr
		{
			FixedRingBuffer<int, 8> buffer;
			buffer.push_back(10);
			buffer.push_back(20);
			return buffer.front() + buffer.back();
		}();

		static_assert(test == 30);
		EXPECT_EQ(test, 30);
	}

	TEST(FixedRingBuffer, StdQueueAdapter)
	{
		FixedQueue<int, 8> queue;
		queue.push(1);
		queue.push(2);
		queue.push(3);

		EXPECT_EQ(queue.front(), 1);
		EXPECT_EQ(queue.back(), 3);
		EXPECT_EQ(queue.size(), 3);

		queue.pop();
		EXPECT_EQ(queue.front(), 2);
	}

	TEST(FixedRingBuffer, StdStackAdapter)
	{
		FixedStack<int, 8> stack;
		stack.push(1);
		stack.push(2);
		stack.push(3);

		EXPECT_EQ(stack.top(), 3);
		EXPECT_EQ(stack.size(), 3);

		stack.pop();
		EXPECT_EQ(stack.top(), 2);
	}
}
