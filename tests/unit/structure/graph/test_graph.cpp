/**
 * @brief Unit tests for graph data structure
 */

#include <gtest/gtest.h>

import std;
import synodic.library;

namespace
{
	struct VertexData
	{
		int id = 0;

		constexpr bool operator==(const VertexData&) const = default;
	};

	struct EdgeData
	{
		float weight = 1.0f;

		constexpr bool operator==(const EdgeData&) const = default;
	};

	// ============================================================================
	// Static Storage Tests (compile-time subgraphs)
	// ============================================================================

	TEST(GraphStaticStorage, ConstexprConstruction)
	{
		using namespace synodic::graph;
		using namespace synodic::graph::storage;

		// Verify constexpr construction works
		constexpr auto make_graph = []() constexpr
		{
			StaticStorage<void, void, 4, 4> storage;
			auto v0 = storage.add_vertex();
			auto v1 = storage.add_vertex();
			auto v2 = storage.add_vertex();
			storage.add_edge(v0, v1);
			storage.add_edge(v1, v2);
			storage.add_edge(v0, v2);
			return storage;
		};

		constexpr auto storage = make_graph();

		static_assert(storage.num_vertices() == 3);
		static_assert(storage.num_edges() == 3);

		EXPECT_EQ(storage.num_vertices(), 3);
		EXPECT_EQ(storage.num_edges(), 3);
	}

	TEST(GraphStaticStorage, VertexProperties)
	{
		using namespace synodic::graph::storage;

		StaticStorage<VertexData, void, 4, 4> storage;
		auto v0 = storage.add_vertex(VertexData {10});
		auto v1 = storage.add_vertex(VertexData {20});

		EXPECT_EQ(storage[v0].id, 10);
		EXPECT_EQ(storage[v1].id, 20);
	}

	TEST(GraphStaticStorage, OutEdges)
	{
		using namespace synodic::graph::storage;

		StaticStorage<void, void, 4, 8> storage;
		auto v0 = storage.add_vertex();
		auto v1 = storage.add_vertex();
		auto v2 = storage.add_vertex();

		storage.add_edge(v0, v1);
		storage.add_edge(v0, v2);
		storage.add_edge(v1, v2);

		EXPECT_EQ(storage.out_degree(v0), 2);
		EXPECT_EQ(storage.out_degree(v1), 1);
		EXPECT_EQ(storage.out_degree(v2), 0);
	}

	TEST(GraphStaticStorage, BoundaryVertices)
	{
		using namespace synodic::graph::storage;

		StaticStorage<void, void, 4, 4> storage;
		auto v0 = storage.add_vertex();
		auto v1 = storage.add_vertex();
		auto v2 = storage.add_vertex();

		storage.mark_boundary(v0);
		storage.mark_boundary(v2);

		EXPECT_EQ(storage.num_boundary_vertices(), 2);

		auto boundaries = storage.boundary_vertices();
		EXPECT_EQ(boundaries[0], v0);
		EXPECT_EQ(boundaries[1], v2);
	}

	// ============================================================================
	// Adjacency List Storage Tests (runtime modification)
	// ============================================================================

	TEST(GraphAdjacencyList, AddRemoveVertices)
	{
		using namespace synodic::graph::storage;

		AdjacencyListStorage<VertexData, void> storage;

		auto v0 = storage.add_vertex(VertexData {1});
		auto v1 = storage.add_vertex(VertexData {2});
		auto v2 = storage.add_vertex(VertexData {3});

		EXPECT_EQ(storage.num_vertices(), 3);
		EXPECT_EQ(storage[v0].id, 1);

		storage.remove_vertex(v1);

		EXPECT_EQ(storage.num_vertices(), 2);
	}

