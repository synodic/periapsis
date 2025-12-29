export module synodic.periapsis:structure.graph.storage.undirected_fixed;

import :structure.graph.concepts;

import std;

export namespace peri::graph::storage
{
	/**
	 * @brief A constexpr-capable undirected graph storage using fixed-size arrays.
	 *        Each edge is stored once and accessible from both endpoints.
	 *
	 * @tparam VertexProperty Type stored per vertex, void for none
	 * @tparam EdgeProperty Type stored per edge, void for none
	 * @tparam MaxVertices Maximum number of vertices
	 * @tparam MaxEdges Maximum number of edges
	 * @tparam MaxIncidentPerVertex Maximum incident edges per vertex for fixed arrays
	 * @tparam IndexType Unsigned integer type for indices
	 */
	template<
		typename VertexProperty,
		typename EdgeProperty,
		std::size_t MaxVertices,
		std::size_t MaxEdges,
		std::size_t MaxIncidentPerVertex = MaxEdges,
		std::unsigned_integral IndexType = std::size_t>
	class UndirectedFixed
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
		static constexpr bool HAS_STABLE_VERTEX_DESCRIPTORS = true;
		static constexpr bool HAS_STABLE_EDGE_DESCRIPTORS   = true;
		static constexpr bool IS_UNDIRECTED                 = true;

		static constexpr VerticesSizeType max_vertices          = static_cast<VerticesSizeType>(MaxVertices);
		static constexpr EdgesSizeType max_edges                = static_cast<EdgesSizeType>(MaxEdges);
		static constexpr DegreeSizeType max_incident_per_vertex = static_cast<DegreeSizeType>(MaxIncidentPerVertex);

		// Rebind to different property types
		template<typename VP, typename EP>
		using Rebind = UndirectedFixed<VP, EP, MaxVertices, MaxEdges, MaxIncidentPerVertex, IndexType>;

	private:
		// Edge storage: both endpoints + optional property
		struct EdgeData
		{
			VertexDescriptor endpoint1{};
			VertexDescriptor endpoint2{};
		};

		// Per-vertex incident edge list (fixed size)
		struct VertexIncidence
		{
			std::array<EdgeDescriptor, MaxIncidentPerVertex> edges{};
			DegreeSizeType count{0};
		};

		// Property storage helpers
		struct EmptyVertexProps
		{
		};

		struct EmptyEdgeProps
		{
		};

		using VertexPropsStorage = std::conditional_t<
			std::same_as<VertexProperty, void>,
			EmptyVertexProps,
			std::array<VertexProperty, MaxVertices>>;

		using EdgePropsStorage =
		std::conditional_t<std::same_as<EdgeProperty, void>, EmptyEdgeProps, std::array<EdgeProperty, MaxEdges>>;

		// Storage
		std::array<EdgeData, MaxEdges> edges_{};
		std::array<VertexIncidence, MaxVertices> incidence_{};

		[[no_unique_address]] VertexPropsStorage vertexProperties_{};
		[[no_unique_address]] EdgePropsStorage edgeProperties_{};

		VerticesSizeType numVertices_{0};
		EdgesSizeType numEdges_{0};

		// Boundary vertices for subgraph composition
		std::array<VertexDescriptor, MaxVertices> boundary_{};
		VerticesSizeType numBoundary_{0};

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		constexpr UndirectedFixed() = default;

		// ========================================================================
		// Vertex Operations
		// ========================================================================

		[[nodiscard]] constexpr auto NumVertices() const noexcept -> VerticesSizeType
		{
			return numVertices_;
		}

		[[nodiscard]] constexpr auto Vertices() const noexcept
		{
			return std::views::iota(VertexDescriptor{0}, numVertices_);
		}

		constexpr auto AddVertex() -> VertexDescriptor
		{
			if (numVertices_ >= max_vertices)
			{
				// Compile-time error in constexpr context
			}
			return numVertices_++;
		}

