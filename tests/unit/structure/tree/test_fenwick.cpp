#include <gtest/gtest.h>

import std;
import synodic.periapsis;

using namespace peri::structure;

namespace
{
	// ========================================================================
	// FenwickDynamic Tests
	// ========================================================================

	TEST(FenwickDynamic, ConstructWithCapacity)
	{
		FenwickDynamic<int> tree(5);
		EXPECT_EQ(tree.size(), 5);
	}

	TEST(FenwickDynamic, EmptyConstructor)
	{
		FenwickDynamic<int> tree(0);
		EXPECT_TRUE(tree.empty());
	}

	TEST(FenwickDynamic, SingleUpdate)
	{
		FenwickDynamic<int> tree(3);
		tree.update(0, 5);
		EXPECT_EQ(tree.prefix(0), 5);
	}

	TEST(FenwickDynamic, MultipleUpdates)
	{
		FenwickDynamic<int> tree(4);
		tree.update(0, 1);
		tree.update(1, 2);
		tree.update(2, 3);
		tree.update(3, 4);
		EXPECT_EQ(tree.sum(), 10);
	}

	TEST(FenwickDynamic, PrefixSumBasic)
	{
		FenwickDynamic<int> tree(3);
		tree.update(0, 1);
		tree.update(1, 2);
		tree.update(2, 3);
		EXPECT_EQ(tree.prefix(0), 1);
		EXPECT_EQ(tree.prefix(1), 3);
		EXPECT_EQ(tree.prefix(2), 6);
	}

	TEST(FenwickDynamic, InclusiveRangeBasic)
	{
		FenwickDynamic<int> tree(5);
		tree.update(0, 1);
		tree.update(1, 2);
		tree.update(2, 3);
		tree.update(3, 4);
		tree.update(4, 5);
		EXPECT_EQ(tree.inclusive(1, 3), 9);
	}

	TEST(FenwickDynamic, ExclusiveRangeBasic)
	{
		FenwickDynamic<int> tree(5);
		tree.update(0, 1);
		tree.update(1, 2);
		tree.update(2, 3);
		tree.update(3, 4);
		tree.update(4, 5);
		EXPECT_EQ(tree.exclusive(1, 4), 9);
	}

	TEST(FenwickDynamic, SumAfterUpdates)
	{
		FenwickDynamic<int> tree(4);
		tree.update(0, 10);
		tree.update(1, 20);
		tree.update(2, 30);
		tree.update(3, 40);
		EXPECT_EQ(tree.sum(), 100);
	}

	TEST(FenwickDynamic, ClearResets)
	{
		FenwickDynamic<int> tree(3);
		tree.update(0, 5);
		tree.update(1, 10);
		tree.clear();
		EXPECT_EQ(tree.sum(), 0);
	}

	TEST(FenwickDynamic, UpdateOutOfBounds)
	{
		FenwickDynamic<int> tree(3);
		tree.update(5, 10);
		EXPECT_EQ(tree.sum(), 0);
	}

	TEST(FenwickDynamic, PrefixOutOfBounds)
	{
		FenwickDynamic<int> tree(3);
		tree.update(0, 1);
		tree.update(1, 2);
		tree.update(2, 3);
		EXPECT_EQ(tree.prefix(10), 6);
	}

	TEST(FenwickDynamic, InclusiveInvalidRange)
	{
		FenwickDynamic<int> tree(5);
		tree.update(0, 1);
		tree.update(1, 2);
		EXPECT_EQ(tree.inclusive(3, 1), 0);
	}

	TEST(FenwickDynamic, ExclusiveEmptyRange)
	{
		FenwickDynamic<int> tree(5);
		tree.update(0, 1);
		EXPECT_EQ(tree.exclusive(2, 2), 0);
	}

	TEST(FenwickDynamic, ConstructFromVector)
	{
		std::vector<int> vec = {0, 0, 0, 0};
		FenwickDynamic<int> tree(vec);
		tree.update(0, 1);
		tree.update(1, 2);
		EXPECT_EQ(tree.sum(), 3);
	}

	TEST(FenwickDynamic, DoubleType)
	{
		FenwickDynamic<double> tree(3);
		tree.update(0, 1.5);
		tree.update(1, 2.5);
		tree.update(2, 3.0);
		EXPECT_DOUBLE_EQ(tree.sum(), 7.0);
	}

