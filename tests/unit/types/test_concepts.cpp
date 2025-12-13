/**
 * @brief Unit tests for container concepts
 */

#include <gtest/gtest.h>

import std;
import synodic.periapsis;

namespace
{
	// Type aliases for testing
	using Array				= std::array<int, 3>;
	using Deque				= std::deque<int>;
	using ForwardList		= std::forward_list<int>;
	using List				= std::list<int>;
	using Map				= std::map<int, int>;
	using MultiMap			= std::multimap<int, int>;
	using Set				= std::set<int>;
	using MultiSet			= std::multiset<int>;
	using String			= std::basic_string<int>;
	using UnorderedMap		= std::unordered_map<int, int>;
	using UnorderedMultiMap = std::unordered_multimap<int, int>;
	using UnorderedSet		= std::unordered_set<int>;
	using UnorderedMultiSet = std::unordered_multiset<int>;
	using Vector			= std::vector<int>;

	TEST(Container, Clearable)
	{
		static_assert(peri::clearable_container<Deque>);
		static_assert(peri::clearable_container<ForwardList>);
		static_assert(peri::clearable_container<List>);
		static_assert(peri::clearable_container<Map>);
		static_assert(peri::clearable_container<MultiMap>);
		static_assert(peri::clearable_container<Set>);
		static_assert(peri::clearable_container<MultiSet>);
		static_assert(peri::clearable_container<String>);
		static_assert(peri::clearable_container<UnorderedMap>);
		static_assert(peri::clearable_container<UnorderedMultiMap>);
		static_assert(peri::clearable_container<UnorderedSet>);
		static_assert(peri::clearable_container<UnorderedMultiSet>);
		static_assert(peri::clearable_container<Vector>);

		static_assert(not peri::clearable_container<Array>);
	}
}
