/**
 * @brief Unit tests for sparse hash set
 */

#include <gtest/gtest.h>

import synodic.library;

namespace
{
	using HashSet = synodic::sparse::HashSet<int>;

	TEST(SparseHashSet, Type)
	{
		// static_assert(synodic::associative_container<HashSet>);
	}

}  // namespace
