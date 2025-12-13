#include <gtest/gtest.h>

import std;
import synodic.periapsis;

using namespace peri::sparse;

namespace
{
	// ========================================================================
	// IndexVector Tests
	// ========================================================================

	TEST(IndexVector, DefaultConstruct)
	{
		IndexVector<> iv;
		EXPECT_TRUE(iv.empty());
		EXPECT_EQ(iv.size(), 0);
	}

	TEST(IndexVector, InitializerListConstruct_SortsElements)
	{
		IndexVector<> iv {5, 1, 3, 2, 4};
		EXPECT_EQ(iv.size(), 5);

		EXPECT_EQ(iv[0], 1);
		EXPECT_EQ(iv[1], 2);
		EXPECT_EQ(iv[2], 3);
		EXPECT_EQ(iv[3], 4);
		EXPECT_EQ(iv[4], 5);
	}

	TEST(IndexVector, Insert)
	{
		IndexVector<> iv;
		EXPECT_TRUE(iv.insert(10));
		EXPECT_TRUE(iv.insert(5));
		EXPECT_TRUE(iv.insert(15));

		EXPECT_EQ(iv.size(), 3);
		EXPECT_EQ(iv[0], 5);
		EXPECT_EQ(iv[1], 10);
		EXPECT_EQ(iv[2], 15);
	}

	TEST(IndexVector, InsertDuplicate_ReturnsFalse)
	{
		IndexVector<> iv;
		EXPECT_TRUE(iv.insert(10));
		EXPECT_FALSE(iv.insert(10));
		EXPECT_EQ(iv.size(), 1);
	}

	TEST(IndexVector, Erase_RemovesPresentAndIgnoresMissing)
	{
		IndexVector<> iv {1, 2, 3, 4, 5};

		EXPECT_TRUE(iv.erase(3));
		EXPECT_EQ(iv.size(), 4);
		EXPECT_FALSE(iv.contains(3));

		EXPECT_FALSE(iv.erase(100));
		EXPECT_EQ(iv.size(), 4);
	}

	TEST(IndexVector, Contains)
	{
		IndexVector<> iv {10, 20, 30, 40, 50};

		EXPECT_TRUE(iv.contains(10));
		EXPECT_TRUE(iv.contains(30));
		EXPECT_TRUE(iv.contains(50));
		EXPECT_FALSE(iv.contains(25));
		EXPECT_FALSE(iv.contains(0));
	}

	TEST(IndexVector, Find)
	{
		IndexVector<> iv {10, 20, 30};

		auto it = iv.find(20);
		EXPECT_NE(it, iv.end());
		EXPECT_EQ(*it, 20);

		it = iv.find(25);
		EXPECT_EQ(it, iv.end());
	}

	TEST(IndexVector, Rank_ReturnsPositionOrNotValid)
	{
		IndexVector<> iv {100, 200, 300, 400, 500};

		EXPECT_EQ(iv.rank(100), 0);
		EXPECT_EQ(iv.rank(300), 2);
		EXPECT_EQ(iv.rank(500), 4);
		EXPECT_EQ(iv.rank(250), IndexVector<>::NOT_VALID);
	}

	TEST(IndexVector, Select)
	{
		IndexVector<> iv {100, 200, 300, 400, 500};

		EXPECT_EQ(iv.select(0), 100);
		EXPECT_EQ(iv.select(2), 300);
		EXPECT_EQ(iv.select(4), 500);
	}

