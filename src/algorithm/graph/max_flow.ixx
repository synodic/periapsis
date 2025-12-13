export module synodic.periapsis:algorithm.graph.max_flow;

import :structure.graph.concepts;
import :structure.graph.storage.static_storage;
import :structure.graph.graph;
import :container.fixed.ring_buffer;

import std;

export namespace peri::algorithm::graph
{
	/**
	 * @brief Edge properties required for flow network algorithms.
	 *        Capacity is the maximum flow allowed, flow is the current flow.
	 */
	template<typename T> requires std::is_arithmetic_v<T>
	struct FlowEdge
	{
		T capacity = 0;
		T flow     = 0;

		[[nodiscard]] constexpr T Residual() const noexcept
		{
			return capacity - flow;
		}

		constexpr bool operator==(const FlowEdge&) const = default;
	};

	/**
	 * @brief Result of a max flow computation.
	 */
	template<typename T> requires std::is_arithmetic_v<T>
	struct MaxFlowResult
	{
		T maxFlow      = 0;
		bool foundPath = false;

		constexpr bool operator==(const MaxFlowResult&) const = default;
	};

	/**
	 * @brief Edmonds-Karp algorithm for maximum flow.
	 *
	 * @tparam VertexProperty The vertex property type
	 * @tparam EdgeProperty The edge property type
	 * @tparam MaxV Maximum vertices in graph
	 * @tparam MaxE Maximum edges in graph
	 * @param graph The flow network
	 * @param source Source vertex
	 * @param sink Sink vertex
	 * @return The maximum flow value
	 */
	template<
		typename VertexProperty,
		typename EdgeProperty,
		std::size_t MaxV,
		std::size_t MaxE>
	constexpr auto MaxFlow(
		peri::graph::Graph<VertexProperty, EdgeProperty, MaxV, MaxE>& graph,
		typename peri::graph::Graph<VertexProperty, EdgeProperty, MaxV, MaxE>::vertex_descriptor source,
		typename peri::graph::Graph<VertexProperty, EdgeProperty, MaxV, MaxE>::vertex_descriptor sink)
		-> MaxFlowResult<decltype(std::declval<EdgeProperty>().capacity)>
	{
		using graph_type        = peri::graph::Graph<VertexProperty, EdgeProperty, MaxV, MaxE>;
		using vertex_descriptor = graph_type::vertex_descriptor;
		using edge_descriptor   = graph_type::edge_descriptor;
		using flow_type         = decltype(std::declval<EdgeProperty>().capacity);

		MaxFlowResult<flow_type> result{};

		// Path reconstruction info
		struct PathInfo
		{
			vertex_descriptor parent{};
			edge_descriptor edge{};
			bool isForward = true;
			bool visited   = false;
		};

		std::array<PathInfo, MaxV> parent{};
		FixedRingBuffer<vertex_descriptor, MaxV> queue;

		// BFS to find augmenting path
		auto findAugmentingPath = [&]() constexpr -> bool
		{
			// Reset visited flags
			for (std::size_t i = 0; i < MaxV; ++i)
			{
				parent[i].visited = false;
			}
			queue.clear();

			queue.push_back(source);
			parent[source].visited = true;

			while (not queue.empty())
			{
				auto u = queue.front();
				queue.pop_front();

				if (u == sink)
				{
					return true;
				}

				// Forward edges
				for (auto e: graph.out_edges(u))
				{
					auto v = graph.target(e);
					if (not parent[v].visited and graph[e].Residual() > 0)
					{
						parent[v] = PathInfo{u, e, true, true};
						queue.push_back(v);
					}
				}

				// Check all edges for reverse direction
				// For graphs without bidirectional storage, we scan all edges
				for (auto e: graph.edges())
				{
					if (graph.target(e) == u)
					{
						auto v = graph.source(e);
						if (not parent[v].visited and graph[e].flow > 0)
						{
							parent[v] = PathInfo{u, e, false, true};
							queue.push_back(v);
						}
					}
				}
			}

			return false;
		};

		// Main loop: find augmenting paths and push flow
		while (findAugmentingPath())
		{
			result.foundPath = true;

			// Find bottleneck capacity along the path
			flow_type pathFlow = std::numeric_limits<flow_type>::max();

			for (auto v = sink; v != source;)
			{
				const auto& info = parent[v];
				if (info.isForward)
				{
					pathFlow = std::min(pathFlow, graph[info.edge].Residual());
				}
				else
				{
					pathFlow = std::min(pathFlow, graph[info.edge].flow);
				}
				v = info.parent;
			}

			// Augment flow along the path
			for (auto v = sink; v != source;)
			{
				const auto& info = parent[v];
				if (info.isForward)
				{
					graph[info.edge].flow += pathFlow;
				}
				else
				{
					graph[info.edge].flow -= pathFlow;
				}
				v = info.parent;
			}

			result.maxFlow += pathFlow;
		}

		return result;
	}
}
