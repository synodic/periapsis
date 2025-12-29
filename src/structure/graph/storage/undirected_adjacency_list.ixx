export module synodic.periapsis:structure.graph.storage.undirected_adjacency_list;

import :structure.graph.concepts;

import std;

export namespace peri::graph::storage
{
	/**
	 * @brief Dynamic undirected graph storage using adjacency lists.
	 *        Each edge is stored once and accessible from both endpoints.
	 *        Memory efficient compared to bidirectional with no duplicate edge storage.
	 *
	 * @tparam VertexProperty Type stored per vertex, void for none
	 * @tparam EdgeProperty Type stored per edge, void for none
	 * @tparam VertexContainer Container template for vertex storage
	 * @tparam EdgeContainer Container template for edge lists
	 * @tparam IndexType Unsigned integer type for descriptors
	 */
	template<
		typename VertexProperty,
		typename EdgeProperty,
		template<typename...> typename VertexContainer = std::vector,
		template<typename...> typename EdgeContainer = std::vector,
		std::unsigned_integral IndexType = std::size_t>
	class UndirectedAdjacencyList
	{
	public:
		// ========================================================================
		// Type Definitions
		// ========================================================================

		using VertexDescriptor   = IndexType;
		using EdgeDescriptor     = IndexType;
		using VerticesSizeType   = IndexType;
		using EdgesSizeType      = IndexType;
		using DegreeSizeType     = IndexType;
		using VertexPropertyType = VertexProperty;
		using EdgePropertyType   = EdgeProperty;

		// Compile-time properties
		static constexpr bool IS_DIRECTED                   = false;
		static constexpr bool ALLOWS_PARALLEL_EDGES         = true;
		static constexpr bool HAS_STABLE_VERTEX_DESCRIPTORS = false;
		static constexpr bool HAS_STABLE_EDGE_DESCRIPTORS   = false;
		static constexpr bool IS_UNDIRECTED                 = true;

		// Rebind to different property types
		template<typename VP, typename EP>
		using Rebind = UndirectedAdjacencyList<VP, EP, VertexContainer, EdgeContainer, IndexType>;

	private:
		// Edge stored once with both endpoints
		struct EdgeData
		{
			VertexDescriptor endpoint1;
			VertexDescriptor endpoint2;
			[[no_unique_address]] std::
			conditional_t<std::same_as<EdgePropertyType, void>, std::monostate, EdgePropertyType> property;
		};

		// Per-vertex data: property + list of incident edge indices
		struct VertexData
		{
			[[no_unique_address]] std::
			conditional_t<std::same_as<VertexPropertyType, void>, std::monostate, VertexPropertyType> property;
			EdgeContainer<EdgeDescriptor> incidentEdges;
		};

		VertexContainer<VertexData> vertices_;
		EdgeContainer<EdgeData> edges_;

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		UndirectedAdjacencyList() = default;

		explicit UndirectedAdjacencyList(VerticesSizeType initialVertexCapacity)
		{
			vertices_.reserve(initialVertexCapacity);
		}

		// ========================================================================
		// Vertex Operations
		// ========================================================================

		[[nodiscard]] auto NumVertices() const noexcept -> VerticesSizeType
		{
			return static_cast<VerticesSizeType>(vertices_.size());
		}

		[[nodiscard]] auto Vertices() const noexcept
		{
			return std::views::iota(VertexDescriptor{0}, NumVertices());
		}

		auto AddVertex() -> VertexDescriptor
		{
			auto v = static_cast<VertexDescriptor>(vertices_.size());
			vertices_.emplace_back();
			return v;
		}

		template<typename... Args>
			requires(not std::same_as<VertexPropertyType, void>) and
			        std::constructible_from<VertexPropertyType, Args...>
		auto AddVertex(Args&&... args) -> VertexDescriptor
		{
			auto v        = static_cast<VertexDescriptor>(vertices_.size());
			auto& data    = vertices_.emplace_back();
			data.property = VertexPropertyType(std::forward<Args>(args)...);
			return v;
		}

