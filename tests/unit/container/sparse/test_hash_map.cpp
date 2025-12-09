/**
 * @brief Unit tests for sparse hash map
 */

#include <gtest/gtest.h>

import synodic.library;

namespace
{
	using HashMap = synodic::sparse::HashMap<int, int>;

	TEST(SparseHashMap, Type)
	{
		// static_assert(synodic::associative_container<HashMap>);
	}

}  // namespace
