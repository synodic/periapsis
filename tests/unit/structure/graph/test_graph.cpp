import std;
import synodic.honesty.test;

import synodic.library;

using namespace honesty::test;
using namespace honesty::test::literals;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace
{
	// ============================================================================
	// Static Storage Tests (compile-time subgraphs)
	// ============================================================================

	Suite STATIC_STORAGE_SUITE(
		"graph_static_storage",
		[]() -> Generator
		{
			co_yield "constexpr_construction"_test = [](const Requirements& requirements)
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

				requirements.Expect(storage.num_vertices() == 3);
				requirements.Expect(storage.num_edges() == 3);
			};

			co_yield "vertex_properties"_test = [](const Requirements& requirements)
			{
				using namespace synodic::graph::storage;

				StaticStorage<VertexData, void, 4, 4> storage;
				auto v0 = storage.add_vertex(VertexData {10});
				auto v1 = storage.add_vertex(VertexData {20});

				requirements.Expect(storage[v0].id == 10);
				requirements.Expect(storage[v1].id == 20);
			};

			co_yield "out_edges"_test = [](const Requirements& requirements)
			{
				using namespace synodic::graph::storage;

				StaticStorage<void, void, 4, 8> storage;
				auto v0 = storage.add_vertex();
				auto v1 = storage.add_vertex();
				auto v2 = storage.add_vertex();

				storage.add_edge(v0, v1);
				storage.add_edge(v0, v2);
				storage.add_edge(v1, v2);

				requirements.Expect(storage.out_degree(v0) == 2);
				requirements.Expect(storage.out_degree(v1) == 1);
				requirements.Expect(storage.out_degree(v2) == 0);
			};

			co_yield "boundary_vertices"_test = [](const Requirements& requirements)
			{
				using namespace synodic::graph::storage;

				StaticStorage<void, void, 4, 4> storage;
				auto v0 = storage.add_vertex();
				auto v1 = storage.add_vertex();
				auto v2 = storage.add_vertex();

				storage.mark_boundary(v0);
				storage.mark_boundary(v2);

				requirements.Expect(storage.num_boundary_vertices() == 2);

				auto boundaries = storage.boundary_vertices();
				requirements.Expect(boundaries[0] == v0);
				requirements.Expect(boundaries[1] == v2);
			};
		});
	SuiteRegistrar _static(STATIC_STORAGE_SUITE);

	// ============================================================================
	// Adjacency List Storage Tests (runtime modification)
	// ============================================================================

	Suite ADJACENCY_LIST_SUITE(
		"graph_adjacency_list",
		[]() -> Generator
		{
			co_yield "add_remove_vertices"_test = [](const Requirements& requirements)
			{
				using namespace synodic::graph::storage;

				AdjacencyListStorage<VertexData, void> storage;

				auto v0 = storage.add_vertex(VertexData {1});
				auto v1 = storage.add_vertex(VertexData {2});
				auto v2 = storage.add_vertex(VertexData {3});

				requirements.Expect(storage.num_vertices() == 3);
				requirements.Expect(storage[v0].id == 1);

				storage.remove_vertex(v1);

				requirements.Expect(storage.num_vertices() == 2);
			};

			co_yield "add_remove_edges"_test = [](const Requirements& requirements)
			{
				using namespace synodic::graph::storage;

				AdjacencyListStorage<void, EdgeData> storage;

				auto v0 = storage.add_vertex();
				auto v1 = storage.add_vertex();
				auto v2 = storage.add_vertex();

				auto [e01, ok1] = storage.add_edge(v0, v1, EdgeData {1.5f});
				auto [e12, ok2] = storage.add_edge(v1, v2, EdgeData {2.5f});

				requirements.Expect(ok1);
				requirements.Expect(ok2);
				requirements.Expect(storage.num_edges() == 2);
				requirements.Expect(storage[e01].weight == 1.5f);

				storage.remove_edge(v0, v1);

				requirements.Expect(storage.num_edges() == 1);
			};

			co_yield "adjacency_traversal"_test = [](const Requirements& requirements)
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

				requirements.Expect(neighbor_count == 2);
				requirements.Expect(storage.out_degree(v0) == 2);
			};
		});
	SuiteRegistrar _adj(ADJACENCY_LIST_SUITE);

	// ============================================================================
	// Graph Wrapper Tests
	// ============================================================================

	Suite GRAPH_WRAPPER_SUITE(
		"graph_wrapper",
		[]() -> Generator
		{
			co_yield "dynamic_graph"_test = [](const Requirements& requirements)
			{
				using namespace synodic::graph;

				DynamicGraph<VertexData, EdgeData> graph;

				auto v0 = graph.add_vertex(VertexData {100});
				auto v1 = graph.add_vertex(VertexData {200});

				auto [edge, ok] = graph.add_edge(v0, v1, EdgeData {5.0f});

				requirements.Expect(ok);
				requirements.Expect(graph.num_vertices() == 2);
				requirements.Expect(graph.num_edges() == 1);
				requirements.Expect(graph[v0].id == 100);
				requirements.Expect(graph[edge].weight == 5.0f);
				requirements.Expect(graph.source(edge) == v0);
				requirements.Expect(graph.target(edge) == v1);
			};

			co_yield "concept_satisfaction"_test = [](const Requirements& requirements)
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

				requirements.Expect(true);	// Static asserts passed
			};
		});
	SuiteRegistrar _wrapper(GRAPH_WRAPPER_SUITE);

	// ============================================================================
	// Subgraph Composition Tests
	// ============================================================================

	Suite COMPOSITION_SUITE(
		"graph_composition",
		[]() -> Generator
		{
			co_yield "compose_static_into_dynamic"_test = [](const Requirements& requirements)
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
				requirements.Expect(host.num_vertices() == 3);
				requirements.Expect(host.num_edges() == 1);

				// Verify handle mapping
				requirements.Expect(mapping.size() == 2);
				requirements.Expect(host[mapping[s0]].id == 1);
				requirements.Expect(host[mapping[s1]].id == 2);
			};
		});
	SuiteRegistrar _comp(COMPOSITION_SUITE);
}
