export module synodic.library:structure.graph.storage.csr;

import :structure.graph.concepts;

import std;

export namespace synodic::graph::storage
{
	/**
	 * @brief Compressed Sparse Row (CSR) graph storage for immutable, cache-friendly graphs.
	 *        Ideal for graphs loaded via #embed where topology is fixed at compile-time
	 *        but properties may be updated at runtime.
	 *
	 *        Memory layout:
	 *        - row_offsets_: [V+1] array where row_offsets_[v] is start of v's edges
	 *        - col_indices_: [E] array of target vertex indices (contiguous per source)
	 *        - Vertex properties: [V] array indexed by vertex descriptor
	 *        - Edge properties: [E] array indexed by edge descriptor
	 *
	 *        Performance characteristics:
	 *        - O(1) vertex property access
	 *        - O(1) edge property access
	 *        - O(1) out-degree query
	 *        - O(degree) iteration over out-edges (cache-friendly sequential access)
	 *        - O(log V) source vertex lookup from edge (rare operation)
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
		template<typename...> typename EdgeContainer   = std::vector,
		std::unsigned_integral IndexType			   = std::size_t>
	class CSRStorage
	{
	public:
		// ========================================================================
		// Type Definitions
		// ========================================================================

		using vertex_descriptor	   = IndexType;
		using edge_descriptor	   = IndexType;
		using vertices_size_type   = IndexType;
		using edges_size_type	   = IndexType;
		using degree_size_type	   = IndexType;
		using vertex_property_type = VertexProperty;
		using edge_property_type   = EdgeProperty;

		// Compile-time properties
		static constexpr bool is_directed					= true;
		static constexpr bool allows_parallel_edges			= true;
		static constexpr bool is_constexpr_capable			= false;  // Uses std::vector
		static constexpr bool has_stable_vertex_descriptors = true;
		static constexpr bool has_stable_edge_descriptors	= true;

		// Rebind to different property types
		template<typename VP, typename EP>
		using rebind = CSRStorage<VP, EP, VertexContainer, EdgeContainer, IndexType>;

	private:
		// CSR structure arrays
		VertexContainer<edges_size_type> row_offsets_;
		EdgeContainer<vertex_descriptor> col_indices_;

		// Property storage (conditionally present)
		struct Empty
		{
		};

		using VertexPropsStorage =
			std::conditional_t<std::same_as<VertexProperty, void>, Empty, VertexContainer<VertexProperty>>;

		using EdgePropsStorage =
			std::conditional_t<std::same_as<EdgeProperty, void>, Empty, EdgeContainer<EdgeProperty>>;

		[[no_unique_address]] VertexPropsStorage vertex_properties_;
		[[no_unique_address]] EdgePropsStorage edge_properties_;

	public:
		// ========================================================================
		// Construction from Edge List
		// ========================================================================

		CSRStorage() :
			row_offsets_(1, 0)
		{
		}

		/**
		 * @brief Construct CSR from sorted edge list.
		 *        Edges must be sorted by source vertex.
		 * @param num_vertices Number of vertices in the graph
		 * @param edges Range of (source, target) pairs, sorted by source
		 */
		template<std::ranges::input_range EdgeRange>
			requires std::
				same_as<std::ranges::range_value_t<EdgeRange>, std::pair<vertex_descriptor, vertex_descriptor>>
			CSRStorage(vertices_size_type num_vertices, EdgeRange&& edges) :
			row_offsets_(num_vertices + 1, 0)
		{
			// Count edges per vertex
			for (const auto& [src, _]: edges)
			{
				++row_offsets_[src + 1];
			}

			// Prefix sum to get offsets
			for (vertices_size_type v = 1; v <= num_vertices; ++v)
			{
				row_offsets_[v] += row_offsets_[v - 1];
			}

			// Allocate edge storage
			col_indices_.resize(row_offsets_[num_vertices]);

			// Fill column indices (using temporary offset copy)
			auto current_offset = row_offsets_;
			for (const auto& [src, tgt]: edges)
			{
				col_indices_[current_offset[src]++] = tgt;
			}

			// Allocate property storage
			if constexpr (not std::same_as<VertexProperty, void>)
			{
				vertex_properties_.resize(num_vertices);
			}
			if constexpr (not std::same_as<EdgeProperty, void>)
			{
				edge_properties_.resize(col_indices_.size());
			}
		}

