#include <gtest/gtest.h>

import std;
import synodic.periapsis;

using namespace peri::sparse;
using namespace peri::fixed;

namespace
{
	// ========================================================================
	// BitVector Tests
	// ========================================================================

	TEST(BitVector, DefaultConstruct)
	{
		BitVector<> bv;
		EXPECT_TRUE(bv.empty());
		EXPECT_EQ(bv.size(), 0);
	}

	TEST(BitVector, ConstructWithSize)
	{
		BitVector<> bv(100);
		EXPECT_EQ(bv.size(), 100);
		EXPECT_TRUE(bv.none());
	}

	TEST(BitVector, SetAndTest)
	{
		BitVector<> bv(64);
		bv.set(10);
		bv.set(20);
		bv.set(30);

		EXPECT_TRUE(bv.test(10));
		EXPECT_TRUE(bv.test(20));
		EXPECT_TRUE(bv.test(30));
		EXPECT_FALSE(bv.test(15));
	}

	TEST(BitVector, ResetBit)
	{
		BitVector<> bv(32);
		bv.set(5);
		EXPECT_TRUE(bv.test(5));

		bv.reset(5);
		EXPECT_FALSE(bv.test(5));
	}

	TEST(BitVector, FlipBit)
	{
		BitVector<> bv(32);
		EXPECT_FALSE(bv.test(7));

		bv.flip(7);
		EXPECT_TRUE(bv.test(7));

		bv.flip(7);
		EXPECT_FALSE(bv.test(7));
	}

	TEST(BitVector, Count)
	{
		BitVector<> bv(100);
		bv.set(0);
		bv.set(25);
		bv.set(50);
		bv.set(75);
		bv.set(99);

		EXPECT_EQ(bv.count(), 5);
	}

	TEST(BitVector, RankBasic)
	{
		BitVector<> bv(100);
		bv.set(10);
		bv.set(20);
		bv.set(30);
		bv.set(40);

		EXPECT_EQ(bv.rank(9), 0);	// Before first set bit
		EXPECT_EQ(bv.rank(10), 1);	// At first set bit
		EXPECT_EQ(bv.rank(15), 1);	// Between first and second
		EXPECT_EQ(bv.rank(20), 2);	// At second set bit
		EXPECT_EQ(bv.rank(40), 4);	// At fourth set bit
		EXPECT_EQ(bv.rank(50), 4);	// After all set bits
	}

	TEST(BitVector, RankRange)
	{
		BitVector<> bv(100);
		bv.set(10);
		bv.set(20);
		bv.set(30);
		bv.set(40);

		EXPECT_EQ(bv.rank(15, 35), 2);	// 20 and 30
		EXPECT_EQ(bv.rank(10, 40), 4);	// All four
		EXPECT_EQ(bv.rank(11, 19), 0);	// None
	}

	TEST(BitVector, SelectBasic)
	{
		BitVector<> bv(100);
		bv.set(10);
		bv.set(20);
		bv.set(30);
		bv.set(40);

		EXPECT_EQ(bv.select(0), 10);					  // First set bit
		EXPECT_EQ(bv.select(1), 20);					  // Second set bit
		EXPECT_EQ(bv.select(2), 30);					  // Third set bit
		EXPECT_EQ(bv.select(3), 40);					  // Fourth set bit
		EXPECT_EQ(bv.select(4), BitVector<>::NOT_VALID);  // Out of range
	}

	TEST(BitVector, SelectWithDenseBlock)
	{
		BitVector<> bv(128);
		// Set consecutive bits
		for (std::size_t i = 0; i < 64; ++i)
		{
			bv.set(i);
		}

		EXPECT_EQ(bv.select(0), 0);
		EXPECT_EQ(bv.select(32), 32);
		EXPECT_EQ(bv.select(63), 63);
		EXPECT_EQ(bv.select(64), BitVector<>::NOT_VALID);
	}

	TEST(BitVector, FindFirst)
	{
		BitVector<> bv(100);
		EXPECT_EQ(bv.find_first(), BitVector<>::NOT_VALID);

		bv.set(42);
		EXPECT_EQ(bv.find_first(), 42);

		bv.set(10);
		EXPECT_EQ(bv.find_first(), 10);
	}

	TEST(BitVector, FindNext)
	{
		BitVector<> bv(100);
		bv.set(10);
		bv.set(20);
		bv.set(30);

		EXPECT_EQ(bv.find_next(5), 10);
		EXPECT_EQ(bv.find_next(10), 20);
		EXPECT_EQ(bv.find_next(20), 30);
		EXPECT_EQ(bv.find_next(30), BitVector<>::NOT_VALID);
	}

	TEST(BitVector, AllAnyNone)
	{
		BitVector<> bv(8);

		EXPECT_TRUE(bv.none());
		EXPECT_FALSE(bv.any());
		EXPECT_FALSE(bv.all());

		bv.set(0);
		EXPECT_FALSE(bv.none());
		EXPECT_TRUE(bv.any());
		EXPECT_FALSE(bv.all());

		for (std::size_t i = 1; i < 8; ++i)
		{
			bv.set(i);
		}
		EXPECT_FALSE(bv.none());
		EXPECT_TRUE(bv.any());
		EXPECT_TRUE(bv.all());
	}

	TEST(BitVector, Clear)
	{
		BitVector<> bv(64);
		bv.set(10);
		bv.set(20);
		bv.set(30);

		EXPECT_EQ(bv.count(), 3);

		bv.clear();
		EXPECT_EQ(bv.count(), 0);
		EXPECT_TRUE(bv.none());
	}