	TEST(GraphAdjacencyList, AddRemoveEdges)
	{
		using namespace synodic::graph::storage;

		AdjacencyListStorage<void, EdgeData> storage;

		auto v0 = storage.add_vertex();
		auto v1 = storage.add_vertex();
		auto v2 = storage.add_vertex();

		auto [e01, ok1] = storage.add_edge(v0, v1, EdgeData {1.5f});
		auto [e12, ok2] = storage.add_edge(v1, v2, EdgeData {2.5f});

		EXPECT_TRUE(ok1);
		EXPECT_TRUE(ok2);
		EXPECT_EQ(storage.num_edges(), 2);
		EXPECT_EQ(storage[e01].weight, 1.5f);

		storage.remove_edge(v0, v1);

		EXPECT_EQ(storage.num_edges(), 1);
	}

	TEST(GraphAdjacencyList, AdjacencyTraversal)
	{
		using namespace synodic::graph::storage;

		AdjacencyListStorage<void, void> storage;

		auto v0 = storage.add_vertex();
		auto v1 = storage.add_vertex();
		auto v2 = storage.add_vertex();

		storage.add_edge(v0, v1);
		storage.add_edge(v0, v2);

		int neighbor_count = 0;
		for ([[maybe_unused]] auto neighbor: storage.adjacent_vertices(v0))
		{
			++neighbor_count;
		}

		EXPECT_EQ(neighbor_count, 2);
		EXPECT_EQ(storage.out_degree(v0), 2);
	}

	// ============================================================================
	// Graph Wrapper Tests
	// ============================================================================

	TEST(GraphWrapper, DynamicGraph)
	{
		using namespace synodic::graph;

		DynamicGraph<VertexData, EdgeData> graph;

		auto v0 = graph.add_vertex(VertexData {100});
		auto v1 = graph.add_vertex(VertexData {200});

		auto [edge, ok] = graph.add_edge(v0, v1, EdgeData {5.0f});

		EXPECT_TRUE(ok);
		EXPECT_EQ(graph.num_vertices(), 2);
		EXPECT_EQ(graph.num_edges(), 1);
		EXPECT_EQ(graph[v0].id, 100);
		EXPECT_EQ(graph[edge].weight, 5.0f);
		EXPECT_EQ(graph.source(edge), v0);
		EXPECT_EQ(graph.target(edge), v1);
	}

	TEST(GraphWrapper, ConceptSatisfaction)
	{
		using namespace synodic::graph;
		using namespace synodic::graph::storage;

		// Verify storage types satisfy the expected concepts
		static_assert(graph_storage<StaticStorage<void, void, 4, 4>>);
		static_assert(vertex_list_storage<StaticStorage<void, void, 4, 4>>);
		static_assert(incidence_storage<StaticStorage<void, void, 4, 4>>);
		static_assert(constexpr_storage<StaticStorage<void, void, 4, 4>>);

		static_assert(graph_storage<AdjacencyListStorage<void, void>>);
		static_assert(mutable_storage<AdjacencyListStorage<void, void>>);
		static_assert(vertex_list_storage<AdjacencyListStorage<void, void>>);

		EXPECT_TRUE(true);	// Static asserts passed
	}

	// ============================================================================
	// Subgraph Composition Tests
	// ============================================================================

	TEST(GraphComposition, ComposeStaticIntoDynamic)
	{
		using namespace synodic::graph;
		using namespace synodic::graph::storage;

		// Create a compile-time subgraph
		StaticStorage<VertexData, void, 4, 4> subgraph;
		auto s0 = subgraph.add_vertex(VertexData {1});
		auto s1 = subgraph.add_vertex(VertexData {2});
		subgraph.add_edge(s0, s1);
		subgraph.mark_boundary(s0);

		// Create runtime host graph
		AdjacencyListStorage<VertexData, void> host;
		auto h0 = host.add_vertex(VertexData {100});

		// Compose subgraph into host
		auto mapping = host.compose(subgraph);

		// Host should now have 3 vertices (1 original + 2 from subgraph)
		EXPECT_EQ(host.num_vertices(), 3);
		EXPECT_EQ(host.num_edges(), 1);

		// Verify handle mapping
		EXPECT_EQ(mapping.size(), 2);
		EXPECT_EQ(host[mapping[s0]].id, 1);
		EXPECT_EQ(host[mapping[s1]].id, 2);
	}

}  // namespace
