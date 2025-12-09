/**
 * @brief Unit tests for sparse vector
 */

#include <gtest/gtest.h>

import synodic.library;

namespace
{
	using Vector = synodic::sparse::Vector<int, 3>;

	TEST(SparseVector, Type)
	{
		// static_assert(synodic::contiguous_container<Vector>);
	}

}  // namespace
