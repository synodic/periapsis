export module synodic.periapsis:structure.graph.storage.csr;

import :structure.graph.concepts;

import std;

export namespace peri::graph::storage
{
	/**
	 * @brief Compressed Sparse Row (CSR) graph storage for immutable, cache-friendly graphs.
	 *        Ideal for graphs where topology is fixed at compile-time but properties may be updated at runtime.
	 *
	 * @tparam VertexProperty Type stored per vertex
	 * @tparam EdgeProperty Type stored per edge
	 * @tparam VertexContainer Container for vertex data (default: std::vector)
	 * @tparam EdgeContainer Container for edge data (default: std::vector)
	 * @tparam IndexType Unsigned integer type for indices
	 */
	template<
		typename VertexProperty,
		typename EdgeProperty,
		template<typename...> typename VertexContainer = std::vector,
		template<typename...> typename EdgeContainer = std::vector,
		std::unsigned_integral IndexType = std::size_t>
	class CSRStorage
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

		// Rebind to different property types
		template<typename VP, typename EP>
		using Rebind = CSRStorage<VP, EP, VertexContainer, EdgeContainer, IndexType>;

	private:
		// CSR structure arrays
		VertexContainer<EdgesSizeType> rowOffsets_;
		EdgeContainer<VertexDescriptor> colIndices_;

		// Property storage (conditionally present)
		struct Empty
		{
		};

		using VertexPropsStorage =
		std::conditional_t<std::same_as<VertexProperty, void>, Empty, VertexContainer<VertexProperty>>;

		using EdgePropsStorage =
		std::conditional_t<std::same_as<EdgeProperty, void>, Empty, EdgeContainer<EdgeProperty>>;

		[[no_unique_address]] VertexPropsStorage vertexProperties_;
		[[no_unique_address]] EdgePropsStorage edgeProperties_;

	public:
		// ========================================================================
		// Construction from Edge List
		// ========================================================================

		CSRStorage() :
			rowOffsets_(1, 0)
		{
		}

		/**
		 * @brief Construct CSR from sorted edge list.
		 *        Edges must be sorted by source vertex.
		 * @param numVertices Number of vertices in the graph
		 * @param edges Range of (source, target) pairs, sorted by source
		 */
		template<std::ranges::input_range EdgeRange>
			requires std::same_as<std::ranges::range_value_t<EdgeRange>, std::pair<VertexDescriptor, VertexDescriptor>>
		CSRStorage(VerticesSizeType numVertices, EdgeRange&& edges) :
			rowOffsets_(numVertices + 1, 0)
		{
			// Count edges per vertex
			for (const auto& [src, _]: edges)
			{
				++rowOffsets_[src + 1];
			}

			// Prefix sum to get offsets
			for (VerticesSizeType v = 1; v <= numVertices; ++v)
			{
				rowOffsets_[v] += rowOffsets_[v - 1];
			}

			// Allocate edge storage
			colIndices_.resize(rowOffsets_[numVertices]);

			// Fill column indices (using temporary offset copy)
			auto currentOffset = rowOffsets_;
			for (const auto& [src, tgt]: edges)
			{
				colIndices_[currentOffset[src]++] = tgt;
			}

			// Allocate property storage
			if constexpr (not std::same_as<VertexProperty, void>)
			{
				vertexProperties_.resize(numVertices);
			}
			if constexpr (not std::same_as<EdgeProperty, void>)
			{
				edgeProperties_.resize(colIndices_.size());
			}
		}

		/**
		 * @brief Construct CSR with properties from sorted edge list.
		 * @param numVertices Number of vertices
		 * @param vertexProps Vertex properties (indexed by vertex descriptor)
		 * @param edges Range of (source, target, edge_property) tuples, sorted by source
		 */
		template<std::ranges::input_range VertexPropRange, std::ranges::input_range EdgeRange>
			requires(not std::same_as<VertexProperty, void>) and (not std::same_as<EdgeProperty, void>) and
			        std::convertible_to<std::ranges::range_value_t<VertexPropRange>, VertexProperty>
		CSRStorage(VerticesSizeType numVertices, VertexPropRange&& vertexProps, EdgeRange&& edges) :
			rowOffsets_(numVertices + 1, 0)
		{
			// Copy vertex properties
			vertexProperties_.reserve(numVertices);
			for (auto&& prop: vertexProps)
			{
				vertexProperties_.push_back(std::forward<decltype(prop)>(prop));
			}

			// Count edges per vertex
			for (const auto& edge: edges)
			{
				++rowOffsets_[std::get<0>(edge) + 1];
			}

			// Prefix sum
			for (VerticesSizeType v = 1; v <= numVertices; ++v)
			{
				rowOffsets_[v] += rowOffsets_[v - 1];
			}

			// Allocate
			colIndices_.resize(rowOffsets_[numVertices]);
			edgeProperties_.resize(rowOffsets_[numVertices]);

			// Fill
			auto currentOffset = rowOffsets_;
			for (const auto& edge: edges)
			{
				auto idx             = currentOffset[std::get<0>(edge)]++;
				colIndices_[idx]     = std::get<1>(edge);
				edgeProperties_[idx] = std::get<2>(edge);
			}
		}

		// ========================================================================
		// Vertex Operations
		// ========================================================================

		[[nodiscard]] auto NumVertices() const noexcept -> VerticesSizeType
		{
			return static_cast<VerticesSizeType>(rowOffsets_.size() - 1);
		}

		[[nodiscard]] auto Vertices() const noexcept
		{
			return std::views::iota(VertexDescriptor{0}, NumVertices());
		}

		// ========================================================================
		// Edge Operations
		// ========================================================================