		void RemoveVertex(VertexDescriptor v)
		{
			// Remove all incident edges first
			// Work backwards to avoid invalidating indices
			auto& incident = vertices_[v].incidentEdges;
			while (!incident.empty())
			{
				RemoveEdge(incident.back());
			}

			// Swap with last and pop
			if (v != vertices_.size() - 1)
			{
				auto last = static_cast<VertexDescriptor>(vertices_.size() - 1);

				// Update all edges that reference 'last' to reference 'v'
				for (auto& edge: edges_)
				{
					if (edge.endpoint1 == last)
					{
						edge.endpoint1 = v;
					}
					if (edge.endpoint2 == last)
					{
						edge.endpoint2 = v;
					}
				}

				vertices_[v] = std::move(vertices_[last]);
			}
			vertices_.pop_back();
		}

		void ClearVertices()
		{
			vertices_.clear();
			edges_.clear();
		}

		// ========================================================================
		// Edge Operations
		// ========================================================================

		[[nodiscard]] auto NumEdges() const noexcept -> EdgesSizeType
		{
			return static_cast<EdgesSizeType>(edges_.size());
		}

		[[nodiscard]] auto Edges() const noexcept
		{
			return std::views::iota(EdgeDescriptor{0}, NumEdges());
		}

		auto AddEdge(VertexDescriptor u, VertexDescriptor v) -> std::pair<EdgeDescriptor, bool>
		{
			auto edgeIdx = static_cast<EdgeDescriptor>(edges_.size());
			edges_.push_back(EdgeData{u, v, {}});

			// Add to both endpoints' incident lists
			vertices_[u].incidentEdges.push_back(edgeIdx);
			if (u != v) // Don't add twice for self-loops
			{
				vertices_[v].incidentEdges.push_back(edgeIdx);
			}

			return {edgeIdx, true};
		}

		template<typename... Args>
			requires(not std::same_as<EdgePropertyType, void>) and std::constructible_from<EdgePropertyType, Args...>
		auto AddEdge(VertexDescriptor u, VertexDescriptor v, Args&&... args) -> std::pair<EdgeDescriptor, bool>
		{
			auto edgeIdx = static_cast<EdgeDescriptor>(edges_.size());
			edges_.push_back(EdgeData{u, v, EdgePropertyType(std::forward<Args>(args)...)});

			vertices_[u].incidentEdges.push_back(edgeIdx);
			if (u != v)
			{
				vertices_[v].incidentEdges.push_back(edgeIdx);
			}

			return {edgeIdx, true};
		}

		void RemoveEdge(VertexDescriptor u, VertexDescriptor v)
		{
			// Find edge connecting u and v
			for (auto edgeIdx: vertices_[u].incidentEdges)
			{
				auto& edge = edges_[edgeIdx];
				if ((edge.endpoint1 == u && edge.endpoint2 == v) || (edge.endpoint1 == v && edge.endpoint2 == u))
				{
					remove_edge(edgeIdx);
					return;
				}
			}
		}

		void RemoveEdge(EdgeDescriptor e)
		{
			auto& edge = edges_[e];
			auto u     = edge.endpoint1;
			auto v     = edge.endpoint2;

			// Remove from incident lists
			std::erase(vertices_[u].incidentEdges, e);
			if (u != v)
			{
				std::erase(vertices_[v].incidentEdges, e);
			}

			// Swap with last edge and pop
			if (e != edges_.size() - 1)
			{
				auto lastIdx   = static_cast<EdgeDescriptor>(edges_.size() - 1);
				auto& lastEdge = edges_[lastIdx];

				// Update incident lists to reference new index
				for (auto& idx: vertices_[lastEdge.endpoint1].incidentEdges)
				{
					if (idx == lastIdx)
					{
						idx = e;
					}
				}
				if (lastEdge.endpoint1 != lastEdge.endpoint2)
				{
					for (auto& idx: vertices_[lastEdge.endpoint2].incidentEdges)
					{
						if (idx == lastIdx)
						{
							idx = e;
						}
					}
				}

				edges_[e] = std::move(edges_[lastIdx]);
			}
			edges_.pop_back();
		}

		void ClearEdges()
		{
			for (auto& vertex: vertices_)
			{
				vertex.incidentEdges.clear();
			}
			edges_.clear();
		}

		// ========================================================================
		// Undirected Edge Access
		// ========================================================================

		// Total degree (number of incident edges, self-loops count once)
		[[nodiscard]] auto Degree(VertexDescriptor v) const noexcept -> DegreeSizeType
		{
			return static_cast<DegreeSizeType>(vertices_[v].incidentEdges.size());
		}

