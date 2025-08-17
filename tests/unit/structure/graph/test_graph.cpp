import std;
import synodic.honesty.test;

import synodic.library.algorithm.graph;
import synodic.library.structure;

using namespace honesty::test;
using namespace honesty::test::literals;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VertexData
{
	bool data = true;
};

struct EdgeData
{
	float capacity = 3;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace
{
	Suite SUITE(
		"graph",
		[]() -> Generator
		{
			co_yield "builder"_test = [](const Requirements& requirements)
			{
				synodic::graph::directed::GraphBuilder<std::int32_t, std::int32_t> builder;

				// builder.PushNode();

				const auto graph = builder.Build();

				requirements.ExpectFalse(graph.EdgeCount());
			};

			co_yield "static"_test = [](const Requirements& requirements)
			{
				using namespace synodic::graph;

				// The test data representation for the vertex range
				constexpr std::array<VertexData, 3> vertexData;

				// The test data representation for the edge range
				std::array<EdgeData, 3> edgeDate;

				// TODO: Temporary. All edge-vertex connections are consumed by the graph structure.
				// TODO: Edges should be index or value based.

				//constexpr directed::StaticGraph graph(vertices, edges);

				//EXPECT_FALSE(graph.EdgeCount());
			};

			co_yield "max_flow"_test = [](const Requirements& requirements)
			{
				using namespace synodic::graph;
				using namespace synodic::algorithm::graph;

				// constexpr std::array edges {Edge8(0, 1), Edge8(1, 2), Edge8(0, 2)};
				// constexpr directed::StaticGraph<std::int8_t, std::int8_t, 3, 3> graph(edges);

				// MaxFlow(graph);
			};
		}
		);
	SuiteRegistrar _(SUITE);
}
