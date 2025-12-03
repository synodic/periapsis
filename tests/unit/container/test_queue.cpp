import std;
import synodic.honesty.test;

import synodic.library;

using namespace honesty::test;
using namespace honesty::test::literals;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace
{
	using namespace synodic;

	Suite SUITE(
		"fixed_ring_buffer",
		[]() -> Generator
		{
			co_yield "default_construction"_test = [](const Requirements& requirements)
			{
				FixedRingBuffer<int, 8> buffer;
				requirements.Expect(buffer.empty());
				requirements.Expect(buffer.size() == 0);
				requirements.Expect(buffer.capacity() == 8);
			};

			co_yield "push_back_pop_front"_test = [](const Requirements& requirements)
			{
				FixedRingBuffer<int, 8> buffer;
				buffer.push_back(1);
				buffer.push_back(2);
				buffer.push_back(3);

				requirements.Expect(buffer.size() == 3);
				requirements.Expect(buffer.front() == 1);
				requirements.Expect(buffer.back() == 3);

				buffer.pop_front();
				requirements.Expect(buffer.front() == 2);
				requirements.Expect(buffer.size() == 2);
			};

			co_yield "push_front_pop_back"_test = [](const Requirements& requirements)
			{
				FixedRingBuffer<int, 8> buffer;
				buffer.push_front(1);
				buffer.push_front(2);
				buffer.push_front(3);

				requirements.Expect(buffer.size() == 3);
				requirements.Expect(buffer.front() == 3);
				requirements.Expect(buffer.back() == 1);

				buffer.pop_back();
				requirements.Expect(buffer.back() == 2);
				requirements.Expect(buffer.size() == 2);
			};

			co_yield "wrap_around"_test = [](const Requirements& requirements)
			{
				FixedRingBuffer<int, 4> buffer;
				buffer.push_back(1);
				buffer.push_back(2);
				buffer.push_back(3);
				buffer.pop_front();	 // Remove 1
				buffer.pop_front();	 // Remove 2
				buffer.push_back(4);
				buffer.push_back(5);  // Should wrap around

				requirements.Expect(buffer.size() == 3);
				requirements.Expect(buffer.front() == 3);
				requirements.Expect(buffer.back() == 5);
			};

			co_yield "constexpr_usage"_test = [](const Requirements& requirements)
			{
				constexpr auto test = []() constexpr
				{
					FixedRingBuffer<int, 8> buffer;
					buffer.push_back(10);
					buffer.push_back(20);
					return buffer.front() + buffer.back();
				}();

				static_assert(test == 30);
				requirements.Expect(test == 30);
			};

			co_yield "std_queue_adapter"_test = [](const Requirements& requirements)
			{
				FixedQueue<int, 8> queue;
				queue.push(1);
				queue.push(2);
				queue.push(3);

				requirements.Expect(queue.front() == 1);
				requirements.Expect(queue.back() == 3);
				requirements.Expect(queue.size() == 3);

				queue.pop();
				requirements.Expect(queue.front() == 2);
			};

			co_yield "std_stack_adapter"_test = [](const Requirements& requirements)
			{
				FixedStack<int, 8> stack;
				stack.push(1);
				stack.push(2);
				stack.push(3);

				requirements.Expect(stack.top() == 3);
				requirements.Expect(stack.size() == 3);

				stack.pop();
				requirements.Expect(stack.top() == 2);
			};
		});
	SuiteRegistrar _(SUITE);
}