	TEST(FenwickDynamic, NegativeValues)
	{
		FenwickDynamic<int> tree(4);
		tree.update(0, 5);
		tree.update(1, -3);
		tree.update(2, 2);
		EXPECT_EQ(tree.sum(), 4);
	}

	TEST(FenwickDynamic, LargeRange)
	{
		FenwickDynamic<int> tree(100);
		for (int i = 0; i < 100; ++i)
		{
			tree.update(i, 1);
		}
		EXPECT_EQ(tree.sum(), 100);
	}

	// ========================================================================
	// FenwickSpan Tests
	// ========================================================================

	TEST(FenwickSpan, ConstructFromSpan)
	{
		std::array<int, 5> arr = {0, 0, 0, 0, 0};
		FenwickSpan<int> tree(arr);
		EXPECT_EQ(tree.size(), 5);
	}

	TEST(FenwickSpan, UpdateViaSpan)
	{
		std::array<int, 3> arr = {0, 0, 0};
		FenwickSpan<int> tree(arr);
		tree.update(0, 5);
		tree.update(1, 10);
		EXPECT_EQ(tree.sum(), 15);
	}

	TEST(FenwickSpan, PrefixViaSpan)
	{
		std::array<int, 4> arr = {0, 0, 0, 0};
		FenwickSpan<int> tree(arr);
		tree.update(0, 1);
		tree.update(1, 2);
		tree.update(2, 3);
		EXPECT_EQ(tree.prefix(1), 3);
	}

	TEST(FenwickSpan, InclusiveViaSpan)
	{
		std::array<int, 5> arr = {0, 0, 0, 0, 0};
		FenwickSpan<int> tree(arr);
		tree.update(0, 1);
		tree.update(1, 2);
		tree.update(2, 3);
		tree.update(3, 4);
		EXPECT_EQ(tree.inclusive(1, 2), 5);
	}

	TEST(FenwickSpan, ClearViaSpan)
	{
		std::array<int, 3> arr = {0, 0, 0};
		FenwickSpan<int> tree(arr);
		tree.update(0, 5);
		tree.update(1, 10);
		tree.clear();
		EXPECT_EQ(tree.sum(), 0);
	}

	TEST(FenwickSpan, ExclusiveViaSpan)
	{
		std::array<int, 5> arr = {0, 0, 0, 0, 0};
		FenwickSpan<int> tree(arr);
		tree.update(0, 1);
		tree.update(1, 2);
		tree.update(2, 3);
		tree.update(3, 4);
		EXPECT_EQ(tree.exclusive(0, 2), 3);
	}

	// ========================================================================
	// FenwickFixed Tests
	// ========================================================================

	TEST(FenwickFixed, ConstructFixed)
	{
		std::array<int, 5> arr = {0, 0, 0, 0, 0};
		Fenwick<int, std::array<int, 5>> tree(arr);
		EXPECT_EQ(tree.size(), 5);
	}

	TEST(FenwickFixed, UpdateFixed)
	{
		std::array<int, 4> arr = {0, 0, 0, 0};
		Fenwick<int, std::array<int, 4>> tree(arr);
		tree.update(0, 5);
		tree.update(1, 10);
		EXPECT_EQ(tree.sum(), 15);
	}

	TEST(FenwickFixed, PrefixFixed)
	{
		std::array<int, 3> arr = {0, 0, 0};
		Fenwick<int, std::array<int, 3>> tree(arr);
		tree.update(0, 1);
		tree.update(1, 2);
		EXPECT_EQ(tree.prefix(1), 3);
	}

	TEST(FenwickFixed, InclusiveFixed)
	{
		std::array<int, 5> arr = {0, 0, 0, 0, 0};
		Fenwick<int, std::array<int, 5>> tree(arr);
		tree.update(0, 1);
		tree.update(1, 2);
		tree.update(2, 3);
		EXPECT_EQ(tree.inclusive(0, 2), 6);
	}

	TEST(FenwickFixed, ClearFixed)
	{
		std::array<int, 3> arr = {0, 0, 0};
		Fenwick<int, std::array<int, 3>> tree(arr);
		tree.update(0, 5);
		tree.clear();
		EXPECT_EQ(tree.sum(), 0);
	}
}
