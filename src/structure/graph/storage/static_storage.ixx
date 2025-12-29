export module synodic.periapsis:structure.graph.storage.static_storage;

import :structure.graph.concepts;

import std;

export namespace peri::graph::storage
{
	/**
	 * @brief A constexpr-capable graph storage using fixed-size arrays.
	 *        Ideal for compile-time subgraph construction where topology is known at compile-time.
	 *        Uses CSR-like layout for cache-friendly traversal with O(1) vertex/edge access.
	 *
	 * @tparam VertexProperty Type stored per vertex, use NoProperty for none
	 * @tparam EdgeProperty Type stored per edge, use no_property for none)
	 * @tparam MaxVertices Maximum number of vertices
	 * @tparam MaxEdges Maximum number of edges
	 * @tparam IndexType Unsigned integer type for indices (default: std::size_t)
	 */
	template<
		typename VertexProperty,
		typename EdgeProperty,
		std::size_t MaxVertices,
		std::size_t MaxEdges,
		std::unsigned_integral IndexType = std::size_t>
	class StaticStorage
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
		static constexpr bool IS_DIRECTED                   = true;
		static constexpr bool ALLOWS_PARALLEL_EDGES         = true;
		static constexpr bool HAS_STABLE_VERTEX_DESCRIPTORS = true; // Indices don't move
		static constexpr bool HAS_STABLE_EDGE_DESCRIPTORS   = true;

		static constexpr VerticesSizeType max_vertices = static_cast<VerticesSizeType>(MaxVertices);
		static constexpr EdgesSizeType max_edges       = static_cast<EdgesSizeType>(MaxEdges);

		// Rebind to different property types
		template<typename VP, typename EP>
		using Rebind = StaticStorage<VP, EP, MaxVertices, MaxEdges, IndexType>;

	private:
		// CSR-style storage for O(1) vertex lookup and cache-friendly edge iteration
		//	rowOffsets_[v] gives the start index in targets_ for vertex v's out-edges
		//	rowOffsets_[v+1] - rowOffsets_[v] gives out-degree
		std::array<EdgesSizeType, MaxVertices + 1> rowOffsets_{};
		std::array<VertexDescriptor, MaxEdges> targets_{};

		// Property storage (conditionally empty for NoProperty via [[no_unique_address]])
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

		[[no_unique_address]] VertexPropsStorage vertexProperties_{};
		[[no_unique_address]] EdgePropsStorage edgeProperties_{};

		VerticesSizeType numVertices_{0};
		EdgesSizeType numEdges_{0};

