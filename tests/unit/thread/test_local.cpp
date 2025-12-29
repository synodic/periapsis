/**
 * @brief Unit tests for ThreadLocal wrapper
 */

#include <gtest/gtest.h>

import std;
import synodic.periapsis;

namespace
{
	using namespace peri;

	TEST(ThreadLocal, DefaultConstruction)
	{
		ThreadLocal<int> value;
		EXPECT_EQ(*value, 0);
		EXPECT_TRUE(value.HasValue());
	}

	TEST(ThreadLocal, FactoryConstruction)
	{
		int constructionCount = 0;
		ThreadLocal value(
			[&constructionCount]
			{
				return ++constructionCount;
			});

		// First access should trigger construction
		EXPECT_EQ(*value, 1);
		EXPECT_EQ(constructionCount, 1);

		// Subsequent access should not trigger construction
		EXPECT_EQ(*value, 1);
		EXPECT_EQ(constructionCount, 1);
	}

	TEST(ThreadLocal, PrototypeConstruction)
	{
		ThreadLocal vec(std::vector{1, 2, 3});

		auto& v = *vec;
		EXPECT_EQ(v.size(), 3);
		EXPECT_EQ(v[0], 1);
		EXPECT_EQ(v[1], 2);
		EXPECT_EQ(v[2], 3);
	}

	TEST(ThreadLocal, DeductionGuide)
	{
		ThreadLocal counter(
			[]
			{
				return 42;
			});
		EXPECT_EQ(*counter, 42);

		ThreadLocal text(
			[]
			{
				return std::string{"hello"};
			});
		EXPECT_EQ(*text, "hello");
	}

	TEST(ThreadLocal, PointerAccess)
	{
		ThreadLocal<std::vector<int>> vec;
		vec->push_back(1);
		vec->push_back(2);
		vec->push_back(3);

		EXPECT_EQ(vec->size(), 3);
		EXPECT_EQ((*vec)[0], 1);
		EXPECT_EQ((*vec)[2], 3);
	}

	TEST(ThreadLocal, HasValue)
	{
		ThreadLocal<int> value;

		// Should have value after first access
		EXPECT_FALSE(value.HasValue());
		(void)*value; // Trigger initialization
		EXPECT_TRUE(value.HasValue());
	}

	TEST(ThreadLocal, ResetNoArgs)
	{
		ThreadLocal value(
			[]
			{
				return 42;
			});

		// Access and modify
		*value = 100;
		EXPECT_EQ(*value, 100);

		// Reset should destroy and recreate with factory
		value.Reset();
		EXPECT_FALSE(value.HasValue());

		// Next access uses factory again
		EXPECT_EQ(*value, 42);
	}

	TEST(ThreadLocal, ResetWithValue)
	{
		ThreadLocal value(
			[]
			{
				return 42;
			});

		EXPECT_EQ(*value, 42);

		// Reset to specific value
		value.Reset(999);
		EXPECT_EQ(*value, 999);
		EXPECT_TRUE(value.HasValue());

		// Modify and reset again
		*value = 123;
		value.Reset(777);
		EXPECT_EQ(*value, 777);
	}

	TEST(ThreadLocal, MoveConstruction)
	{
		ThreadLocal original(
			[]
			{
				return 42;
			});
		*original = 100;

		ThreadLocal<int> moved = std::move(original);
		EXPECT_EQ(*moved, 100);
	}

	TEST(ThreadLocal, MoveAssignment)
	{
		ThreadLocal original(
			[]
			{
				return 42;
			});
		*original = 100;

		ThreadLocal target(
			[]
			{
				return 99;
			});
		target = std::move(original);

		EXPECT_EQ(*target, 100);
	}

	TEST(ThreadLocal, MultipleInstances)
	{
		ThreadLocal value1(
			[]
			{
				return 1;
			});
		ThreadLocal value2(
			[]
			{
				return 2;
			});
		ThreadLocal value3(
			[]
			{
				return 3;
			});

		EXPECT_EQ(*value1, 1);
		EXPECT_EQ(*value2, 2);
		EXPECT_EQ(*value3, 3);

		*value1 = 10;
		*value2 = 20;
		*value3 = 30;

		EXPECT_EQ(*value1, 10);
		EXPECT_EQ(*value2, 20);
		EXPECT_EQ(*value3, 30);
	}

