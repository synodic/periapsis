/**
 * @brief Unit tests for DynamicBitset
 */

#include <gtest/gtest.h>

import synodic.periapsis;

import std;

namespace
{
	using Bitset = peri::DynamicBitset<>;

	// ========================================================================
	// Construction Tests
	// ========================================================================

	TEST(DynamicBitset, DefaultConstruction)
	{
		static_assert(
			[]() constexpr
			{
				const Bitset bs;
				return bs.empty() && bs.size() == 0;
			}());
		const Bitset bs;
		EXPECT_TRUE(bs.empty());
		EXPECT_EQ(bs.size(), 0);
	}

	TEST(DynamicBitset, SizeConstruction)
	{
		static_assert(
			[]() constexpr
			{
				const Bitset bs(64);
				return bs.size() == 64 && bs.none();
			}());
		const Bitset bs(64);
		EXPECT_EQ(bs.size(), 64);
		EXPECT_TRUE(bs.none());
	}

	TEST(DynamicBitset, SizeValueConstruction)
	{
		static_assert(
			[]() constexpr
			{
				const Bitset bs(16, 0b10101010);
				return bs.size() == 16 && bs.count() == 4;
			}());
		const Bitset bs(16, 0b10101010);
		EXPECT_EQ(bs.size(), 16);
		EXPECT_EQ(bs.count(), 4);
		EXPECT_TRUE(bs.test(1));
		EXPECT_FALSE(bs.test(0));
	}

	TEST(DynamicBitset, StringConstruction)
	{
		const Bitset bs("11001100");
		EXPECT_EQ(bs.size(), 8);
		EXPECT_EQ(bs.count(), 4);
		EXPECT_TRUE(bs.test(2));
		EXPECT_TRUE(bs.test(3));
		EXPECT_FALSE(bs.test(0));
		EXPECT_FALSE(bs.test(1));
	}

	TEST(DynamicBitset, StdStringConstruction)
	{
		const std::string str = "10101010";
		const Bitset bs(str);
		EXPECT_EQ(bs.size(), 8);
		EXPECT_EQ(bs.count(), 4);
	}

	// ========================================================================
	// Capacity Tests
	// ========================================================================

	TEST(DynamicBitset, Capacity)
	{
		Bitset bs;
		EXPECT_TRUE(bs.empty());
		bs.resize(100);
		EXPECT_FALSE(bs.empty());
		EXPECT_EQ(bs.size(), 100);
		EXPECT_GE(bs.capacity(), 100);
	}

	TEST(DynamicBitset, Reserve)
	{
		Bitset bs;
		bs.reserve(1000);
		EXPECT_GE(bs.capacity(), 1000);
		EXPECT_EQ(bs.size(), 0);
	}

	TEST(DynamicBitset, ShrinkToFit)
	{
		Bitset bs(1000);
		bs.resize(10);
		bs.shrink_to_fit();
		// Implementation-defined, but capacity should be reduced
		EXPECT_GE(bs.capacity(), 10);
	}

	// ========================================================================
	// Element Access Tests
	// ========================================================================

	TEST(DynamicBitset, SubscriptOperator)
	{
		Bitset bs(10);
		bs[3] = true;
		bs[7] = true;
		EXPECT_TRUE(bs[3]);
		EXPECT_TRUE(bs[7]);
		EXPECT_FALSE(bs[0]);
	}

	TEST(DynamicBitset, ConstSubscriptOperator)
	{
		const Bitset bs(8, 0b10101010);
		EXPECT_FALSE(bs[0]);
		EXPECT_TRUE(bs[1]);
		EXPECT_FALSE(bs[2]);
		EXPECT_TRUE(bs[3]);
	}

	TEST(DynamicBitset, Test)
	{
		const Bitset bs(8, 0b11110000);
		EXPECT_FALSE(bs.test(0));
		EXPECT_TRUE(bs.test(4));
		EXPECT_THROW(bs.test(100), std::out_of_range);
	}

	TEST(DynamicBitset, TestSet)
	{
		Bitset bs(8);
		EXPECT_FALSE(bs.test_set(3, true));
		EXPECT_TRUE(bs.test(3));
		EXPECT_TRUE(bs.test_set(3, false));
		EXPECT_FALSE(bs.test(3));
	}