		template<typename... Args>
			requires(not std::same_as<VertexProperty, void>) and std::constructible_from<VertexProperty, Args...>
		constexpr auto AddVertex(Args&&... args) -> VertexDescriptor
		{
			auto v               = AddVertex();
			vertexProperties_[v] = VertexProperty(std::forward<Args>(args)...);
			return v;
		}

		// ========================================================================
		// Edge Operations
		// ========================================================================

		[[nodiscard]] constexpr auto NumEdges() const noexcept -> EdgesSizeType
		{
			return numEdges_;
		}

		[[nodiscard]] constexpr auto Edges() const noexcept
		{
			return std::views::iota(EdgeDescriptor{0}, numEdges_);
		}

		constexpr auto AddEdge(VertexDescriptor u, VertexDescriptor v) -> std::pair<EdgeDescriptor, bool>
		{
			if (numEdges_ >= max_edges)
			{
				return {EdgeDescriptor{}, false};
			}

			auto edgeIdx    = numEdges_++;
			edges_[edgeIdx] = EdgeData{u, v};

			// Add to both vertices' incident lists
			auto& incU               = incidence_[u];
			incU.edges[incU.count++] = edgeIdx;

			if (u != v) // Don't add twice for self-loops
			{
				auto& incV               = incidence_[v];
				incV.edges[incV.count++] = edgeIdx;
			}

			return {edgeIdx, true};
		}

		template<typename... Args>
			requires(not std::same_as<EdgeProperty, void>) and std::constructible_from<EdgeProperty, Args...>
		constexpr auto AddEdge(
			VertexDescriptor u,
			VertexDescriptor v,
			Args&&... args)
			-> std::pair<EdgeDescriptor, bool>
		{
			auto [edge, inserted] = AddEdge(u, v);
			if (inserted)
			{
				edgeProperties_[edge] = EdgeProperty(std::forward<Args>(args)...);
			}
			return {edge, inserted};
		}

		// ========================================================================
		// Undirected Edge Access
		// ========================================================================

		// Total degree (number of incident edges)
		[[nodiscard]] constexpr auto Degree(VertexDescriptor v) const noexcept -> DegreeSizeType
		{
			return incidence_[v].count;
		}

		// All edges incident to vertex v
		[[nodiscard]] constexpr auto IncidentEdges(VertexDescriptor v) const noexcept
		{
			return std::span{incidence_[v].edges.data(), incidence_[v].count};
		}

		// Get the other endpoint of edge e from vertex v
		[[nodiscard]] constexpr auto Opposite(EdgeDescriptor e, VertexDescriptor v) const noexcept -> VertexDescriptor
		{
			const auto& edge = edges_[e];
			return edge.endpoint1 == v ? edge.endpoint2 : edge.endpoint1;
		}

		// Get first endpoint of edge
		[[nodiscard]] constexpr auto Endpoint1(EdgeDescriptor e) const noexcept -> VertexDescriptor
		{
			return edges_[e].endpoint1;
		}

		// Get second endpoint of edge
		[[nodiscard]] constexpr auto Endpoint2(EdgeDescriptor e) const noexcept -> VertexDescriptor
		{
			return edges_[e].endpoint2;
		}

		// ========================================================================
		// Directed-style access (for compatibility)
		// ========================================================================

		[[nodiscard]] constexpr auto Source(EdgeDescriptor e) const noexcept -> VertexDescriptor
		{
			return edges_[e].endpoint1;
		}

		[[nodiscard]] constexpr auto Target(EdgeDescriptor e) const noexcept -> VertexDescriptor
		{
			return edges_[e].endpoint2;
		}

		[[nodiscard]] constexpr auto OutEdges(VertexDescriptor v) const noexcept
		{
			return IncidentEdges(v);
		}