		/**
		 * @brief Construct CSR with properties from sorted edge list.
		 * @param num_vertices Number of vertices
		 * @param vertex_props Vertex properties (indexed by vertex descriptor)
		 * @param edges Range of (source, target, edge_property) tuples, sorted by source
		 */
		template<std::ranges::input_range VertexPropRange, std::ranges::input_range EdgeRange>
			requires(not std::same_as<VertexProperty, void>) and (not std::same_as<EdgeProperty, void>) and
					std::convertible_to<std::ranges::range_value_t<VertexPropRange>, VertexProperty>
		CSRStorage(vertices_size_type num_vertices, VertexPropRange&& vertex_props, EdgeRange&& edges) :
			row_offsets_(num_vertices + 1, 0)
		{
			// Copy vertex properties
			vertex_properties_.reserve(num_vertices);
			for (auto&& prop: vertex_props)
			{
				vertex_properties_.push_back(std::forward<decltype(prop)>(prop));
			}

			// Count edges per vertex
			for (const auto& edge: edges)
			{
				++row_offsets_[std::get<0>(edge) + 1];
			}

			// Prefix sum
			for (vertices_size_type v = 1; v <= num_vertices; ++v)
			{
				row_offsets_[v] += row_offsets_[v - 1];
			}

			// Allocate
			col_indices_.resize(row_offsets_[num_vertices]);
			edge_properties_.resize(row_offsets_[num_vertices]);

			// Fill
			auto current_offset = row_offsets_;
			for (const auto& edge: edges)
			{
				auto idx			  = current_offset[std::get<0>(edge)]++;
				col_indices_[idx]	  = std::get<1>(edge);
				edge_properties_[idx] = std::get<2>(edge);
			}
		}

		// ========================================================================
		// Vertex Operations
		// ========================================================================

		[[nodiscard]] auto num_vertices() const noexcept -> vertices_size_type
		{
			return static_cast<vertices_size_type>(row_offsets_.size() - 1);
		}

		[[nodiscard]] auto vertices() const noexcept
		{
			return std::views::iota(vertex_descriptor {0}, num_vertices());
		}

		// ========================================================================
		// Edge Operations
		// ========================================================================

		[[nodiscard]] auto num_edges() const noexcept -> edges_size_type
		{
			return static_cast<edges_size_type>(col_indices_.size());
		}

		[[nodiscard]] auto edges() const noexcept
		{
			return std::views::iota(edge_descriptor {0}, num_edges());
		}

		// ========================================================================
		// Incidence (Out-Edge Traversal) - O(1) access, cache-friendly iteration
		// ========================================================================

		[[nodiscard]] auto out_degree(vertex_descriptor v) const noexcept -> degree_size_type
		{
			return static_cast<degree_size_type>(row_offsets_[v + 1] - row_offsets_[v]);
		}

		[[nodiscard]] auto out_edges(vertex_descriptor v) const noexcept
		{
			return std::views::iota(
				static_cast<edge_descriptor>(row_offsets_[v]),
				static_cast<edge_descriptor>(row_offsets_[v + 1]));
		}

		[[nodiscard]] auto source(edge_descriptor e) const noexcept -> vertex_descriptor
		{
			// Binary search in row_offsets to find owning vertex
			auto it = std::ranges::upper_bound(row_offsets_, e);
			return static_cast<vertex_descriptor>(std::distance(row_offsets_.begin(), it) - 1);
		}

		[[nodiscard]] auto target(edge_descriptor e) const noexcept -> vertex_descriptor
		{
			return col_indices_[e];
		}

		// ========================================================================
		// Adjacency (Direct Neighbor Access)
		// ========================================================================

		[[nodiscard]] auto adjacent_vertices(vertex_descriptor v) const noexcept
		{
			return std::span {col_indices_.data() + row_offsets_[v], col_indices_.data() + row_offsets_[v + 1]};
		}

		// ========================================================================
		// Property Access - O(1) via descriptor index
		// ========================================================================

		[[nodiscard]] decltype(auto) operator[](vertex_descriptor v)
			requires(not std::same_as<VertexProperty, void>)
		{
			return (vertex_properties_[v]);
		}

		[[nodiscard]] decltype(auto) operator[](vertex_descriptor v) const
			requires(not std::same_as<VertexProperty, void>)
		{
			return (vertex_properties_[v]);
		}

		[[nodiscard]] decltype(auto) operator[](edge_descriptor e)
			requires(not std::same_as<EdgeProperty, void>)
		{
			return (edge_properties_[e]);
		}

		[[nodiscard]] decltype(auto) operator[](edge_descriptor e) const
			requires(not std::same_as<EdgeProperty, void>)
		{
			return (edge_properties_[e]);
		}

		// ========================================================================
		// Raw Data Access (for serialization, #embed loading)
		// ========================================================================

		[[nodiscard]] auto row_offsets_data() const noexcept
		{
			return row_offsets_.data();
		}

		[[nodiscard]] auto col_indices_data() const noexcept
		{
			return col_indices_.data();
		}

		[[nodiscard]] auto vertex_properties_data() const noexcept
			requires(not std::same_as<VertexProperty, void>)
		{
			return vertex_properties_.data();
		}

		[[nodiscard]] auto edge_properties_data() const noexcept
			requires(not std::same_as<EdgeProperty, void>)
		{
			return edge_properties_.data();
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
