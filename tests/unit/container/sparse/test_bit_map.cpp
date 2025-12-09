/**
 * @brief Unit tests for sparse bit map
 */

#include <gtest/gtest.h>

import synodic.library;

namespace
{
	using BitMap = synodic::sparse::BitMap<int, 4>;

	TEST(SparseBitMap, Type)
	{
		// static_assert(synodic::sequence_container<BitMap>);
	}

}  // namespace