	// ========================================================================
	// Bit Manipulation Tests
	// ========================================================================

	TEST(DynamicBitset, Set)
	{
		Bitset bs(16);
		bs.set();
		EXPECT_TRUE(bs.all());
		EXPECT_EQ(bs.count(), 16);
	}

	TEST(DynamicBitset, SetPosition)
	{
		Bitset bs(16);
		bs.set(5);
		EXPECT_TRUE(bs.test(5));
		EXPECT_EQ(bs.count(), 1);
		EXPECT_THROW(bs.set(100), std::out_of_range);
	}

	TEST(DynamicBitset, Reset)
	{
		Bitset bs(16, 0xFF'FF);
		bs.reset();
		EXPECT_TRUE(bs.none());
	}

	TEST(DynamicBitset, ResetPosition)
	{
		Bitset bs(16, 0xFF'FF);
		bs.reset(5);
		EXPECT_FALSE(bs.test(5));
		EXPECT_EQ(bs.count(), 15);
	}

	TEST(DynamicBitset, Flip)
	{
		Bitset bs(8, 0b11110000);
		bs.flip();
		EXPECT_EQ(bs.count(), 4);
		EXPECT_TRUE(bs.test(0));
		EXPECT_FALSE(bs.test(4));
	}

	TEST(DynamicBitset, FlipPosition)
	{
		Bitset bs(8);
		bs.flip(3);
		EXPECT_TRUE(bs.test(3));
		bs.flip(3);
		EXPECT_FALSE(bs.test(3));
	}

	// ========================================================================
	// Resizing Tests
	// ========================================================================

	TEST(DynamicBitset, Clear)
	{
		Bitset bs(100);
		bs.clear();
		EXPECT_TRUE(bs.empty());
		EXPECT_EQ(bs.size(), 0);
	}

	TEST(DynamicBitset, Resize)
	{
		Bitset bs(8, 0xFF);
		bs.resize(16, true);
		EXPECT_EQ(bs.size(), 16);
		EXPECT_TRUE(bs.all());

		bs.resize(4);
		EXPECT_EQ(bs.size(), 4);
		EXPECT_TRUE(bs.all());
	}

	TEST(DynamicBitset, PushBack)
	{
		Bitset bs;
		bs.push_back(true);
		bs.push_back(false);
		bs.push_back(true);
		EXPECT_EQ(bs.size(), 3);
		EXPECT_TRUE(bs.test(0));
		EXPECT_FALSE(bs.test(1));
		EXPECT_TRUE(bs.test(2));
	}

	TEST(DynamicBitset, PopBack)
	{
		Bitset bs(8, 0xFF);
		bs.pop_back();
		EXPECT_EQ(bs.size(), 7);
		EXPECT_TRUE(bs.all());
	}

	TEST(DynamicBitset, Append)
	{
		Bitset bs(4, 0xF);
		bs.append(0xAA);
		EXPECT_EQ(bs.size(), 4 + Bitset::BITS_PER_BLOCK);
	}

	// ========================================================================
	// Query Tests
	// ========================================================================

	TEST(DynamicBitset, Count)
	{
		static_assert(
			[]() constexpr
			{
				const Bitset bs(64, 0xAA'AA'AA'AA'AA'AA'AA'AA);
				return bs.count() == 32;
			}());
		const Bitset bs(64, 0xAA'AA'AA'AA'AA'AA'AA'AA);
		EXPECT_EQ(bs.count(), 32);
	}

	TEST(DynamicBitset, All)
	{
		Bitset bs(10);
		bs.set();
		EXPECT_TRUE(bs.all());
		bs.reset(0);
		EXPECT_FALSE(bs.all());
	}

	TEST(DynamicBitset, Any)
	{
		Bitset bs(100);
		EXPECT_FALSE(bs.any());
		bs.set(50);
		EXPECT_TRUE(bs.any());
	}

	TEST(DynamicBitset, None)
	{
		Bitset bs(100);
		EXPECT_TRUE(bs.none());
		bs.set(50);
		EXPECT_FALSE(bs.none());
	}

	// ========================================================================
	// Search Tests
	// ========================================================================

	TEST(DynamicBitset, FindFirst)
	{
		Bitset bs(100);
		EXPECT_EQ(bs.find_first(), Bitset::NOT_VALID);
		bs.set(42);
		EXPECT_EQ(bs.find_first(), 42);
		bs.set(10);
		EXPECT_EQ(bs.find_first(), 10);
	}

	TEST(DynamicBitset, FindNext)
	{
		Bitset bs(100);
		bs.set(10);
		bs.set(42);
		bs.set(99);
		EXPECT_EQ(bs.find_next(0), 10);
		EXPECT_EQ(bs.find_next(10), 42);
		EXPECT_EQ(bs.find_next(42), 99);
		EXPECT_EQ(bs.find_next(99), Bitset::NOT_VALID);
	}

	TEST(DynamicBitset, FindFirstOff)
	{
		Bitset bs(100);
		bs.set();
		EXPECT_EQ(bs.find_first_off(), Bitset::NOT_VALID);
		bs.reset(42);
		EXPECT_EQ(bs.find_first_off(), 42);
		bs.reset(10);
		EXPECT_EQ(bs.find_first_off(), 10);
	}

	TEST(DynamicBitset, FindNextOff)
	{
		Bitset bs(100);
		bs.set();
		bs.reset(10);
		bs.reset(42);
		bs.reset(99);
		EXPECT_EQ(bs.find_next_off(0), 10);
		EXPECT_EQ(bs.find_next_off(10), 42);
		EXPECT_EQ(bs.find_next_off(42), 99);
		EXPECT_EQ(bs.find_next_off(99), Bitset::NOT_VALID);
	}

	// ========================================================================
	// Set Operation Tests
	// ========================================================================

	TEST(DynamicBitset, IsSubsetOf)
	{
		const Bitset a(8, 0b00001111);
		const Bitset b(8, 0b11111111);
		const Bitset c(8, 0b00001111);

		EXPECT_TRUE(a.is_subset_of(b));
		EXPECT_TRUE(a.is_subset_of(c));
		EXPECT_FALSE(b.is_subset_of(a));
	}

	TEST(DynamicBitset, IsProperSubsetOf)
	{
		const Bitset a(8, 0b00001111);
		const Bitset b(8, 0b11111111);
		const Bitset c(8, 0b00001111);

		EXPECT_TRUE(a.is_proper_subset_of(b));
		EXPECT_FALSE(a.is_proper_subset_of(c));
	}

	TEST(DynamicBitset, Intersects)
	{
		const Bitset a(8, 0b00001111);	// bits 0-3
		const Bitset b(8, 0b11110000);	// bits 4-7 (no overlap with a)
		const Bitset c(8, 0b00001100);	// bits 2-3 (overlaps with a)

		EXPECT_FALSE(a.intersects(b));
		EXPECT_TRUE(a.intersects(c));
	}

	// ========================================================================
	// Bitwise Operator Tests
	// ========================================================================

	TEST(DynamicBitset, BitwiseAnd)
	{
		Bitset a(8, 0b11110000);
		const Bitset b(8, 0b11001100);
		a &= b;
		EXPECT_EQ(a.to_ulong(), 0b11000000);
	}

	TEST(DynamicBitset, BitwiseOr)
	{
		Bitset a(8, 0b11110000);
		const Bitset b(8, 0b00001111);
		a |= b;
		EXPECT_EQ(a.to_ulong(), 0b11111111);
	}

	TEST(DynamicBitset, BitwiseXor)
	{
		Bitset a(8, 0b11110000);
		const Bitset b(8, 0b11001100);
		a ^= b;
		EXPECT_EQ(a.to_ulong(), 0b00111100);
	}

	TEST(DynamicBitset, BitwiseDifference)
	{
		Bitset a(8, 0b11111111);
		const Bitset b(8, 0b11110000);
		a -= b;
		EXPECT_EQ(a.to_ulong(), 0b00001111);
	}

	TEST(DynamicBitset, BitwiseNot)
	{
		const Bitset a(8, 0b11110000);
		const Bitset b = ~a;
		EXPECT_EQ(b.to_ulong(), 0b00001111);
	}

	TEST(DynamicBitset, LeftShift)
	{
		Bitset a(16, 0b11110000);
		a <<= 4;
		EXPECT_EQ(a.to_ulong(), 0b111100000000);
	}

	TEST(DynamicBitset, RightShift)
	{
		Bitset a(16, 0b111100000000);
		a >>= 4;
		EXPECT_EQ(a.to_ulong(), 0b11110000);
	}

	TEST(DynamicBitset, BinaryAnd)
	{
		const Bitset a(8, 0b11110000);
		const Bitset b(8, 0b11001100);
		const Bitset c = a & b;
		EXPECT_EQ(c.to_ulong(), 0b11000000);
	}

	TEST(DynamicBitset, BinaryOr)
	{
		const Bitset a(8, 0b11110000);
		const Bitset b(8, 0b00001111);
		const Bitset c = a | b;
		EXPECT_EQ(c.to_ulong(), 0b11111111);
	}

	TEST(DynamicBitset, BinaryXor)
	{
		const Bitset a(8, 0b11110000);
		const Bitset b(8, 0b11001100);
		const Bitset c = a ^ b;
		EXPECT_EQ(c.to_ulong(), 0b00111100);
	}

	// ========================================================================
	// Comparison Tests
	// ========================================================================

	TEST(DynamicBitset, Equality)
	{
		const Bitset a(8, 0b11110000);
		const Bitset b(8, 0b11110000);
		const Bitset c(8, 0b00001111);

		EXPECT_EQ(a, b);
		EXPECT_NE(a, c);
	}

	TEST(DynamicBitset, ThreeWayComparison)
	{
		const Bitset a(8, 0b00001111);
		const Bitset b(8, 0b11110000);
		const Bitset c(8, 0b00001111);

		EXPECT_TRUE(a < b);
		EXPECT_TRUE(b > a);
		EXPECT_TRUE(a <= c);
		EXPECT_TRUE(a >= c);
	}

	// ========================================================================
	// Conversion Tests
	// ========================================================================

	TEST(DynamicBitset, ToUlong)
	{
		const Bitset bs(32, 0xDE'AD'BE'EF);
		EXPECT_EQ(bs.to_ulong(), 0xDE'AD'BE'EF);
	}

	TEST(DynamicBitset, ToUllong)
	{
		const Bitset bs(64, 0xDE'AD'BE'EF'12'34'56'78ULL);
		EXPECT_EQ(bs.to_ullong(), 0xDE'AD'BE'EF'12'34'56'78ULL);
	}

	TEST(DynamicBitset, ToString)
	{
		const Bitset bs(8, 0b10101010);
		EXPECT_EQ(bs.to_string(), "10101010");
	}

	// ========================================================================
	// Iterator Tests
	// ========================================================================

	TEST(DynamicBitset, IteratorBasic)
	{
		Bitset bs(8, 0b10101010);
		auto it = bs.begin();
		EXPECT_FALSE(*it);
		++it;
		EXPECT_TRUE(*it);
		++it;
		EXPECT_FALSE(*it);
	}

	TEST(DynamicBitset, IteratorRangeFor)
	{
		const Bitset bs(8, 0b11110000);
		std::size_t count = 0;
		for (const bool bit: bs)
		{
			if (bit)
			{
				++count;
			}
		}
		EXPECT_EQ(count, 4);
	}

	TEST(DynamicBitset, IteratorModification)
	{
		Bitset bs(8);
		for (auto it = bs.begin(); it != bs.end(); ++it)
		{
			*it = true;
		}
		EXPECT_TRUE(bs.all());
	}

	TEST(DynamicBitset, ReverseIterator)
	{
		Bitset bs(8, 0b10000000);
		auto it = bs.rbegin();
		EXPECT_TRUE(*it);
		++it;
		EXPECT_FALSE(*it);
	}

	TEST(DynamicBitset, IteratorRandomAccess)
	{
		const Bitset bs(8, 0b10101010);
		const auto it = bs.begin();
		EXPECT_FALSE(it[0]);
		EXPECT_TRUE(it[1]);
		EXPECT_FALSE(it[2]);
		EXPECT_TRUE(it[3]);
	}

	// ========================================================================
	// Reference Proxy Tests
	// ========================================================================

	TEST(DynamicBitset, ReferenceAssignment)
	{
		Bitset bs(8);
		bs[0] = true;
		bs[1] = bs[0];
		EXPECT_TRUE(bs[1]);
	}

	TEST(DynamicBitset, ReferenceFlip)
	{
		Bitset bs(8, 0b11110000);
		bs[0].flip();
		EXPECT_TRUE(bs[0]);
		bs[4].flip();
		EXPECT_FALSE(bs[4]);
	}

	TEST(DynamicBitset, ReferenceCompoundAssignment)
	{
		Bitset bs(8, 0b10101010);
		bs[0] |= true;
		EXPECT_TRUE(bs[0]);
		bs[1] &= false;
		EXPECT_FALSE(bs[1]);
		bs[2] ^= true;
		EXPECT_TRUE(bs[2]);
	}

	// ========================================================================
	// Swap Tests
	// ========================================================================

	TEST(DynamicBitset, MemberSwap)
	{
		Bitset a(8, 0xFF);
		Bitset b(16, 0);
		a.swap(b);
		EXPECT_EQ(a.size(), 16);
		EXPECT_EQ(b.size(), 8);
		EXPECT_TRUE(a.none());
		EXPECT_TRUE(b.all());
	}

	TEST(DynamicBitset, FreeSwap)
	{
		Bitset a(8, 0xFF);
		Bitset b(16, 0);
		swap(a, b);
		EXPECT_EQ(a.size(), 16);
		EXPECT_EQ(b.size(), 8);
	}

	// ========================================================================
	// Custom Block Type Tests
	// ========================================================================

	TEST(DynamicBitset, CustomBlockType)
	{
		peri::DynamicBitset<std::uint8_t> bs8(100);
		peri::DynamicBitset<std::uint16_t> bs16(100);
		peri::DynamicBitset<std::uint32_t> bs32(100);
		peri::DynamicBitset<std::uint64_t> bs64(100);

		bs8.set(50);
		bs16.set(50);
		bs32.set(50);
		bs64.set(50);

		EXPECT_TRUE(bs8.test(50));
		EXPECT_TRUE(bs16.test(50));
		EXPECT_TRUE(bs32.test(50));
		EXPECT_TRUE(bs64.test(50));
	}

	// ========================================================================
	// Constexpr Tests
	// ========================================================================

	TEST(DynamicBitset, ConstexprOperations)
	{
		constexpr auto test = []()
		{
			peri::DynamicBitset<> bs(64, 0xAA'AA'AA'AA'AA'AA'AA'AA);
			bs.set(0);
			bs.reset(1);
			bs.flip(2);
			return bs.count();
		};

		constexpr auto result = test();
		static_assert(result == 33);
		EXPECT_EQ(result, 33);
	}

	// ========================================================================
	// Edge Cases
	// ========================================================================

	TEST(DynamicBitset, EmptyBitset)
	{
		const Bitset bs;
		EXPECT_TRUE(bs.none());
		EXPECT_TRUE(bs.all());	// Vacuously true
		EXPECT_FALSE(bs.any());
		EXPECT_EQ(bs.count(), 0);
		EXPECT_EQ(bs.find_first(), Bitset::NOT_VALID);
	}

	TEST(DynamicBitset, SingleBit)
	{
		Bitset bs(1);
		EXPECT_FALSE(bs[0]);
		bs.set(0);
		EXPECT_TRUE(bs.all());
		EXPECT_EQ(bs.count(), 1);
	}

	TEST(DynamicBitset, LargeBitset)
	{
		Bitset bs(10'000);
		bs.set(9999);
		EXPECT_EQ(bs.find_first(), 9999);
		EXPECT_EQ(bs.count(), 1);
	}

	TEST(DynamicBitset, ShiftByZero)
	{
		Bitset bs(16, 0xFF);
		bs <<= 0;
		EXPECT_EQ(bs.to_ulong(), 0xFF);
		bs >>= 0;
		EXPECT_EQ(bs.to_ulong(), 0xFF);
	}

	TEST(DynamicBitset, ShiftBeyondSize)
	{
		Bitset bs(16, 0xFF);
		bs <<= 100;
		EXPECT_TRUE(bs.none());

		bs = Bitset(16, 0xFF);
		bs >>= 100;
		EXPECT_TRUE(bs.none());
	}
}
