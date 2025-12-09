/**
 * @brief Unit tests for utility functions
 */

#include <gtest/gtest.h>

import std;
import synodic.library;

namespace
{
	TEST(Utility, NthElement)
	{
		static_assert(1 == synodic::nth_element<0>(1));
		static_assert(1 == synodic::nth_element<0>(1, 2));
		static_assert(2 == synodic::nth_element<1>(1, 2));
		static_assert(1 == synodic::nth_element<0>(1, 2, 3));
		static_assert(2 == synodic::nth_element<1>(1, 2, 3));
	}

}  // namespace
