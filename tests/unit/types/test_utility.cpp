/**
 * @brief Unit tests for utility functions
 */

#include <gtest/gtest.h>

import std;
import synodic.periapsis;

namespace
{
	TEST(Utility, NthElement)
	{
		static_assert(1 == peri::nth_element<0>(1));
		static_assert(1 == peri::nth_element<0>(1, 2));
		static_assert(2 == peri::nth_element<1>(1, 2));
		static_assert(1 == peri::nth_element<0>(1, 2, 3));
		static_assert(2 == peri::nth_element<1>(1, 2, 3));
	}
}
