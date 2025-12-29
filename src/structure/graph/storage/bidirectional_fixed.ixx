export module synodic.periapsis:structure.graph.storage.bidirectional_fixed;

import :structure.graph.concepts;

import std;

export namespace peri::graph::storage
{
	/**
	 * @brief A constexpr-capable bidirectional graph storage using fixed-size arrays.
	 *        Extends StaticStorage with in-edge tracking for O(1) predecessor access.
	 *        Uses CSR-like layout for out-edges and CSC-like layout for in-edges.
	 *
	 * @tparam VertexProperty Type stored per vertex, use void for none
	 * @tparam EdgeProperty Type stored per edge, use void for none)
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
	class BidirectionalFixed
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
		static constexpr bool HAS_STABLE_VERTEX_DESCRIPTORS = true;
		static constexpr bool HAS_STABLE_EDGE_DESCRIPTORS   = true;
		static constexpr bool IS_BIDIRECTIONAL              = true;

		static constexpr VerticesSizeType max_vertices = static_cast<VerticesSizeType>(MaxVertices);
		static constexpr EdgesSizeType max_edges       = static_cast<EdgesSizeType>(MaxEdges);

		// Rebind to different property types
		template<typename VP, typename EP>
		using Rebind = BidirectionalFixed<VP, EP, MaxVertices, MaxEdges, IndexType>;

	private:
		// CSR-style storage for out-edges
		//   outRowOffsets_[v] gives the start index in outTargets_ for vertex v's out-edges
		//   outRowOffsets_[v+1] - outRowOffsets_[v] gives out-degree
		std::array<EdgesSizeType, MaxVertices + 1> outRowOffsets_{};
		std::array<VertexDescriptor, MaxEdges> outTargets_{};

		// CSC-style storage for in-edges (reverse/transpose structure)
		//   inRowOffsets_[v] gives the start index in inSources_ for vertex v's in-edges
		//   inRowOffsets_[v+1] - inRowOffsets_[v] gives in-degree
		//   inEdgeIndices_ stores the original edge index for property access
		std::array<EdgesSizeType, MaxVertices + 1> inRowOffsets_{};
		std::array<VertexDescriptor, MaxEdges> inSources_{};
		std::array<EdgeDescriptor, MaxEdges> inEdgeIndices_{};

		// Property storage (conditionally empty via [[no_unique_address]])
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

		// Boundary vertices for subgraph composition
		std::array<VertexDescriptor, MaxVertices> boundary_{};
		VerticesSizeType numBoundary_{0};

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		constexpr BidirectionalFixed() = default;

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
			}
			auto v                = numVertices_++;
			outRowOffsets_[v + 1] = outRowOffsets_[v];
			inRowOffsets_[v + 1]  = inRowOffsets_[v];
			return v;
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
		// Edge Operations (Build Phase)
		// ========================================================================

		constexpr auto AddEdge(VertexDescriptor source, VertexDescriptor target) -> std::pair<EdgeDescriptor, bool>
		{
			if (numEdges_ >= max_edges)
			{
				return {EdgeDescriptor{}, false};
			}

			auto edgeIdx         = numEdges_++;
			outTargets_[edgeIdx] = target;

			// Update out-edge row offsets for vertices after source
			for (auto v = source + 1; v <= numVertices_; ++v)
			{
				++outRowOffsets_[v];
			}

			// Update in-edge structure
			inSources_[edgeIdx]     = source;
			inEdgeIndices_[edgeIdx] = edgeIdx;

			// Update in-edge row offsets for vertices after target
			for (auto v = target + 1; v <= numVertices_; ++v)
			{
				++inRowOffsets_[v];
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
				edgeProperties_[edge] = EdgeProperty(std::forward<Args>(args)...);
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
			return outRowOffsets_[v + 1] - outRowOffsets_[v];
		}

		[[nodiscard]] constexpr auto OutEdges(VertexDescriptor v) const noexcept
		{
			return std::views::iota(outRowOffsets_[v], outRowOffsets_[v + 1]);
		}

		[[nodiscard]] constexpr auto Source(EdgeDescriptor e) const noexcept -> VertexDescriptor
		{
			// Binary search to find which vertex owns this edge
			auto it = std::ranges::upper_bound(outRowOffsets_ | std::views::take(numVertices_ + 1), e);
			return static_cast<VertexDescriptor>(std::distance(outRowOffsets_.begin(), it) - 1);
		}

		[[nodiscard]] constexpr auto Target(EdgeDescriptor e) const noexcept -> VertexDescriptor
		{
			return outTargets_[e];
		}

		// ========================================================================
		// Bidirectional (In-Edge Traversal)
		// ========================================================================

		[[nodiscard]] constexpr auto InDegree(VertexDescriptor v) const noexcept -> DegreeSizeType
		{
			return inRowOffsets_[v + 1] - inRowOffsets_[v];
		}

		/**
		 * @brief Returns range of edge descriptors for in-edges of vertex v.
		 *        These are the original edge descriptors, so edge properties can be accessed.
		 */
		[[nodiscard]] constexpr auto InEdges(VertexDescriptor v) const noexcept
		{
			return std::views::iota(inRowOffsets_[v], inRowOffsets_[v + 1]) | std::views::transform(
				       [this](EdgesSizeType i)
				       {
					       return inEdgeIndices_[i];
				       });
		}

		/**
		 * @brief Returns the source vertex for an in-edge at the given position in the in-edge list.
		 *        Useful when iterating in_edges() and needing the predecessor vertex.
		 */
		[[nodiscard]] constexpr auto InEdgeSource(
			VertexDescriptor v,
			DegreeSizeType inEdgeIndex) const noexcept
			-> VertexDescriptor
		{
			return inSources_[inRowOffsets_[v] + inEdgeIndex];
		}

		// ========================================================================
		// Adjacency (Direct Neighbor Access)
		// ========================================================================

		[[nodiscard]] constexpr auto AdjacentVertices(VertexDescriptor v) const noexcept
		{
			return std::views::iota(outRowOffsets_[v], outRowOffsets_[v + 1]) | std::views::transform(
				       [this](EdgeDescriptor e)
				       {
					       return outTargets_[e];
				       });
		}

		/**
		 * @brief Returns range of predecessor vertices (vertices with edges pointing to v).
		 */
		[[nodiscard]] constexpr auto PredecessorVertices(VertexDescriptor v) const noexcept
		{
			return std::views::iota(inRowOffsets_[v], inRowOffsets_[v + 1]) | std::views::transform(
				       [this](EdgesSizeType i)
				       {
					       return inSources_[i];
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

	// Bidirectional fixed storage without properties (topology only)
	template<std::size_t MaxVertices, std::size_t MaxEdges, std::unsigned_integral IndexType = std::size_t>
	using BidirectionalFixedTopology = BidirectionalFixed<void, void, MaxVertices, MaxEdges, IndexType>;

	// Bidirectional fixed storage with vertex properties only
	template<
		typename VertexProperty,
		std::size_t MaxVertices,
		std::size_t MaxEdges,
		std::unsigned_integral IndexType = std::size_t>
	using BidirectionalFixedVertexStorage = BidirectionalFixed<VertexProperty, void, MaxVertices, MaxEdges, IndexType>;

	// Bidirectional fixed storage with edge properties only
	template<
		typename EdgeProperty,
		std::size_t MaxVertices,
		std::size_t MaxEdges,
		std::unsigned_integral IndexType = std::size_t>
	using BidirectionalFixedEdgeStorage = BidirectionalFixed<void, EdgeProperty, MaxVertices, MaxEdges, IndexType>;
}