	TEST(ThreadLocal, ThreadIsolation)
	{
		ThreadLocal sharedValue(
			[]
			{
				return 0;
			});

		// Set value in main thread
		*sharedValue = 42;

		// Launch another thread and verify it has its own value
		std::jthread worker(
			[&sharedValue]
			{
				EXPECT_EQ(*sharedValue, 0); // Factory default
				*sharedValue = 100;
				EXPECT_EQ(*sharedValue, 100);
			});

		worker.join();

		// Main thread value should be unchanged
		EXPECT_EQ(*sharedValue, 42);
	}

	TEST(ThreadLocal, MultipleThreads)
	{
		ThreadLocal<std::vector<int>> buffer;
		std::atomic threadCount{0};

		auto worker = [&buffer, &threadCount](int id)
		{
			// Each thread should start with empty buffer
			EXPECT_TRUE(buffer->empty());

			// Add thread-specific data
			for (int i = 0; i < 10; ++i)
			{
				buffer->push_back(id * 100 + i);
			}

			EXPECT_EQ(buffer->size(), 10);
			EXPECT_EQ((*buffer)[0], id * 100);
			EXPECT_EQ((*buffer)[9], id * 100 + 9);

			++threadCount;
		};

		// Spawn multiple threads
		std::vector<std::jthread> threads;
		for (int i = 0; i < 5; ++i)
		{
			threads.emplace_back(worker, i);
		}

		// Wait for all threads
		threads.clear();

		EXPECT_EQ(threadCount, 5);

		// Main thread buffer should still be empty
		EXPECT_TRUE(buffer->empty());
	}

	TEST(ThreadLocal, ComplexType)
	{
		struct Counter
		{
			int value = 0;
			std::string name;

			Counter() :
				name("default")
			{
			}

			explicit Counter(std::string n) :
				name(std::move(n))
			{
			}
		};

		ThreadLocal counter(
			[]
			{
				return Counter("custom");
			});

		EXPECT_EQ(counter->value, 0);
		EXPECT_EQ(counter->name, "custom");

		counter->value = 42;
		EXPECT_EQ(counter->value, 42);
	}

	TEST(ThreadLocal, MoveOnlyType)
	{
		ThreadLocal ptr(
			[]
			{
				return std::make_unique<int>(42);
			});

		EXPECT_NE(ptr->get(), nullptr);
		EXPECT_EQ(**ptr, 42);

		**ptr = 100;
		EXPECT_EQ(**ptr, 100);
	}

	TEST(ThreadLocal, ResetAcrossThreads)
	{
		ThreadLocal value(
			[]
			{
				return 1;
			});

		*value = 10;
		EXPECT_EQ(*value, 10);

		std::jthread worker(
			[&value]
			{
				*value = 20;
				EXPECT_EQ(*value, 20);

				value.Reset();
				EXPECT_EQ(*value, 1); // Factory default
			});

		worker.join();

		// Main thread value unaffected
		EXPECT_EQ(*value, 10);
	}

	TEST(ThreadLocal, LazyInitialization)
	{
		int initCount = 0;

		ThreadLocal value(
			[&initCount]
			{
				initCount++;
				return 42;
			});

		// No initialization yet
		EXPECT_EQ(initCount, 0);
		EXPECT_FALSE(value.HasValue());

		// First access triggers initialization
		[[maybe_unused]] auto v = *value;
		EXPECT_EQ(initCount, 1);
		EXPECT_TRUE(value.HasValue());

		// Subsequent access doesn't re-initialize
		[[maybe_unused]] auto v2 = *value;
		EXPECT_EQ(initCount, 1);
	}

	TEST(ThreadLocal, ExceptionInFactory)
	{
		struct ThrowingFactory
		{
			int operator()() const
			{
				throw std::runtime_error("Factory failed");
			}
		};

		ThreadLocal value(ThrowingFactory{});

		EXPECT_THROW(*value, std::runtime_error);
	}

	TEST(ThreadLocal, ThreadLocalStorage)
	{
		// Verify that each ThreadLocal<T> instance has independent storage
		ThreadLocal tl1(
			[]
			{
				return 1;
			});
		ThreadLocal tl2(
			[]
			{
				return 2;
			});

		EXPECT_EQ(*tl1, 1);
		EXPECT_EQ(*tl2, 2);

		*tl1 = 100;
		*tl2 = 200;

		EXPECT_EQ(*tl1, 100);
		EXPECT_EQ(*tl2, 200);

		// In another thread
		std::jthread worker(
			[&tl1, &tl2]
			{
				EXPECT_EQ(*tl1, 1); // Factory default
				EXPECT_EQ(*tl2, 2); // Factory default
			});

		worker.join();
	}
}