		[[nodiscard]] constexpr auto OutDegree(VertexDescriptor v) const noexcept -> DegreeSizeType
		{
			return Degree(v);
		}

		// ========================================================================
		// Adjacency (Direct Neighbor Access)
		// ========================================================================

		[[nodiscard]] constexpr auto AdjacentVertices(VertexDescriptor v) const noexcept
		{
			return std::span{incidence_[v].edges.data(), incidence_[v].count} | std::views::transform(
				       [this, v](EdgeDescriptor e)
				       {
					       return Opposite(e, v);
				       });
		}

		// ========================================================================
		// Property Access
		// ========================================================================

		[[nodiscard]] constexpr decltype(auto) operator[](VertexDescriptor v)
			requires(not std::same_as<VertexProperty, void>)
		{
			return vertexProperties_[v];
		}

		[[nodiscard]] constexpr decltype(auto) operator[](VertexDescriptor v) const
			requires(not std::same_as<VertexProperty, void>)
		{
			return vertexProperties_[v];
		}

		[[nodiscard]] constexpr decltype(auto) operator[](EdgeDescriptor e)
			requires(not std::same_as<EdgeProperty, void>)
		{
			return edgeProperties_[e];
		}

		[[nodiscard]] constexpr decltype(auto) operator[](EdgeDescriptor e) const
			requires(not std::same_as<EdgeProperty, void>)
		{
			return edgeProperties_[e];
		}

		// Named property accessors (avoid ambiguity when VertexDescriptor == EdgeDescriptor)
		[[nodiscard]] constexpr decltype(auto) vertex_property(VertexDescriptor v)
			requires(not std::same_as<VertexProperty, void>)
		{
			return vertexProperties_[v];
		}

		[[nodiscard]] constexpr decltype(auto) vertex_property(VertexDescriptor v) const
			requires(not std::same_as<VertexProperty, void>)
		{
			return vertexProperties_[v];
		}

		[[nodiscard]] constexpr decltype(auto) edge_property(EdgeDescriptor e)
			requires(not std::same_as<EdgeProperty, void>)
		{
			return edgeProperties_[e];
		}

		[[nodiscard]] constexpr decltype(auto) edge_property(EdgeDescriptor e) const
			requires(not std::same_as<EdgeProperty, void>)
		{
			return edgeProperties_[e];
		}

		// ========================================================================
		// Subgraph Composition Support
		// ========================================================================

		constexpr void MarkBoundary(VertexDescriptor v)
		{
			boundary_[numBoundary_++] = v;
		}

		[[nodiscard]] constexpr auto BoundaryVertices() const noexcept
		{
			return std::span{boundary_.data(), numBoundary_};
		}

		[[nodiscard]] constexpr auto NumBoundaryVertices() const noexcept -> VerticesSizeType
		{
			return numBoundary_;
		}
	};

	// ============================================================================
	// Convenience Aliases
	// ============================================================================

	// Undirected fixed topology (no properties)
	template<std::size_t MaxVertices, std::size_t MaxEdges, std::unsigned_integral IndexType = std::size_t>
	using UndirectedFixedTopology = UndirectedFixed<void, void, MaxVertices, MaxEdges, MaxEdges, IndexType>;

	// Undirected fixed with vertex properties only
	template<
		typename VertexProperty,
		std::size_t MaxVertices,
		std::size_t MaxEdges,
		std::unsigned_integral IndexType = std::size_t>
	using UndirectedFixedVertexStorage =
	UndirectedFixed<VertexProperty, void, MaxVertices, MaxEdges, MaxEdges, IndexType>;

	// Undirected fixed with edge properties only
	template<
		typename EdgeProperty,
		std::size_t MaxVertices,
		std::size_t MaxEdges,
		std::unsigned_integral IndexType = std::size_t>
	using UndirectedFixedEdgeStorage = UndirectedFixed<void, EdgeProperty, MaxVertices, MaxEdges, MaxEdges, IndexType>;
}