		// All edges incident to vertex v
		[[nodiscard]] auto IncidentEdges(VertexDescriptor v) const noexcept
		{
			return std::span{vertices_[v].incidentEdges};
		}

		// Get the other endpoint of edge e from vertex v
		[[nodiscard]] auto Opposite(EdgeDescriptor e, VertexDescriptor v) const noexcept -> VertexDescriptor
		{
			const auto& edge = edges_[e];
			return edge.endpoint1 == v ? edge.endpoint2 : edge.endpoint1;
		}

		// Get first endpoint of edge
		[[nodiscard]] auto Endpoint1(EdgeDescriptor e) const noexcept -> VertexDescriptor
		{
			return edges_[e].endpoint1;
		}

		// Get second endpoint of edge
		[[nodiscard]] auto Endpoint2(EdgeDescriptor e) const noexcept -> VertexDescriptor
		{
			return edges_[e].endpoint2;
		}

		// ========================================================================
		// Directed-style access (for compatibility)
		// For undirected, source/target are arbitrary but consistent
		// ========================================================================

		[[nodiscard]] auto Source(EdgeDescriptor e) const noexcept -> VertexDescriptor
		{
			return edges_[e].endpoint1;
		}

		[[nodiscard]] auto Target(EdgeDescriptor e) const noexcept -> VertexDescriptor
		{
			return edges_[e].endpoint2;
		}

		// For undirected, out_edges returns incident_edges
		[[nodiscard]] auto OutEdges(VertexDescriptor v) const noexcept
		{
			return IncidentEdges(v);
		}

		[[nodiscard]] auto OutDegree(VertexDescriptor v) const noexcept -> DegreeSizeType
		{
			return Degree(v);
		}

		// ========================================================================
		// Adjacency (Direct Neighbor Access)
		// ========================================================================

		[[nodiscard]] auto AdjacentVertices(VertexDescriptor v) const noexcept
		{
			return vertices_[v].incidentEdges | std::views::transform(
				       [this, v](EdgeDescriptor e)
				       {
					       return Opposite(e, v);
				       });
		}

		// ========================================================================
		// Property Access
		// ========================================================================

		template<typename Self>
		[[nodiscard]] auto&& VertexProperty(this Self&& self, VertexDescriptor v)
			requires(not std::same_as<VertexPropertyType, void>)
		{
			return std::forward<Self>(self).vertices_[v].property;
		}

		template<typename Self>
		[[nodiscard]] auto&& EdgeProperty(this Self&& self, EdgeDescriptor e)
			requires(not std::same_as<EdgePropertyType, void>)
		{
			return std::forward<Self>(self).edges_[e].property;
		}

		[[nodiscard]] decltype(auto) operator[](VertexDescriptor v)
			requires(not std::same_as<VertexPropertyType, void>)
		{
			return VertexProperty(v);
		}

		[[nodiscard]] decltype(auto) operator[](VertexDescriptor v) const
			requires(not std::same_as<VertexPropertyType, void>)
		{
			return VertexProperty(v);
		}

		[[nodiscard]] decltype(auto) operator[](EdgeDescriptor e)
			requires(not std::same_as<EdgePropertyType, void>)
		{
			return EdgeProperty(e);
		}

		[[nodiscard]] decltype(auto) operator[](EdgeDescriptor e) const
			requires(not std::same_as<EdgePropertyType, void>)
		{
			return EdgeProperty(e);
		}
	};

	// ============================================================================
	// Convenience Aliases
	// ============================================================================

	// Undirected topology (no properties)
	template<std::unsigned_integral IndexType = std::size_t>
	using UndirectedTopology = UndirectedAdjacencyList<void, void, std::vector, std::vector, IndexType>;

	// Undirected with vertex properties only
	template<typename VertexProperty, std::unsigned_integral IndexType = std::size_t>
	using UndirectedVertexStorage = UndirectedAdjacencyList<VertexProperty, void, std::vector, std::vector, IndexType>;

	// Undirected with edge properties only (weighted graphs)
	template<typename EdgeProperty, std::unsigned_integral IndexType = std::size_t>
	using UndirectedEdgeStorage = UndirectedAdjacencyList<void, EdgeProperty, std::vector, std::vector, IndexType>;
}