		[[nodiscard]] auto NumEdges() const noexcept -> EdgesSizeType
		{
			return static_cast<EdgesSizeType>(colIndices_.size());
		}

		[[nodiscard]] auto Edges() const noexcept
		{
			return std::views::iota(EdgeDescriptor{0}, NumEdges());
		}

		// ========================================================================
		// Incidence (Out-Edge Traversal) - O(1) access, cache-friendly iteration
		// ========================================================================

		[[nodiscard]] auto OutDegree(VertexDescriptor v) const noexcept -> DegreeSizeType
		{
			return static_cast<DegreeSizeType>(rowOffsets_[v + 1] - rowOffsets_[v]);
		}

		[[nodiscard]] auto OutEdges(VertexDescriptor v) const noexcept
		{
			return std::views::iota(
				static_cast<EdgeDescriptor>(rowOffsets_[v]),
				static_cast<EdgeDescriptor>(rowOffsets_[v + 1]));
		}

		[[nodiscard]] auto Source(EdgeDescriptor e) const noexcept -> VertexDescriptor
		{
			// Binary search in rowOffsets_ to find owning vertex
			auto it = std::ranges::upper_bound(rowOffsets_, e);
			return static_cast<VertexDescriptor>(std::distance(rowOffsets_.begin(), it) - 1);
		}

		[[nodiscard]] auto Target(EdgeDescriptor e) const noexcept -> VertexDescriptor
		{
			return colIndices_[e];
		}

		// ========================================================================
		// Adjacency: Direct Neighbor Access
		// ========================================================================

		[[nodiscard]] auto AdjacentVertices(VertexDescriptor v) const noexcept
		{
			return std::span{colIndices_.data() + rowOffsets_[v], colIndices_.data() + rowOffsets_[v + 1]};
		}

		// ========================================================================
		// Property Access - O(1) via descriptor index
		// ========================================================================

		[[nodiscard]] decltype(auto) operator[](VertexDescriptor v)
			requires(not std::same_as<VertexProperty, void>)
		{
			return vertexProperties_[v];
		}

		[[nodiscard]] decltype(auto) operator[](VertexDescriptor v) const
			requires(not std::same_as<VertexProperty, void>)
		{
			return vertexProperties_[v];
		}

		[[nodiscard]] decltype(auto) operator[](EdgeDescriptor e)
			requires(not std::same_as<EdgeProperty, void>)
		{
			return edgeProperties_[e];
		}

		[[nodiscard]] decltype(auto) operator[](EdgeDescriptor e) const
			requires(not std::same_as<EdgeProperty, void>)
		{
			return edgeProperties_[e];
		}

		// Named property accessors (avoid ambiguity when VertexDescriptor == EdgeDescriptor)
		[[nodiscard]] decltype(auto) vertex_property(VertexDescriptor v)
			requires(not std::same_as<VertexProperty, void>)
		{
			return vertexProperties_[v];
		}

		[[nodiscard]] decltype(auto) vertex_property(VertexDescriptor v) const
			requires(not std::same_as<VertexProperty, void>)
		{
			return vertexProperties_[v];
		}

		[[nodiscard]] decltype(auto) edge_property(EdgeDescriptor e)
			requires(not std::same_as<EdgeProperty, void>)
		{
			return edgeProperties_[e];
		}

		[[nodiscard]] decltype(auto) edge_property(EdgeDescriptor e) const
			requires(not std::same_as<EdgeProperty, void>)
		{
			return edgeProperties_[e];
		}

		// PascalCase named property accessors
		[[nodiscard]] decltype(auto) VertexProperty(VertexDescriptor v)
			requires(not std::same_as<VertexProperty, void>)
		{
			return vertexProperties_[v];
		}

		[[nodiscard]] decltype(auto) VertexProperty(VertexDescriptor v) const
			requires(not std::same_as<VertexProperty, void>)
		{
			return vertexProperties_[v];
		}

		[[nodiscard]] decltype(auto) EdgeProperty(EdgeDescriptor e)
			requires(not std::same_as<EdgeProperty, void>)
		{
			return edgeProperties_[e];
		}

		[[nodiscard]] decltype(auto) EdgeProperty(EdgeDescriptor e) const
			requires(not std::same_as<EdgeProperty, void>)
		{
			return edgeProperties_[e];
		}

		// ========================================================================
		// Raw Data Access (for serialization, #embed loading)
		// ========================================================================

		[[nodiscard]] auto RowOffsetsData() const noexcept
		{
			return rowOffsets_.data();
		}

		[[nodiscard]] auto ColIndicesData() const noexcept
		{
			return colIndices_.data();
		}

		[[nodiscard]] auto VertexPropertiesData() const noexcept
			requires(not std::same_as<VertexProperty, void>)
		{
			return vertexProperties_.data();
		}

		[[nodiscard]] auto EdgePropertiesData() const noexcept
			requires(not std::same_as<EdgeProperty, void>)
		{
			return edgeProperties_.data();
		}
	};

	// ============================================================================
	// Convenience Aliases
	// ============================================================================

	template<std::unsigned_integral IndexType = std::size_t>
	using CSRTopology = CSRStorage<void, void, std::vector, std::vector, IndexType>;

	template<typename VertexProperty, std::unsigned_integral IndexType = std::size_t>
	using CSRVertexStorage = CSRStorage<VertexProperty, void, std::vector, std::vector, IndexType>;

	template<typename EdgeProperty, std::unsigned_integral IndexType = std::size_t>
	using CSREdgeStorage = CSRStorage<void, EdgeProperty, std::vector, std::vector, IndexType>;
}