		// Boundary vertices for subgraph composition (indices into vertex array)
		std::array<VertexDescriptor, MaxVertices> boundary_{};
		VerticesSizeType numBoundary_{0};

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		constexpr StaticStorage() = default;

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
				// In constexpr context, this will cause a compile error
				// At runtime, this is undefined behavior (could add assert)
			}
			auto v             = numVertices_++;
			rowOffsets_[v + 1] = rowOffsets_[v]; // Initialize with no edges
			return v;
		}

		template<typename... Args>
			requires(not std::same_as<VertexProperty, void>) and std::constructible_from<VertexProperty, Args...>
		constexpr auto AddVertex(Args&&... args) -> VertexDescriptor
		{
			auto v               = AddVertex();
			vertexProperties_[v] = VertexPropertyType(std::forward<Args>(args)...);
			return v;
		}

		// ========================================================================
		// Edge Operations (Build Phase)
		// ========================================================================

		// Note: Edges must be added in source-vertex order for CSR format.
		// Use the builder pattern or sort edges before finalizing.
		constexpr auto AddEdge(VertexDescriptor source, VertexDescriptor target) -> std::pair<EdgeDescriptor, bool>
		{
			if (numEdges_ >= max_edges)
			{
				return {EdgeDescriptor{}, false};
			}

			auto edgeIdx      = numEdges_++;
			targets_[edgeIdx] = target;

			// Update row offsets for all vertices after source
			for (auto v = source + 1; v <= numVertices_; ++v)
			{
				++rowOffsets_[v];
			}

			return {edgeIdx, true};
		}

		template<typename... Args>
			requires(not std::same_as<EdgeProperty, void>) and std::constructible_from<EdgeProperty, Args...>
		constexpr auto AddEdge(
			VertexDescriptor source,
			VertexDescriptor target,
			Args&&... args)
			-> std::pair<EdgeDescriptor, bool>
		{
			auto [edge, inserted] = AddEdge(source, target);
			if (inserted)
			{
				edgeProperties_[edge] = EdgePropertyType(std::forward<Args>(args)...);
			}
			return {edge, inserted};
		}

		[[nodiscard]] constexpr auto NumEdges() const noexcept -> EdgesSizeType
		{
			return numEdges_;
		}

		[[nodiscard]] constexpr auto Edges() const noexcept
		{
			return std::views::iota(EdgeDescriptor{0}, numEdges_);
		}

		// ========================================================================
		// Incidence (Out-Edge Traversal)
		// ========================================================================

		[[nodiscard]] constexpr auto OutDegree(VertexDescriptor v) const noexcept -> DegreeSizeType
		{
			return rowOffsets_[v + 1] - rowOffsets_[v];
		}

		// Returns range of edge descriptors for out-edges of vertex v
		[[nodiscard]] constexpr auto OutEdges(VertexDescriptor v) const noexcept
		{
			return std::views::iota(rowOffsets_[v], rowOffsets_[v + 1]);
		}

		[[nodiscard]] constexpr auto Source(EdgeDescriptor e) const noexcept -> VertexDescriptor
		{
			// Binary search to find which vertex owns this edge
			// O(log V) but typically not called in hot paths
			auto it = std::ranges::upper_bound(rowOffsets_ | std::views::take(numVertices_ + 1), e);
			return static_cast<VertexDescriptor>(std::distance(rowOffsets_.begin(), it) - 1);
		}

		[[nodiscard]] constexpr auto Target(EdgeDescriptor e) const noexcept -> VertexDescriptor
		{
			return targets_[e];
		}

		// ========================================================================
		// Adjacency (Direct Neighbor Access)
		// ========================================================================

		[[nodiscard]] constexpr auto AdjacentVertices(VertexDescriptor v) const noexcept
		{
			return std::views::iota(rowOffsets_[v], rowOffsets_[v + 1]) | std::views::transform(
				       [this](EdgeDescriptor e)
				       {
					       return targets_[e];
				       });
		}

		// ========================================================================
		// Property Access (O(1) via descriptor index)
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

		// PascalCase named property accessors
		[[nodiscard]] constexpr decltype(auto) VertexProperty(VertexDescriptor v)
			requires(not std::same_as<VertexProperty, void>)
		{
			return vertexProperties_[v];
		}

		[[nodiscard]] constexpr decltype(auto) VertexProperty(VertexDescriptor v) const
			requires(not std::same_as<VertexProperty, void>)
		{
			return vertexProperties_[v];
		}

		[[nodiscard]] constexpr decltype(auto) EdgeProperty(EdgeDescriptor e)
			requires(not std::same_as<EdgeProperty, void>)
		{
			return edgeProperties_[e];
		}

		[[nodiscard]] constexpr decltype(auto) EdgeProperty(EdgeDescriptor e) const
			requires(not std::same_as<EdgeProperty, void>)
		{
			return edgeProperties_[e];
		}

		// ========================================================================
		// Subgraph Composition Support
		// ========================================================================

		// Mark a vertex as a boundary vertex (entry/exit point for composition)
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

	// Static storage without properties (topology only)
	template<std::size_t MaxVertices, std::size_t MaxEdges, std::unsigned_integral IndexType = std::size_t>
	using StaticTopology = StaticStorage<void, void, MaxVertices, MaxEdges, IndexType>;

	// Static storage with vertex properties only
	template<
		typename VertexProperty,
		std::size_t MaxVertices,
		std::size_t MaxEdges,
		std::unsigned_integral IndexType = std::size_t>
	using StaticVertexStorage = StaticStorage<VertexProperty, void, MaxVertices, MaxEdges, IndexType>;

	// Static storage with edge properties only (common for weighted graphs)
	template<
		typename EdgeProperty,
		std::size_t MaxVertices,
		std::size_t MaxEdges,
		std::unsigned_integral IndexType = std::size_t>
	using StaticEdgeStorage = StaticStorage<void, EdgeProperty, MaxVertices, MaxEdges, IndexType>;
}
