export module synodic.periapsis:algorithm.graph.max_flow;

import :structure.graph.concepts;
import :structure.graph.storage.static_storage;
import :structure.graph.storage.bidirectional_fixed;
import :structure.graph.graph;
import :container.fixed.ring_buffer;

import std;

export namespace peri::algorithm::graph
{
	/**
	 * @brief Edge properties required for flow network algorithms.
	 *        Capacity is the maximum flow allowed, flow is the current flow.
	 */
	template<typename T>
		requires std::is_arithmetic_v<T>
	struct FlowEdge
	{
		T capacity = 0;
		T flow	   = 0;

		[[nodiscard]] constexpr T Residual() const noexcept
		{
			return capacity - flow;
		}

		constexpr bool operator==(const FlowEdge&) const = default;
	};

	/**
	 * @brief Result of a max flow computation.
	 */
	template<typename T>
		requires std::is_arithmetic_v<T>
	struct MaxFlowResult
	{
		T maxFlow	   = 0;
		bool foundPath = false;

		constexpr bool operator==(const MaxFlowResult&) const = default;
	};

	namespace detail
	{
		/**
		 * @brief BFS helper that uses in_edges() for efficient reverse traversal
		 *        when the graph supports bidirectional storage.
		 */
		template<typename GraphType, std::size_t MaxV, typename PathInfo, typename Queue>
		constexpr auto find_augmenting_path_bidirectional(
			GraphType& graph,
			typename GraphType::VertexDescriptor source,
			typename GraphType::VertexDescriptor sink,
			std::array<PathInfo, MaxV>& parent,
			Queue& queue) -> bool
			requires peri::graph::bidirectional_storage<typename GraphType::StorageType>
		{
			using VertexDescriptor = typename GraphType::VertexDescriptor;

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

				// Forward edges - O(out_degree)
				for (auto e: graph.OutEdges(u))
				{
					auto v = graph.Target(e);
					if (not parent[v].visited and graph.EdgeProperty(e).Residual() > 0)
					{
						parent[v] = PathInfo {u, e, true, true};
						queue.push_back(v);
					}
				}

				// Reverse edges using InEdges() - O(in_degree) instead of O(E)!
				for (auto e: graph.InEdges(u))
				{
					auto v = graph.Source(e);
					if (not parent[v].visited and graph.EdgeProperty(e).flow > 0)
					{
						parent[v] = PathInfo {u, e, false, true};
						queue.push_back(v);
					}
				}
			}

			return false;
		}

		/**
		 * @brief BFS helper that scans all edges for reverse traversal
		 *        when bidirectional storage is not available. O(V * E) total.
		 */
		template<typename GraphType, std::size_t MaxV, typename PathInfo, typename Queue>
		constexpr auto find_augmenting_path_fallback(
			GraphType& graph,
			typename GraphType::VertexDescriptor source,
			typename GraphType::VertexDescriptor sink,
			std::array<PathInfo, MaxV>& parent,
			Queue& queue) -> bool
		{
			using VertexDescriptor = typename GraphType::VertexDescriptor;

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
				for (auto e: graph.OutEdges(u))
				{
					auto v = graph.Target(e);
					if (not parent[v].visited and graph.EdgeProperty(e).Residual() > 0)
					{
						parent[v] = PathInfo {u, e, true, true};
						queue.push_back(v);
					}
				}

				// Fallback: scan ALL edges for reverse direction - O(E) per vertex
				for (auto e: graph.Edges())
				{
					if (graph.Target(e) == u)
					{
						auto v = graph.Source(e);
						if (not parent[v].visited and graph.EdgeProperty(e).flow > 0)
						{
							parent[v] = PathInfo {u, e, false, true};
							queue.push_back(v);
						}
					}
				}
			}

			return false;
		}
	}

	/**
	 * @brief Edmonds-Karp algorithm for maximum flow.
	 *        Uses concept-constrained dispatch to leverage bidirectional storage
	 *        when available for O(V*E²) complexity instead of O(V²*E²).
	 *
	 * @tparam VertexProperty The vertex property type
	 * @tparam EdgeProperty The edge property type (must have capacity and flow)
	 * @tparam Policy The storage policy (Fixed<MaxV, MaxE, Dir>)
	 * @param graph The flow network
	 * @param source Source vertex
	 * @param sink Sink vertex
	 * @return The maximum flow value
	 */
	template<typename VertexProperty, typename EdgeProperty, typename Policy>
		requires(not Policy::IS_DYNAMIC)  // Only works with fixed-size graphs
	constexpr auto MaxFlow(
		peri::graph::Graph<VertexProperty, EdgeProperty, Policy>& graph,
		typename peri::graph::Graph<VertexProperty, EdgeProperty, Policy>::VertexDescriptor source,
		typename peri::graph::Graph<VertexProperty, EdgeProperty, Policy>::VertexDescriptor sink)
		-> MaxFlowResult<decltype(std::declval<EdgeProperty>().capacity)>
	{
		using GraphType		   = peri::graph::Graph<VertexProperty, EdgeProperty, Policy>;
		using VertexDescriptor = typename GraphType::VertexDescriptor;
		using EdgeDescriptor   = typename GraphType::EdgeDescriptor;
		using FlowType		   = decltype(std::declval<EdgeProperty>().capacity);

		static constexpr std::size_t MaxV = Policy::MAX_VERTICES;

		MaxFlowResult<FlowType> result {};

		// Path reconstruction info
		struct PathInfo
		{
			VertexDescriptor parent {};
			EdgeDescriptor edge {};
			bool isForward = true;
			bool visited   = false;
		};

		std::array<PathInfo, MaxV> parent {};
		FixedRingBuffer<VertexDescriptor, MaxV> queue;

		// BFS to find augmenting path - dispatch based on storage capabilities
		auto findAugmentingPath = [&]() constexpr -> bool
		{
			if constexpr (peri::graph::bidirectional_storage<typename GraphType::StorageType>)
			{
				// O(V + E) per BFS iteration
				return detail::find_augmenting_path_bidirectional<GraphType, MaxV>(graph, source, sink, parent, queue);
			}
			else
			{
				// O(V * E) per BFS iteration (fallback)
				return detail::find_augmenting_path_fallback<GraphType, MaxV>(graph, source, sink, parent, queue);
			}
		};

		// Main loop: find augmenting paths and push flow
		while (findAugmentingPath())
		{
			result.foundPath = true;

			// Find bottleneck capacity along the path
			FlowType pathFlow = std::numeric_limits<FlowType>::max();

			for (auto v = sink; v != source;)
			{
				const auto& info = parent[v];
				if (info.isForward)
				{
					pathFlow = std::min(pathFlow, graph.EdgeProperty(info.edge).Residual());
				}
				else
				{
					pathFlow = std::min(pathFlow, graph.EdgeProperty(info.edge).flow);
				}
				v = info.parent;
			}

			// Augment flow along the path
			for (auto v = sink; v != source;)
			{
				const auto& info = parent[v];
				if (info.isForward)
				{
					graph.EdgeProperty(info.edge).flow += pathFlow;
				}
				else
				{
					graph.EdgeProperty(info.edge).flow -= pathFlow;
				}
				v = info.parent;
			}

			result.maxFlow += pathFlow;
		}

		return result;
	}
}