	TEST(BitVector, LargeVector)
	{
		BitVector<> bv(10'000);

		// Set every 100th bit
		for (std::size_t i = 0; i < 10'000; i += 100)
		{
			bv.set(i);
		}

		EXPECT_EQ(bv.count(), 100);
		EXPECT_EQ(bv.select(0), 0);
		EXPECT_EQ(bv.select(50), 5000);
		EXPECT_EQ(bv.select(99), 9900);
	}

	TEST(BitVector, RankSelectConsistency)
	{
		BitVector<> bv(1000);

		// Set random bits
		std::vector<std::size_t> setBits = {5, 17, 42, 100, 256, 500, 789, 999};
		for (auto bit: setBits)
		{
			bv.set(bit);
		}

		// For each set bit, select(rank(pos)-1) should return pos
		for (std::size_t i = 0; i < setBits.size(); ++i)
		{
			EXPECT_EQ(bv.select(i), setBits[i]);
		}
	}

	// ========================================================================
	// fixed::Bitset Tests
	// ========================================================================

	TEST(FixedBitset, DefaultConstruct)
	{
		Bitset<64> bs;
		EXPECT_EQ(bs.size(), 64);
		EXPECT_TRUE(bs.none());
	}

	TEST(FixedBitset, SetAndTest)
	{
		Bitset<128> bs;
		bs.set(10);
		bs.set(100);

		EXPECT_TRUE(bs.test(10));
		EXPECT_TRUE(bs.test(100));
		EXPECT_FALSE(bs.test(50));
	}

	TEST(FixedBitset, Count)
	{
		Bitset<64> bs;
		bs.set(0);
		bs.set(31);
		bs.set(63);

		EXPECT_EQ(bs.count(), 3);
	}

	TEST(FixedBitset, AllAnyNone)
	{
		Bitset<8> bs;
		EXPECT_TRUE(bs.none());
		EXPECT_FALSE(bs.any());

		bs.set(0);
		EXPECT_FALSE(bs.none());
		EXPECT_TRUE(bs.any());
		EXPECT_FALSE(bs.all());

		bs.set();  // Set all
		EXPECT_TRUE(bs.all());
	}

	TEST(FixedBitset, BlockAccess)
	{
		Bitset<128> bs;
		bs.set(0);
		bs.set(64);

		EXPECT_EQ(bs.block(0), 1);
		EXPECT_EQ(bs.block(1), 1);
	}

	TEST(FixedBitset, FindFirst)
	{
		Bitset<100> bs;
		EXPECT_EQ(bs.find_first(), Bitset<100>::NOT_VALID);

		bs.set(50);
		EXPECT_EQ(bs.find_first(), 50);

		bs.set(10);
		EXPECT_EQ(bs.find_first(), 10);
	}

	// ========================================================================
	// InplaceBitVector Tests
	// ========================================================================

	TEST(InplaceBitVector, BasicOperations)
	{
		InplaceBitVector<128> bv;
		bv.set(10);
		bv.set(64);
		bv.set(100);

		EXPECT_TRUE(bv.test(10));
		EXPECT_TRUE(bv.test(64));
		EXPECT_TRUE(bv.test(100));
		EXPECT_EQ(bv.count(), 3);
	}

	TEST(InplaceBitVector, RankSelect)
	{
		InplaceBitVector<256> bv;
		bv.set(0);
		bv.set(64);
		bv.set(128);
		bv.set(192);

		EXPECT_EQ(bv.rank(63), 1);
		EXPECT_EQ(bv.rank(64), 2);
		EXPECT_EQ(bv.rank(200), 4);

		EXPECT_EQ(bv.select(0), 0);
		EXPECT_EQ(bv.select(1), 64);
		EXPECT_EQ(bv.select(2), 128);
		EXPECT_EQ(bv.select(3), 192);
	}

	// ========================================================================
	// SetBitIterator Tests
	// ========================================================================

	TEST(BitVector, SetBitsIteration)
	{
		BitVector<> bv(100);
		std::vector<std::size_t> expected = {5, 10, 42, 73, 99};

		for (auto pos: expected)
		{
			bv.set(pos);
		}

		std::vector<std::size_t> collected;
		for (auto pos: bv.set_bits())
		{
			collected.push_back(pos);
		}

		EXPECT_EQ(collected, expected);
	}

	TEST(BitVector, SetBitsIterationEmpty)
	{
		BitVector<> bv(64);

		std::vector<std::size_t> collected;
		for (auto pos: bv.set_bits())
		{
			collected.push_back(pos);
		}

		EXPECT_TRUE(collected.empty());
	}

	TEST(BitVector, SetBitsManualIteration)
	{
		BitVector<> bv(50);
		bv.set(10);
		bv.set(20);
		bv.set(30);

		auto it = bv.set_bits_begin();
		EXPECT_EQ(*it, 10);
		++it;
		EXPECT_EQ(*it, 20);
		++it;
		EXPECT_EQ(*it, 30);
		++it;
		EXPECT_EQ(it, bv.set_bits_end());
	}

	TEST(InplaceBitVector, SetBitsIteration)
	{
		InplaceBitVector<256> bv;
		std::vector<std::size_t> expected = {0, 64, 128, 200, 255};

		for (auto pos: expected)
		{
			bv.set(pos);
		}

		std::vector<std::size_t> collected;
		for (auto pos: bv.set_bits())
		{
			collected.push_back(pos);
		}

		EXPECT_EQ(collected, expected);
	}

	TEST(BitVector, SetBitsLarge)
	{
		BitVector<> bv(10'000);

		// Set every 100th bit
		std::vector<std::size_t> expected;
		for (std::size_t i = 0; i < 10'000; i += 100)
		{
			bv.set(i);
			expected.push_back(i);
		}

		std::vector<std::size_t> collected;
		for (auto pos: bv.set_bits())
		{
			collected.push_back(pos);
		}

		EXPECT_EQ(collected, expected);
	}
}
