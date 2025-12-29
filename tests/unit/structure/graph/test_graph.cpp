/**
 * @brief Unit tests for graph data structure
 */

#include <gtest/gtest.h>

import std;
import synodic.periapsis;

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
		using namespace peri::graph;
		using namespace peri::graph::storage;

		// Verify constexpr construction works
		constexpr auto make_graph = []() constexpr
		{
			StaticStorage<void, void, 4, 4> storage;
			auto v0 = storage.AddVertex();
			auto v1 = storage.AddVertex();
			auto v2 = storage.AddVertex();
			(void)storage.AddEdge(v0, v1);
			(void)storage.AddEdge(v1, v2);
			(void)storage.AddEdge(v0, v2);
			return storage;
		};

		constexpr auto storage = make_graph();

		static_assert(storage.NumVertices() == 3);
		static_assert(storage.NumEdges() == 3);

		EXPECT_EQ(storage.NumVertices(), 3);
		EXPECT_EQ(storage.NumEdges(), 3);
	}

	TEST(GraphStaticStorage, VertexProperties)
	{
		using namespace peri::graph::storage;

		StaticStorage<VertexData, void, 4, 4> storage;
		auto v0 = storage.AddVertex(VertexData {10});
		auto v1 = storage.AddVertex(VertexData {20});

		EXPECT_EQ(storage[v0].id, 10);
		EXPECT_EQ(storage[v1].id, 20);
	}

	TEST(GraphStaticStorage, OutEdges)
	{
		using namespace peri::graph::storage;

		StaticStorage<void, void, 4, 8> storage;
		auto v0 = storage.AddVertex();
		auto v1 = storage.AddVertex();
		auto v2 = storage.AddVertex();

		auto [e01, ok1] = storage.AddEdge(v0, v1);
		EXPECT_TRUE(ok1);
		auto [e02, ok2] = storage.AddEdge(v0, v2);
		EXPECT_TRUE(ok2);
		auto [e12, ok3] = storage.AddEdge(v1, v2);
		EXPECT_TRUE(ok3);

		EXPECT_EQ(storage.OutDegree(v0), 2);
		EXPECT_EQ(storage.OutDegree(v1), 1);
		EXPECT_EQ(storage.OutDegree(v2), 0);
	}

	TEST(GraphStaticStorage, BoundaryVertices)
	{
		using namespace peri::graph::storage;

		StaticStorage<void, void, 4, 4> storage;
		auto v0 = storage.AddVertex();
		auto v1 = storage.AddVertex();
		auto v2 = storage.AddVertex();

		storage.MarkBoundary(v0);
		storage.MarkBoundary(v2);

		EXPECT_EQ(storage.NumBoundaryVertices(), 2);

		auto boundaries = storage.BoundaryVertices();
		EXPECT_EQ(boundaries[0], v0);
		EXPECT_EQ(boundaries[1], v2);
	}

	// ============================================================================
	// Adjacency List Storage Tests (runtime modification)
	// ============================================================================

	TEST(GraphAdjacencyList, AddRemoveVertices)
	{
		using namespace peri::graph::storage;

		AdjacencyListStorage<VertexData, void> storage;

		auto v0 = storage.AddVertex(VertexData {1});
		auto v1 = storage.AddVertex(VertexData {2});
		auto v2 = storage.AddVertex(VertexData {3});

		EXPECT_EQ(storage.NumVertices(), 3);
		EXPECT_EQ(storage[v0].id, 1);

		storage.RemoveVertex(v1);

		EXPECT_EQ(storage.NumVertices(), 2);
	}

	TEST(GraphAdjacencyList, AddRemoveEdges)
	{
		using namespace peri::graph::storage;

		AdjacencyListStorage<void, EdgeData> storage;

		auto v0 = storage.AddVertex();
		auto v1 = storage.AddVertex();
		auto v2 = storage.AddVertex();

		auto [e01, ok1] = storage.AddEdge(v0, v1, EdgeData {1.5f});
		auto [e12, ok2] = storage.AddEdge(v1, v2, EdgeData {2.5f});

		EXPECT_TRUE(ok1);
		EXPECT_TRUE(ok2);
		EXPECT_EQ(storage.NumEdges(), 2);
		EXPECT_EQ(storage[e01].weight, 1.5f);

		storage.RemoveEdge(v0, v1);

		EXPECT_EQ(storage.NumEdges(), 1);
	}

	TEST(GraphAdjacencyList, AdjacencyTraversal)
	{
		using namespace peri::graph::storage;

		AdjacencyListStorage<void, void> storage;

		auto v0 = storage.AddVertex();
		auto v1 = storage.AddVertex();
		auto v2 = storage.AddVertex();

		auto [e01, ok1] = storage.AddEdge(v0, v1);
		EXPECT_TRUE(ok1);
		auto [e02, ok2] = storage.AddEdge(v0, v2);
		EXPECT_TRUE(ok2);

		int neighbor_count = 0;
		for ([[maybe_unused]] auto neighbor: storage.AdjacentVertices(v0))
		{
			++neighbor_count;
		}

		EXPECT_EQ(neighbor_count, 2);
		EXPECT_EQ(storage.OutDegree(v0), 2);
	}

	// ============================================================================
	// Graph Wrapper Tests
	// ============================================================================

	TEST(GraphWrapper, DynamicGraph)
	{
		using namespace peri::graph;

		DynamicGraph<VertexData, EdgeData> graph;

		auto v0 = graph.AddVertex(VertexData {100});
		auto v1 = graph.AddVertex(VertexData {200});

		auto [edge, ok] = graph.AddEdge(v0, v1, EdgeData {5.0f});

		EXPECT_TRUE(ok);
		EXPECT_EQ(graph.NumVertices(), 2);
		EXPECT_EQ(graph.NumEdges(), 1);
		EXPECT_EQ(graph[v0].id, 100);
		EXPECT_EQ(graph[edge].weight, 5.0f);
		EXPECT_EQ(graph.Source(edge), v0);
		EXPECT_EQ(graph.Target(edge), v1);
	}

	TEST(GraphWrapper, ConceptSatisfaction)
	{
		using namespace peri::graph;
		using namespace peri::graph::storage;

		// Verify storage types satisfy the expected concepts
		static_assert(graph_storage<StaticStorage<void, void, 4, 4>>);
		static_assert(vertex_list_storage<StaticStorage<void, void, 4, 4>>);
		static_assert(incidence_storage<StaticStorage<void, void, 4, 4>>);

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
		using namespace peri::graph;
		using namespace peri::graph::storage;

		// Create a compile-time subgraph
		StaticStorage<VertexData, void, 4, 4> subgraph;
		auto s0			   = subgraph.AddVertex(VertexData {1});
		auto s1			   = subgraph.AddVertex(VertexData {2});
		auto [es01, oks01] = subgraph.AddEdge(s0, s1);
		EXPECT_TRUE(oks01);
		subgraph.MarkBoundary(s0);

		// Create runtime host graph
		AdjacencyListStorage<VertexData, void> host;
		auto h0 = host.AddVertex(VertexData {100});

		// Compose subgraph into host
		auto mapping = host.Compose(subgraph);

		// Host should now have 3 vertices (1 original + 2 from subgraph)
		EXPECT_EQ(host.NumVertices(), 3);
		EXPECT_EQ(host.NumEdges(), 1);

		// Verify handle mapping
		EXPECT_EQ(mapping.size(), 2);
		EXPECT_EQ(host[mapping[s0]].id, 1);
		EXPECT_EQ(host[mapping[s1]].id, 2);
	}

	// ============================================================================
	// Bidirectional Storage Tests
	// ============================================================================

	TEST(BidirectionalStorage, AdjacencyListInEdges)
	{
		using namespace peri::graph::storage;

		// Create bidirectional adjacency list
		AdjacencyListStorage<void, void, std::vector, std::vector, std::size_t, true> storage;

		auto v0 = storage.AddVertex();
		auto v1 = storage.AddVertex();
		auto v2 = storage.AddVertex();

		// Create edges: v0 -> v1 -> v2, v0 -> v2
		auto [e01, ok1] = storage.AddEdge(v0, v1);
		EXPECT_TRUE(ok1);
		auto [e12, ok2] = storage.AddEdge(v1, v2);
		EXPECT_TRUE(ok2);
		auto [e02, ok3] = storage.AddEdge(v0, v2);
		EXPECT_TRUE(ok3);

		// Check out-degrees
		EXPECT_EQ(storage.OutDegree(v0), 2);
		EXPECT_EQ(storage.OutDegree(v1), 1);
		EXPECT_EQ(storage.OutDegree(v2), 0);

		// Check in-degrees
		EXPECT_EQ(storage.InDegree(v0), 0);
		EXPECT_EQ(storage.InDegree(v1), 1);
		EXPECT_EQ(storage.InDegree(v2), 2);

		// Verify in_edges for v2 (should have edges from v0 and v1)
		std::vector<std::size_t> sources;
		for (auto e: storage.InEdges(v2))
		{
			sources.push_back(storage.Source(e));
		}
		EXPECT_EQ(sources.size(), 2);
		EXPECT_TRUE(std::ranges::find(sources, v0) != sources.end());
		EXPECT_TRUE(std::ranges::find(sources, v1) != sources.end());
	}

	TEST(BidirectionalStorage, ConceptSatisfaction)
	{
		using namespace peri::graph;
		using namespace peri::graph::storage;

		// Non-bidirectional should not satisfy bidirectional_storage
		static_assert(!bidirectional_storage<AdjacencyListStorage<void, void>>);
		static_assert(!bidirectional_storage<StaticStorage<void, void, 4, 4>>);

		// Bidirectional versions should satisfy bidirectional_storage
		static_assert(
			bidirectional_storage<AdjacencyListStorage<void, void, std::vector, std::vector, std::size_t, true>>);
		static_assert(bidirectional_storage<BidirectionalFixed<void, void, 4, 4>>);

		// All should still satisfy basic concepts
		static_assert(incidence_storage<AdjacencyListStorage<void, void, std::vector, std::vector, std::size_t, true>>);
		static_assert(incidence_storage<BidirectionalFixed<void, void, 4, 4>>);

		EXPECT_TRUE(true);	// Static asserts passed
	}

	TEST(BidirectionalStorage, BidirectionalFixedConstexpr)
	{
		using namespace peri::graph::storage;

		constexpr auto make_graph = []() constexpr
		{
			BidirectionalFixed<void, void, 4, 6> graph;
			auto v0 = graph.AddVertex();
			auto v1 = graph.AddVertex();
			auto v2 = graph.AddVertex();
			(void)graph.AddEdge(v0, v1);
			(void)graph.AddEdge(v1, v2);
			(void)graph.AddEdge(v0, v2);
			return graph;
		};

		constexpr auto graph = make_graph();

		static_assert(graph.NumVertices() == 3);
		static_assert(graph.NumEdges() == 3);
		static_assert(graph.OutDegree(0) == 2);
		static_assert(graph.InDegree(2) == 2);

		EXPECT_EQ(graph.NumVertices(), 3);
		EXPECT_EQ(graph.InDegree(2), 2);
	}

	TEST(BidirectionalStorage, BidirectionalFixedInEdges)
	{
		using namespace peri::graph::storage;

		BidirectionalFixed<void, void, 5, 8> graph;

		auto v0 = graph.AddVertex();
		auto v1 = graph.AddVertex();
		auto v2 = graph.AddVertex();
		auto v3 = graph.AddVertex();

		// Create a diamond: v0 -> v1 -> v3, v0 -> v2 -> v3
		auto [e01, ok1] = graph.AddEdge(v0, v1);
		EXPECT_TRUE(ok1);
		auto [e02, ok2] = graph.AddEdge(v0, v2);
		EXPECT_TRUE(ok2);
		auto [e13, ok3] = graph.AddEdge(v1, v3);
		EXPECT_TRUE(ok3);
		auto [e23, ok4] = graph.AddEdge(v2, v3);
		EXPECT_TRUE(ok4);

		// v3 has two incoming edges
		EXPECT_EQ(graph.InDegree(v3), 2);

		// Check predecessor vertices
		std::vector<std::size_t> preds;
		for (auto v: graph.PredecessorVertices(v3))
		{
			preds.push_back(v);
		}
		EXPECT_EQ(preds.size(), 2);
		EXPECT_TRUE(std::ranges::find(preds, v1) != preds.end());
		EXPECT_TRUE(std::ranges::find(preds, v2) != preds.end());
	}

	// ============================================================================
	// Bidirectional Graph Wrapper Tests
	// ============================================================================

	TEST(BidirectionalGraph, DirectionalityEnum)
	{
		using namespace peri::graph;

		// Default graph is directed
		using DirectedGraph = Graph<void, void>;
		static_assert(DirectedGraph::DIRECTIONALITY == Directionality::DIRECTED);
		static_assert(!DirectedGraph::IS_BIDIRECTIONAL);

		// Bidirectional graph
		using BiGraph = BidirectionalGraph<void, void>;
		static_assert(BiGraph::DIRECTIONALITY == Directionality::BIDIRECTIONAL);
		static_assert(BiGraph::IS_BIDIRECTIONAL);

		EXPECT_TRUE(true);	// Static asserts passed
	}

	TEST(BidirectionalGraph, InEdgeTraversal)
	{
		using namespace peri::graph;

		BidirectionalGraph<VertexData, EdgeData> graph;

		auto v0 = graph.AddVertex(VertexData {0});
		auto v1 = graph.AddVertex(VertexData {1});
		auto v2 = graph.AddVertex(VertexData {2});

		auto [e02, ok1] = graph.AddEdge(v0, v2, EdgeData {1.0f});
		EXPECT_TRUE(ok1);
		auto [e12, ok2] = graph.AddEdge(v1, v2, EdgeData {2.0f});
		EXPECT_TRUE(ok2);

		// v2 should have 2 in-edges
		EXPECT_EQ(graph.InDegree(v2), 2);

		// Traverse in-edges and collect sources
		std::vector<std::size_t> sources;
		for (auto e: graph.InEdges(v2))
		{
			sources.push_back(graph.Source(e));
		}
		EXPECT_EQ(sources.size(), 2);
	}

	// ============================================================================
	// Bidirectional Visitor Tests
	// ============================================================================

	TEST(BidirectionalVisitor, BackwardTraversal)
	{
		using namespace peri::graph;

		BidirectionalGraph<void, void> graph;

		auto v0 = graph.AddVertex();
		auto v1 = graph.AddVertex();
		auto v2 = graph.AddVertex();

		auto [e01, ok1] = graph.AddEdge(v0, v1);
		EXPECT_TRUE(ok1);
		auto [e12, ok2] = graph.AddEdge(v1, v2);
		EXPECT_TRUE(ok2);

		// Create bidirectional visitor starting at v2
		auto visitor = make_bidirectional_visitor(graph, v2);

		EXPECT_TRUE(visitor.is_positioned());
		EXPECT_EQ(visitor.position(), v2);

		// Should be able to go backward to v1
		EXPECT_TRUE(visitor.can_advance_backward());
		auto result = visitor.advance_backward();
		EXPECT_EQ(result, AdvanceResult::MOVED);
		EXPECT_EQ(visitor.position(), v1);

		// Continue backward to v0
		result = visitor.advance_backward();
		EXPECT_EQ(result, AdvanceResult::MOVED);
		EXPECT_EQ(visitor.position(), v0);

		// No more backward edges
		EXPECT_FALSE(visitor.can_advance_backward());
	}

	TEST(BidirectionalVisitor, ForwardAndBackward)
	{
		using namespace peri::graph;

		BidirectionalGraph<void, void> graph;

		auto v0 = graph.AddVertex();
		auto v1 = graph.AddVertex();
		auto v2 = graph.AddVertex();

		auto [e01, ok1] = graph.AddEdge(v0, v1);
		EXPECT_TRUE(ok1);
		auto [e12, ok2] = graph.AddEdge(v1, v2);
		EXPECT_TRUE(ok2);

		auto visitor = make_bidirectional_visitor(graph, v1);

		// Can go forward to v2
		EXPECT_TRUE(visitor.can_advance_forward());
		visitor.advance_forward();
		EXPECT_EQ(visitor.position(), v2);

		// Go back to v1
		visitor.advance_backward();
		EXPECT_EQ(visitor.position(), v1);

		// Go back to v0
		visitor.advance_backward();
		EXPECT_EQ(visitor.position(), v0);

		// Go forward to v1 again
		visitor.advance_forward();
		EXPECT_EQ(visitor.position(), v1);
	}

	TEST(BidirectionalVisitor, DirectionParameter)
	{
		using namespace peri::graph;

		BidirectionalGraph<void, void> graph;

		auto v0 = graph.AddVertex();
		auto v1 = graph.AddVertex();

		auto [edge, ok] = graph.AddEdge(v0, v1);
		EXPECT_TRUE(ok);

		auto visitor = make_bidirectional_visitor(graph, v0);

		// Advance with direction parameter
		visitor.advance(TraversalDirection::FORWARD);
		EXPECT_EQ(visitor.position(), v1);

		visitor.advance(TraversalDirection::BACKWARD);
		EXPECT_EQ(visitor.position(), v0);
	}

	TEST(BidirectionalVisitor, DegreeQueries)
	{
		using namespace peri::graph;

		BidirectionalGraph<void, void> graph;

		auto v0 = graph.AddVertex();
		auto v1 = graph.AddVertex();
		auto v2 = graph.AddVertex();

		auto [e01, ok1] = graph.AddEdge(v0, v1);
		EXPECT_TRUE(ok1);
		auto [e02, ok2] = graph.AddEdge(v0, v2);
		EXPECT_TRUE(ok2);
		auto [e12, ok3] = graph.AddEdge(v1, v2);
		EXPECT_TRUE(ok3);

		auto visitor = make_bidirectional_visitor(graph, v0);

		// v0 has out_degree=2, in_degree=0
		EXPECT_EQ(visitor.OutDegree(), 2);
		EXPECT_EQ(visitor.InDegree(), 0);

		visitor.move_to(v2);

		// v2 has out_degree=0, in_degree=2
		EXPECT_EQ(visitor.OutDegree(), 0);
		EXPECT_EQ(visitor.InDegree(), 2);
	}

	// ============================================================================
	// Undirected Graph Tests
	// ============================================================================

	TEST(UndirectedGraph, DynamicBasicOperations)
	{
		using namespace peri::graph;

		UndirectedGraph<void, void> graph;

		auto v0 = graph.AddVertex();
		auto v1 = graph.AddVertex();
		auto v2 = graph.AddVertex();

		EXPECT_EQ(graph.NumVertices(), 3);

		auto [e01, ok1] = graph.AddEdge(v0, v1);
		auto [e12, ok2] = graph.AddEdge(v1, v2);
		auto [e02, ok3] = graph.AddEdge(v0, v2);

		EXPECT_TRUE(ok1);
		EXPECT_TRUE(ok2);
		EXPECT_TRUE(ok3);
		EXPECT_EQ(graph.NumEdges(), 3);
	}

	TEST(UndirectedGraph, DegreeAndIncidence)
	{
		using namespace peri::graph;

		UndirectedGraph<void, void> graph;

		auto v0 = graph.AddVertex();
		auto v1 = graph.AddVertex();
		auto v2 = graph.AddVertex();
		auto v3 = graph.AddVertex();

		// Create a star topology with v0 at center
		auto [e01, ok1] = graph.AddEdge(v0, v1);
		EXPECT_TRUE(ok1);
		auto [e02, ok2] = graph.AddEdge(v0, v2);
		EXPECT_TRUE(ok2);
		auto [e03, ok3] = graph.AddEdge(v0, v3);
		EXPECT_TRUE(ok3);

		// v0 is connected to 3 vertices
		EXPECT_EQ(graph.Degree(v0), 3);

		// v1, v2, v3 are only connected to v0
		EXPECT_EQ(graph.Degree(v1), 1);
		EXPECT_EQ(graph.Degree(v2), 1);
		EXPECT_EQ(graph.Degree(v3), 1);

		// Check incident edges
		auto incident = graph.IncidentEdges(v0);
		EXPECT_EQ(std::ranges::distance(incident), 3);
	}

	TEST(UndirectedGraph, OppositeEndpoint)
	{
		using namespace peri::graph;

		UndirectedGraph<void, void> graph;

		auto v0 = graph.AddVertex();
		auto v1 = graph.AddVertex();

		auto [edge, ok] = graph.AddEdge(v0, v1);
		EXPECT_TRUE(ok);

		// From v0, opposite should be v1
		EXPECT_EQ(graph.Opposite(edge, v0), v1);

		// From v1, opposite should be v0
		EXPECT_EQ(graph.Opposite(edge, v1), v0);

		// Endpoint accessors
		EXPECT_EQ(graph.Endpoint1(edge), v0);
		EXPECT_EQ(graph.Endpoint2(edge), v1);
	}

	TEST(UndirectedGraph, SelfLoop)
	{
		using namespace peri::graph;

		UndirectedGraph<void, void> graph;

		auto v0 = graph.AddVertex();

		auto [edge, ok] = graph.AddEdge(v0, v0);
		EXPECT_TRUE(ok);

		// Self-loop should have degree 1 (stored once in incident list)
		EXPECT_EQ(graph.Degree(v0), 1);

		// Both endpoints are the same
		EXPECT_EQ(graph.Endpoint1(edge), v0);
		EXPECT_EQ(graph.Endpoint2(edge), v0);
	}

	TEST(UndirectedGraph, WithProperties)
	{
		using namespace peri::graph;

		UndirectedGraph<VertexData, EdgeData> graph;

		auto v0 = graph.AddVertex(VertexData {1});
		auto v1 = graph.AddVertex(VertexData {2});

		auto [edge, ok] = graph.AddEdge(v0, v1, EdgeData {1.5f});
		EXPECT_TRUE(ok);

		// Use VertexProperty/EdgeProperty to avoid operator[] ambiguity
		EXPECT_EQ(graph.VertexProperty(v0).id, 1);
		EXPECT_EQ(graph.VertexProperty(v1).id, 2);
		EXPECT_EQ(graph.EdgeProperty(edge).weight, 1.5f);
	}

	TEST(UndirectedGraph, FixedConstexpr)
	{
		using namespace peri::graph;

		constexpr auto make_triangle = []() constexpr
		{
			UndirectedFixedGraph<void, void, 3, 3> graph;
			auto v0 = graph.AddVertex();
			auto v1 = graph.AddVertex();
			auto v2 = graph.AddVertex();
			(void)graph.AddEdge(v0, v1);
			(void)graph.AddEdge(v1, v2);
			(void)graph.AddEdge(v0, v2);
			return graph;
		};

		constexpr auto graph = make_triangle();

		static_assert(graph.NumVertices() == 3);
		static_assert(graph.NumEdges() == 3);
		static_assert(graph.IS_UNDIRECTED);

		EXPECT_EQ(graph.NumVertices(), 3);
		EXPECT_EQ(graph.NumEdges(), 3);
	}

	TEST(UndirectedGraph, FixedDegreeAndOpposite)
	{
		using namespace peri::graph;

		UndirectedFixedGraph<void, void, 4, 6> graph;

		auto v0 = graph.AddVertex();
		auto v1 = graph.AddVertex();
		auto v2 = graph.AddVertex();
		auto v3 = graph.AddVertex();

		// Complete graph K4
		auto [e01, ok1] = graph.AddEdge(v0, v1);
		EXPECT_TRUE(ok1);
		auto [e02, ok2] = graph.AddEdge(v0, v2);
		EXPECT_TRUE(ok2);
		auto [e03, ok3] = graph.AddEdge(v0, v3);
		EXPECT_TRUE(ok3);
		auto [e12, ok4] = graph.AddEdge(v1, v2);
		EXPECT_TRUE(ok4);
		auto [e13, ok5] = graph.AddEdge(v1, v3);
		EXPECT_TRUE(ok5);
		auto [e23, ok6] = graph.AddEdge(v2, v3);
		EXPECT_TRUE(ok6);

		// Each vertex in K4 has degree 3
		EXPECT_EQ(graph.Degree(v0), 3);
		EXPECT_EQ(graph.Degree(v1), 3);
		EXPECT_EQ(graph.Degree(v2), 3);
		EXPECT_EQ(graph.Degree(v3), 3);

		// Test opposite
		EXPECT_EQ(graph.Opposite(e01, v0), v1);
		EXPECT_EQ(graph.Opposite(e01, v1), v0);
		EXPECT_EQ(graph.Opposite(e02, v0), v2);
		EXPECT_EQ(graph.Opposite(e03, v3), v0);
	}

	TEST(UndirectedGraph, AdjacentVerticesTraversal)
	{
		using namespace peri::graph;

		UndirectedGraph<void, void> graph;

		auto v0 = graph.AddVertex();
		auto v1 = graph.AddVertex();
		auto v2 = graph.AddVertex();
		auto v3 = graph.AddVertex();

		// v0 connects to v1, v2, v3
		auto [e01, ok1] = graph.AddEdge(v0, v1);
		EXPECT_TRUE(ok1);
		auto [e02, ok2] = graph.AddEdge(v0, v2);
		EXPECT_TRUE(ok2);
		auto [e03, ok3] = graph.AddEdge(v0, v3);
		EXPECT_TRUE(ok3);

		// Traverse neighbors via incident edges
		std::vector<decltype(v0)> neighbors;
		for (auto e: graph.IncidentEdges(v0))
		{
			neighbors.push_back(graph.Opposite(e, v0));
		}

		EXPECT_EQ(neighbors.size(), 3);
		EXPECT_TRUE(std::ranges::find(neighbors, v1) != neighbors.end());
		EXPECT_TRUE(std::ranges::find(neighbors, v2) != neighbors.end());
		EXPECT_TRUE(std::ranges::find(neighbors, v3) != neighbors.end());
	}

	TEST(UndirectedGraph, DirectedCompatibility)
	{
		using namespace peri::graph;

		// Undirected graphs provide source/target for directed algorithm compatibility
		UndirectedGraph<void, void> graph;

		auto v0 = graph.AddVertex();
		auto v1 = graph.AddVertex();

		auto [edge, ok] = graph.AddEdge(v0, v1);
		EXPECT_TRUE(ok);

		// source/target map to endpoint1/endpoint2 for compatibility
		EXPECT_EQ(graph.Source(edge), v0);
		EXPECT_EQ(graph.Target(edge), v1);

		// out_edges and out_degree delegate to incident_edges/degree
		EXPECT_EQ(graph.OutDegree(v0), graph.Degree(v0));
	}
}
