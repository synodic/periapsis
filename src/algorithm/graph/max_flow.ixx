export module synodic.library:algorithm.graph.max_flow;

import :structure.graph.concepts;
import :structure.graph.storage.static_storage;
import :structure.graph.graph;
import :container.fixed.ring_buffer;

import std;

export namespace synodic::algorithm::graph
{
	// Bring in constexpr container
	using synodic::FixedRingBuffer;

	/**
	 * @brief Edge properties required for flow network algorithms.
	 *        Capacity is the maximum flow allowed, flow is the current flow.
	 */
	template<typename T = int>
	struct FlowEdge
	{
		T capacity {};
		T flow {};

		[[nodiscard]] constexpr T residual() const noexcept
		{
			return capacity - flow;
		}

		constexpr bool operator==(const FlowEdge&) const = default;
	};

	/**
	 * @brief Result of a max flow computation.
	 */
	template<typename T>
	struct MaxFlowResult
	{
		T max_flow {};
		bool found_path {false};

		constexpr bool operator==(const MaxFlowResult&) const = default;
	};

	/**
	 * @brief Edmonds-Karp algorithm for maximum flow (BFS-based Ford-Fulkerson).
	 *        Constexpr-compatible version for static graphs.
	 *        Time complexity: O(V * E^2)
	 *
	 * @tparam MaxVertices Maximum number of vertices (for constexpr array allocation)
	 * @tparam Storage The graph storage type
	 * @param graph The flow network (modified in place with flow values)
	 * @param source Source vertex
	 * @param sink Sink vertex
	 * @return The maximum flow value
	 */
	template<std::size_t MaxVertices, typename Storage>
		requires synodic::graph::incidence_storage<Storage> and synodic::graph::vertex_list_storage<Storage> and
				 synodic::graph::edge_list_storage<Storage>
	constexpr auto max_flow(
		synodic::graph::Graph<Storage>& graph,
		typename Storage::vertex_descriptor source,
		typename Storage::vertex_descriptor sink)
		-> MaxFlowResult<decltype(std::declval<typename Storage::edge_property_type>().capacity)>
	{
		using vertex_descriptor = typename Storage::vertex_descriptor;
		using edge_descriptor	= typename Storage::edge_descriptor;
		using flow_type			= decltype(std::declval<typename Storage::edge_property_type>().capacity);

		MaxFlowResult<flow_type> result {};

		// Path reconstruction info
		struct PathInfo
		{
			vertex_descriptor parent {};
			edge_descriptor edge {};
			bool is_forward {true};
			bool visited {false};
		};

		std::array<PathInfo, MaxVertices> parent {};
		FixedRingBuffer<vertex_descriptor, MaxVertices> queue;

		// BFS to find augmenting path
		auto find_augmenting_path = [&]() constexpr -> bool
		{
			// Reset visited flags
			for (std::size_t i = 0; i < MaxVertices; ++i)
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

				// Forward edges (u -> v with positive residual capacity)
				for (auto e: graph.out_edges(u))
				{
					auto v = graph.target(e);
					if (not parent[v].visited and graph[e].residual() > 0)
					{
						parent[v] = PathInfo {u, e, true, true};
						queue.push_back(v);
					}
				}

				// Check all edges for reverse direction (v -> u where we can push flow back)
				// For graphs without bidirectional storage, we scan all edges
				for (auto e: graph.edges())
				{
					if (graph.target(e) == u)
					{
						auto v = graph.source(e);
						if (not parent[v].visited and graph[e].flow > 0)
						{
							parent[v] = PathInfo {u, e, false, true};
							queue.push_back(v);
						}
					}
				}
			}

			return false;
		};

		// Main loop: find augmenting paths and push flow
		while (find_augmenting_path())
		{
			result.found_path = true;

			// Find bottleneck capacity along the path
			flow_type path_flow = std::numeric_limits<flow_type>::max();

			for (auto v = sink; v != source;)
			{
				const auto& info = parent[v];
				if (info.is_forward)
				{
					path_flow = std::min(path_flow, graph[info.edge].residual());
				}
				else
				{
					path_flow = std::min(path_flow, graph[info.edge].flow);
				}
				v = info.parent;
			}

			// Augment flow along the path
			for (auto v = sink; v != source;)
			{
				const auto& info = parent[v];
				if (info.is_forward)
				{
					graph[info.edge].flow += path_flow;
				}
				else
				{
					graph[info.edge].flow -= path_flow;
				}
				v = info.parent;
			}

			result.max_flow += path_flow;
		}

		return result;
	}

	/**
	 * @brief Helper to deduce MaxVertices from static storage.
	 */
	template<
		typename VertexProperty,
		typename EdgeProperty,
		std::size_t MaxVertices,
		std::size_t MaxEdges,
		std::unsigned_integral IndexType>
	constexpr auto max_flow(
		synodic::graph::Graph<
			synodic::graph::storage::StaticStorage<VertexProperty, EdgeProperty, MaxVertices, MaxEdges, IndexType>>&
			graph,
		IndexType source,
		IndexType sink)
	{
		return max_flow<MaxVertices>(graph, source, sink);
	}
}
