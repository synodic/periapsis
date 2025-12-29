/**
 * @brief Unit tests for type traits
 */

#include <gtest/gtest.h>

import std;
import synodic.periapsis;

namespace
{
	TEST(TypeTraits, MinimalUnsignedType_Boundaries)
	{
		static_assert(std::is_same_v<peri::MinimalUnsignedType<0>, std::uint8_t>);
		static_assert(
			std::is_same_v<peri::MinimalUnsignedType<std::numeric_limits<std::uint8_t>::max()>, std::uint8_t>);
		static_assert(
			std::is_same_v<peri::MinimalUnsignedType<std::numeric_limits<std::uint8_t>::max() + 1>, std::uint16_t>);
		static_assert(
			std::is_same_v<peri::MinimalUnsignedType<std::numeric_limits<std::uint16_t>::max()>, std::uint16_t>);
		static_assert(
			std::is_same_v<peri::MinimalUnsignedType<std::numeric_limits<std::uint16_t>::max() + 1>, std::uint32_t>);
		static_assert(
			std::is_same_v<peri::MinimalUnsignedType<std::numeric_limits<std::uint32_t>::max()>, std::uint32_t>);
		static_assert(
			std::is_same_v<peri::MinimalUnsignedType<std::numeric_limits<std::uint32_t>::max() + 1ULL>, std::uint64_t>);
		EXPECT_TRUE(true);
	}

	TEST(TypeTraits, MinimalSignedType_Boundaries)
	{
		static_assert(std::is_same_v<peri::MinimalSignedType<0>, std::int8_t>);
		static_assert(std::is_same_v<peri::MinimalSignedType<std::numeric_limits<std::int8_t>::max()>, std::int8_t>);
		static_assert(
			std::is_same_v<peri::MinimalSignedType<std::numeric_limits<std::int8_t>::max() + 1>, std::int16_t>);
		static_assert(std::is_same_v<peri::MinimalSignedType<std::numeric_limits<std::int16_t>::max()>, std::int16_t>);
		static_assert(
			std::is_same_v<peri::MinimalSignedType<std::numeric_limits<std::int16_t>::max() + 1>, std::int32_t>);
		static_assert(std::is_same_v<peri::MinimalSignedType<std::numeric_limits<std::int32_t>::max()>, std::int32_t>);
		static_assert(
			std::is_same_v<peri::MinimalSignedType<std::numeric_limits<std::int32_t>::max() + 1ULL>, std::int64_t>);
		EXPECT_TRUE(true);
	}

	TEST(TypeTraits, MinimalIntegerType_BackwardCompatibility)
	{
		static_assert(std::is_same_v<peri::MinimalIntegerType<0>, peri::MinimalUnsignedType<0>>);
		static_assert(std::is_same_v<peri::MinimalIntegerType<256>, peri::MinimalUnsignedType<256>>);
		EXPECT_TRUE(true);
	}
}
