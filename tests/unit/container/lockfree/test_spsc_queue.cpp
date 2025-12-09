/**
 * @brief Unit tests for SPSC queue
 */

#include <gtest/gtest.h>

import synodic.library;
import std;

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

	template<typename T>
	using Queue = synodic::lockfree::SPSCQueue<int, T>;

	TEST(SPSCQueue, StorageType)
	{
		// Verifies that the expected, supported standard containers fulfill the queue storage concept
		static_assert(synodic::lockfree::spsc_queue_container<Deque>);
		static_assert(synodic::lockfree::spsc_queue_container<List>);
		static_assert(synodic::lockfree::spsc_queue_container<Vector>);

		static_assert(not synodic::lockfree::spsc_queue_container<Array>);
		static_assert(not synodic::lockfree::spsc_queue_container<ForwardList>);
		static_assert(not synodic::lockfree::spsc_queue_container<Map>);
		static_assert(not synodic::lockfree::spsc_queue_container<MultiMap>);
		static_assert(not synodic::lockfree::spsc_queue_container<Set>);
		static_assert(not synodic::lockfree::spsc_queue_container<MultiSet>);
		static_assert(not synodic::lockfree::spsc_queue_container<String>);
		static_assert(not synodic::lockfree::spsc_queue_container<UnorderedMap>);
		static_assert(not synodic::lockfree::spsc_queue_container<UnorderedMultiMap>);
		static_assert(not synodic::lockfree::spsc_queue_container<UnorderedSet>);
		static_assert(not synodic::lockfree::spsc_queue_container<UnorderedMultiSet>);
	}

}  // namespace
