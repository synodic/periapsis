import std;
import synodic.honesty.test;

import synodic.library;

using namespace honesty::test;
using namespace honesty::test::literals;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace
{
	using namespace synodic::graph;
	using namespace synodic::algorithm::graph;

	// ============================================================================
	// Max Flow Algorithm Tests
	// ============================================================================

	Suite MAX_FLOW_SUITE(
		"algorithm_max_flow",
		[]() -> Generator
		{
			co_yield "simple_path"_test = [](const Requirements& requirements)
			{
				// Simple graph: 0 -> 1 -> 2 with capacity 10
				//   0 --10--> 1 --10--> 2
				// Max flow from 0 to 2 should be 10

				Graph<void, FlowEdge<int>, 4, 4> graph;
				auto v0 = graph.add_vertex();
				auto v1 = graph.add_vertex();
				auto v2 = graph.add_vertex();

				graph.add_edge(v0, v1, FlowEdge<int> {10, 0});
				graph.add_edge(v1, v2, FlowEdge<int> {10, 0});

				auto result = max_flow(graph, v0, v2);

				requirements.Expect(result.max_flow == 10);
				requirements.Expect(result.found_path);
			};

			co_yield "bottleneck"_test = [](const Requirements& requirements)
			{
				// Graph with bottleneck: 0 -> 1 -> 2
				//   0 --10--> 1 --5--> 2
				// Max flow should be limited by bottleneck = 5

				Graph<void, FlowEdge<int>, 4, 4> graph;
				auto v0 = graph.add_vertex();
				auto v1 = graph.add_vertex();
				auto v2 = graph.add_vertex();

				graph.add_edge(v0, v1, FlowEdge<int> {10, 0});
				graph.add_edge(v1, v2, FlowEdge<int> {5, 0});

				auto result = max_flow(graph, v0, v2);

				requirements.Expect(result.max_flow == 5);
			};

			co_yield "parallel_paths"_test = [](const Requirements& requirements)
			{
				// Two parallel paths: 0 -> 1 -> 3 and 0 -> 2 -> 3
				//       1
				//      / \
				//   10/   \10
				//    /     \
				//   0       3
				//    \     /
				//   10\   /10
				//      \ /
				//       2
				// Max flow = 20 (10 through each path)

				Graph<void, FlowEdge<int>, 8, 8> graph;
				auto v0 = graph.add_vertex();
				auto v1 = graph.add_vertex();
				auto v2 = graph.add_vertex();
				auto v3 = graph.add_vertex();

				graph.add_edge(v0, v1, FlowEdge<int> {10, 0});
				graph.add_edge(v0, v2, FlowEdge<int> {10, 0});
				graph.add_edge(v1, v3, FlowEdge<int> {10, 0});
				graph.add_edge(v2, v3, FlowEdge<int> {10, 0});

				auto result = max_flow(graph, v0, v3);

				requirements.Expect(result.max_flow == 20);
			};

			co_yield "no_path"_test = [](const Requirements& requirements)
			{
				// Disconnected graph: no path from 0 to 2
				//   0 -> 1    2 (isolated)

				Graph<void, FlowEdge<int>, 4, 4> graph;
				auto v0 = graph.add_vertex();
				auto v1 = graph.add_vertex();
				auto v2 = graph.add_vertex();

				graph.add_edge(v0, v1, FlowEdge<int> {10, 0});

				auto result = max_flow(graph, v0, v2);

				requirements.Expect(result.max_flow == 0);
				requirements.Expect(!result.found_path);
			};

			co_yield "constexpr_max_flow"_test = [](const Requirements& requirements)
			{
				// Verify the algorithm works at compile-time
				constexpr auto compute_flow = []() constexpr
				{
					Graph<void, FlowEdge<int>, 4, 4> graph;
					auto v0 = graph.add_vertex();
					auto v1 = graph.add_vertex();
					auto v2 = graph.add_vertex();

					graph.add_edge(v0, v1, FlowEdge<int> {10, 0});
					graph.add_edge(v1, v2, FlowEdge<int> {7, 0});

					return max_flow(graph, v0, v2);
				};

				constexpr auto result = compute_flow();

				static_assert(result.max_flow == 7);
				static_assert(result.found_path);

				requirements.Expect(result.max_flow == 7);
			};

			co_yield "classic_flow_network"_test = [](const Requirements& requirements)
			{
				// Classic max flow example:
				//
				//       (1)
				//      /   \
				//    16     12
				//    /   4   \
				//  (0)------>(2)----->(5)
				//    \   |   /   20
				//    13  | 9
				//      \ v /
				//       (3)------->(4)
				//            14     \
				//                   4 \
				//                      v
				//                     (5)
				//
				// Simplified version for test:
				//   0 --16--> 1 --12--> 3
				//   0 --13--> 2 --14--> 3
				//   1 --10--> 2
				// Max flow from 0 to 3

				Graph<void, FlowEdge<int>, 8, 8> graph;
				auto v0 = graph.add_vertex();
				auto v1 = graph.add_vertex();
				auto v2 = graph.add_vertex();
				auto v3 = graph.add_vertex();

				graph.add_edge(v0, v1, FlowEdge<int> {16, 0});
				graph.add_edge(v0, v2, FlowEdge<int> {13, 0});
				graph.add_edge(v1, v2, FlowEdge<int> {10, 0});
				graph.add_edge(v1, v3, FlowEdge<int> {12, 0});
				graph.add_edge(v2, v3, FlowEdge<int> {14, 0});

				auto result = max_flow(graph, v0, v3);

				// Max flow should be 23 (12 via v1->v3, 11 via v1->v2->v3, 0 leftover)
				// Actually: 16 from v0->v1, 12 goes to v3, 4 goes to v2
				//           13 from v0->v2, total at v2 = 17, v2->v3 capacity 14
				// So max = 12 + 14 = 26? Let's verify by expected flow paths
				// Path 1: 0->1->3 with 12
				// Path 2: 0->2->3 with 13 (limited by 14, but source limited to 13)
				// Path 3: 0->1->2->3 with 4 (remaining capacity: 16-12=4 from 0->1, 10 from 1->2, 14-13=1 from 2->3)
				// Actually limited by 2->3 = 1 remaining
				// Total = 12 + 13 + 1 = 26? No wait...
				// Let me recalculate: after path 0->1->3 (12), after path 0->2->3 (13):
				// 0->1: 4 remaining, 1->2: 10, 2->3: 1 remaining
				// Path 0->1->2->3: min(4, 10, 1) = 1
				// Total = 12 + 13 + 1 = 26

				requirements.Expect(result.max_flow == 26);
				requirements.Expect(result.found_path);
			};
		});
	SuiteRegistrar _max_flow(MAX_FLOW_SUITE);
}
