/**
 * @brief Unit tests for max flow algorithm
 */

#include <gtest/gtest.h>

import std;
import synodic.periapsis;

namespace
{
	using namespace peri::graph;
	using namespace peri::algorithm::graph;

	// ============================================================================
	// Max Flow Algorithm Tests
	// ============================================================================

	TEST(AlgorithmMaxFlow, SimplePath)
	{
		// Simple graph: 0 -> 1 -> 2 with capacity 10
		//   0 --10--> 1 --10--> 2
		// Max flow from 0 to 2 should be 10

		Graph<void, FlowEdge<int>, Fixed<4, 4>> graph;
		auto v0 = graph.AddVertex();
		auto v1 = graph.AddVertex();
		auto v2 = graph.AddVertex();

		graph.AddEdge(v0, v1, FlowEdge<int> {10, 0});
		graph.AddEdge(v1, v2, FlowEdge<int> {10, 0});

		auto result = MaxFlow(graph, v0, v2);

		EXPECT_EQ(result.maxFlow, 10);
		EXPECT_TRUE(result.foundPath);
	}

	TEST(AlgorithmMaxFlow, Bottleneck)
	{
		// Graph with bottleneck: 0 -> 1 -> 2
		//   0 --10--> 1 --5--> 2
		// Max flow should be limited by bottleneck = 5

		Graph<void, FlowEdge<int>, Fixed<4, 4>> graph;
		auto v0 = graph.AddVertex();
		auto v1 = graph.AddVertex();
		auto v2 = graph.AddVertex();

		graph.AddEdge(v0, v1, FlowEdge<int> {10, 0});
		graph.AddEdge(v1, v2, FlowEdge<int> {5, 0});

		auto result = MaxFlow(graph, v0, v2);

		EXPECT_EQ(result.maxFlow, 5);
	}

	TEST(AlgorithmMaxFlow, ParallelPaths)
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

		Graph<void, FlowEdge<int>, Fixed<8, 8>> graph;
		auto v0 = graph.AddVertex();
		auto v1 = graph.AddVertex();
		auto v2 = graph.AddVertex();
		auto v3 = graph.AddVertex();

		graph.AddEdge(v0, v1, FlowEdge<int> {10, 0});
		graph.AddEdge(v0, v2, FlowEdge<int> {10, 0});
		graph.AddEdge(v1, v3, FlowEdge<int> {10, 0});
		graph.AddEdge(v2, v3, FlowEdge<int> {10, 0});

		auto result = MaxFlow(graph, v0, v3);

		EXPECT_EQ(result.maxFlow, 20);
	}

	TEST(AlgorithmMaxFlow, NoPath)
	{
		// Disconnected graph: no path from 0 to 2
		//   0 -> 1    2 (isolated)

		Graph<void, FlowEdge<int>, Fixed<4, 4>> graph;
		auto v0 = graph.AddVertex();
		auto v1 = graph.AddVertex();
		auto v2 = graph.AddVertex();

		graph.AddEdge(v0, v1, FlowEdge<int> {10, 0});

		auto result = MaxFlow(graph, v0, v2);

		EXPECT_EQ(result.maxFlow, 0);
		EXPECT_FALSE(result.foundPath);
	}

	TEST(AlgorithmMaxFlow, ConstexprMaxFlow)
	{
		// Verify the algorithm works at compile-time
		constexpr auto compute_flow = []() constexpr
		{
			Graph<void, FlowEdge<int>, Fixed<4, 4>> graph;
			auto v0 = graph.AddVertex();
			auto v1 = graph.AddVertex();
			auto v2 = graph.AddVertex();

			graph.AddEdge(v0, v1, FlowEdge<int> {10, 0});
			graph.AddEdge(v1, v2, FlowEdge<int> {7, 0});

			return MaxFlow(graph, v0, v2);
		};

		constexpr auto result = compute_flow();

		static_assert(result.maxFlow == 7);
		static_assert(result.foundPath);

		EXPECT_EQ(result.maxFlow, 7);
	}

	TEST(AlgorithmMaxFlow, ClassicFlowNetwork)
	{
		// Classic max flow example (simplified):
		//   0 --16--> 1 --12--> 3
		//   0 --13--> 2 --14--> 3
		//   1 --10--> 2
		// Max flow from 0 to 3

		Graph<void, FlowEdge<int>, Fixed<8, 8>> graph;
		auto v0 = graph.AddVertex();
		auto v1 = graph.AddVertex();
		auto v2 = graph.AddVertex();
		auto v3 = graph.AddVertex();

		graph.AddEdge(v0, v1, FlowEdge<int> {16, 0});
		graph.AddEdge(v0, v2, FlowEdge<int> {13, 0});
		graph.AddEdge(v1, v2, FlowEdge<int> {10, 0});
		graph.AddEdge(v1, v3, FlowEdge<int> {12, 0});
		graph.AddEdge(v2, v3, FlowEdge<int> {14, 0});

		auto result = MaxFlow(graph, v0, v3);

		// Max flow should be 26
		EXPECT_EQ(result.maxFlow, 26);
		EXPECT_TRUE(result.foundPath);
	}
}