	TEST(IndexVector, Count_ReturnsIndicesInRange)
	{
		IndexVector<> iv {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

		EXPECT_EQ(iv.count(25, 75), 5);
		EXPECT_EQ(iv.count(10, 100), 10);
		EXPECT_EQ(iv.count(45, 55), 1);
		EXPECT_EQ(iv.count(101, 200), 0);
	}

	TEST(IndexVector, Clear)
	{
		IndexVector<> iv {1, 2, 3, 4, 5};
		EXPECT_EQ(iv.size(), 5);

		iv.clear();
		EXPECT_TRUE(iv.empty());
		EXPECT_EQ(iv.size(), 0);
	}

	TEST(IndexVector, FrontBack)
	{
		IndexVector<> iv {50, 10, 90, 30};

		EXPECT_EQ(iv.front(), 10);
		EXPECT_EQ(iv.back(), 90);
	}

	TEST(IndexVector, Iteration)
	{
		IndexVector<> iv {5, 1, 3, 2, 4};

		std::vector<std::size_t> collected;
		for (auto index: iv)
		{
			collected.push_back(index);
		}

		EXPECT_EQ(collected.size(), 5);
		EXPECT_EQ(collected[0], 1);
		EXPECT_EQ(collected[1], 2);
		EXPECT_EQ(collected[2], 3);
		EXPECT_EQ(collected[3], 4);
		EXPECT_EQ(collected[4], 5);
	}

	TEST(IndexVector, Union)
	{
		IndexVector<> a {1, 3, 5, 7};
		IndexVector<> b {2, 4, 5, 6};

		auto result = a.union_with(b);

		EXPECT_EQ(result.size(), 7);
		EXPECT_TRUE(result.contains(1));
		EXPECT_TRUE(result.contains(2));
		EXPECT_TRUE(result.contains(5));
		EXPECT_TRUE(result.contains(7));
	}

	TEST(IndexVector, Intersection)
	{
		IndexVector<> a {1, 2, 3, 4, 5};
		IndexVector<> b {3, 4, 5, 6, 7};

		auto result = a.intersect(b);

		EXPECT_EQ(result.size(), 3);
		EXPECT_TRUE(result.contains(3));
		EXPECT_TRUE(result.contains(4));
		EXPECT_TRUE(result.contains(5));
	}

	TEST(IndexVector, Difference)
	{
		IndexVector<> a {1, 2, 3, 4, 5};
		IndexVector<> b {3, 4, 5, 6, 7};

		auto result = a.minus(b);

		EXPECT_EQ(result.size(), 2);
		EXPECT_TRUE(result.contains(1));
		EXPECT_TRUE(result.contains(2));
	}

	TEST(IndexVector, Comparison)
	{
		IndexVector<> a {1, 2, 3};
		IndexVector<> b {1, 2, 3};
		IndexVector<> c {1, 2, 4};

		EXPECT_EQ(a, b);
		EXPECT_NE(a, c);
		EXPECT_LT(a, c);
	}

	TEST(IndexVector, LargeSet_MaintainsSortedOrder)
	{
		IndexVector<> iv;
		iv.reserve(1000);

		for (std::size_t i = 1000; i > 0; --i)
		{
			iv.insert(i);
		}

		EXPECT_EQ(iv.size(), 1000);
		EXPECT_EQ(iv.front(), 1);
		EXPECT_EQ(iv.back(), 1000);

		for (std::size_t i = 0; i < 999; ++i)
		{
			EXPECT_LT(iv[i], iv[i + 1]);
		}
	}

	TEST(IndexVector, BulkInsert_HandlesUnsortedAndDuplicates)
	{
		IndexVector<> iv {1, 5, 10};

		std::vector<std::size_t> newIndices = {3, 7, 5, 2, 10, 8, 3};
		auto inserted						= iv.insert_range(newIndices);

		EXPECT_EQ(inserted, 4);
		EXPECT_EQ(iv.size(), 7);

		EXPECT_EQ(iv[0], 1);
		EXPECT_EQ(iv[1], 2);
		EXPECT_EQ(iv[2], 3);
		EXPECT_EQ(iv[3], 5);
		EXPECT_EQ(iv[4], 7);
		EXPECT_EQ(iv[5], 8);
		EXPECT_EQ(iv[6], 10);
	}

	TEST(IndexVector, BulkInsertInitializerList)
	{
		IndexVector<> iv;
		auto inserted = iv.insert_range({5, 3, 1, 4, 2});

		EXPECT_EQ(inserted, 5);
		EXPECT_EQ(iv.size(), 5);
		EXPECT_EQ(iv[0], 1);
		EXPECT_EQ(iv[4], 5);
	}

	TEST(IndexVector, BulkInsertLarge)
	{
		IndexVector<> iv;

		// Generate 1000 indices in reverse order (worst case for individual inserts)
		std::vector<std::size_t> indices;
		indices.reserve(1000);
		for (std::size_t i = 1000; i > 0; --i)
		{
			indices.push_back(i);
		}

		auto inserted = iv.insert_range(indices);

		EXPECT_EQ(inserted, 1000);
		EXPECT_EQ(iv.size(), 1000);
		EXPECT_EQ(iv.front(), 1);
		EXPECT_EQ(iv.back(), 1000);
	}

	// ========================================================================
	// InplaceIndexVector Tests
	// ========================================================================

	TEST(InplaceIndexVector, DefaultConstruct)
	{
		InplaceIndexVector<10> iv;
		EXPECT_TRUE(iv.empty());
		EXPECT_EQ(iv.size(), 0);
		EXPECT_EQ(iv.capacity(), 10);
	}

	TEST(InplaceIndexVector, Insert)
	{
		InplaceIndexVector<5> iv;
		EXPECT_TRUE(iv.insert(30));
		EXPECT_TRUE(iv.insert(10));
		EXPECT_TRUE(iv.insert(20));

		EXPECT_EQ(iv.size(), 3);
		EXPECT_EQ(iv[0], 10);
		EXPECT_EQ(iv[1], 20);
		EXPECT_EQ(iv[2], 30);
	}

	TEST(InplaceIndexVector, InsertFull_ReturnsFalse)
	{
		InplaceIndexVector<3> iv;
		EXPECT_TRUE(iv.insert(1));
		EXPECT_TRUE(iv.insert(2));
		EXPECT_TRUE(iv.insert(3));
		EXPECT_FALSE(iv.insert(4));

		EXPECT_EQ(iv.size(), 3);
	}

	TEST(InplaceIndexVector, Remove)
	{
		InplaceIndexVector<5> iv;
		iv.insert(1);
		iv.insert(2);
		iv.insert(3);

		EXPECT_TRUE(iv.erase(2));
		EXPECT_EQ(iv.size(), 2);
		EXPECT_EQ(iv[0], 1);
		EXPECT_EQ(iv[1], 3);
	}

	TEST(InplaceIndexVector, Contains)
	{
		InplaceIndexVector<10> iv;
		iv.insert(5);
		iv.insert(10);
		iv.insert(15);

		EXPECT_TRUE(iv.contains(5));
		EXPECT_TRUE(iv.contains(10));
		EXPECT_TRUE(iv.contains(15));
		EXPECT_FALSE(iv.contains(7));
	}

	TEST(InplaceIndexVector, RankSelect)
	{
		InplaceIndexVector<10> iv;
		iv.insert(100);
		iv.insert(200);
		iv.insert(300);

		EXPECT_EQ(iv.rank(100), 0);
		EXPECT_EQ(iv.rank(200), 1);
		EXPECT_EQ(iv.rank(300), 2);

		EXPECT_EQ(iv.select(0), 100);
		EXPECT_EQ(iv.select(1), 200);
		EXPECT_EQ(iv.select(2), 300);
	}

	TEST(InplaceIndexVector, Iteration)
	{
		InplaceIndexVector<10> iv;
		iv.insert(3);
		iv.insert(1);
		iv.insert(2);

		std::vector<std::size_t> collected;
		for (auto index: iv)
		{
			collected.push_back(index);
		}

		EXPECT_EQ(collected.size(), 3);
		EXPECT_EQ(collected[0], 1);
		EXPECT_EQ(collected[1], 2);
		EXPECT_EQ(collected[2], 3);
	}

	TEST(InplaceIndexVector, Clear_PreservesCapacity)
	{
		InplaceIndexVector<5> iv;
		iv.insert(1);
		iv.insert(2);
		iv.insert(3);

		iv.clear();
		EXPECT_TRUE(iv.empty());
		EXPECT_EQ(iv.capacity(), 5);
	}

	TEST(InplaceIndexVector, DifferentIndexTypes)
	{
		InplaceIndexVector<10, std::uint32_t> iv;
		iv.insert(static_cast<std::uint32_t>(100));
		iv.insert(static_cast<std::uint32_t>(200));

		EXPECT_EQ(iv.size(), 2);
		EXPECT_EQ(iv[0], 100u);
		EXPECT_EQ(iv[1], 200u);
	}
}
