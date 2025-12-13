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
	 * @tparam VertexProperty Type stored per vertex (use no_property for none)
	 * @tparam EdgeProperty Type stored per edge (use no_property for none)
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

		using vertex_descriptor    = IndexType;
		using edge_descriptor      = IndexType;
		using vertices_size_type   = IndexType;
		using edges_size_type      = IndexType;
		using degree_size_type     = IndexType;
		using vertex_property_type = VertexProperty;
		using edge_property_type   = EdgeProperty;

		// Compile-time properties
		static constexpr bool is_directed                   = true;
		static constexpr bool allows_parallel_edges         = true;
		static constexpr bool is_constexpr_capable          = true;
		static constexpr bool has_stable_vertex_descriptors = true; // Indices don't move
		static constexpr bool has_stable_edge_descriptors   = true;

		static constexpr vertices_size_type max_vertices = static_cast<vertices_size_type>(MaxVertices);
		static constexpr edges_size_type max_edges       = static_cast<edges_size_type>(MaxEdges);

		// Rebind to different property types
		template<typename VP, typename EP>
		using rebind = StaticStorage<VP, EP, MaxVertices, MaxEdges, IndexType>;

	private:
		// CSR-style storage for O(1) vertex lookup and cache-friendly edge iteration
		//	row_offsets_[v] gives the start index in targets_ for vertex v's out-edges
		//	row_offsets_[v+1] - row_offsets_[v] gives out-degree
		std::array<edges_size_type, MaxVertices + 1> row_offsets_{};
		std::array<vertex_descriptor, MaxEdges> targets_{};

		// Property storage (conditionally empty for no_property via [[no_unique_address]])
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

		[[no_unique_address]] VertexPropsStorage vertex_properties_{};
		[[no_unique_address]] EdgePropsStorage edge_properties_{};

		vertices_size_type num_vertices_{0};
		edges_size_type num_edges_{0};

		// Boundary vertices for subgraph composition (indices into vertex array)
		std::array<vertex_descriptor, MaxVertices> boundary_{};
		vertices_size_type num_boundary_{0};

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		constexpr StaticStorage() = default;

		// ========================================================================
		// Vertex Operations
		// ========================================================================

		[[nodiscard]] constexpr auto num_vertices() const noexcept -> vertices_size_type
		{
			return num_vertices_;
		}

		[[nodiscard]] constexpr auto vertices() const noexcept
		{
			return std::views::iota(vertex_descriptor{0}, num_vertices_);
		}

		constexpr auto add_vertex() -> vertex_descriptor
		{
			if (num_vertices_ >= max_vertices)
			{
				// In constexpr context, this will cause a compile error
				// At runtime, this is undefined behavior (could add assert)
			}
			auto v              = num_vertices_++;
			row_offsets_[v + 1] = row_offsets_[v]; // Initialize with no edges
			return v;
		}

		template<typename... Args>
			requires(not std::same_as<VertexProperty, void>) and std::constructible_from<VertexProperty, Args...>
		constexpr auto add_vertex(Args&&... args) -> vertex_descriptor
		{
			auto v                = add_vertex();
			vertex_properties_[v] = VertexProperty(std::forward<Args>(args)...);
			return v;
		}

		// ========================================================================
		// Edge Operations (Build Phase)
		// ========================================================================

		// Note: Edges must be added in source-vertex order for CSR format.
		// Use the builder pattern or sort edges before finalizing.
		constexpr auto add_edge(vertex_descriptor source, vertex_descriptor target) -> std::pair<edge_descriptor, bool>
		{
			if (num_edges_ >= max_edges)
			{
				return {edge_descriptor{}, false};
			}

			auto edge_idx      = num_edges_++;
			targets_[edge_idx] = target;

			// Update row offsets for all vertices after source
			for (auto v = source + 1; v <= num_vertices_; ++v)
			{
				++row_offsets_[v];
			}

			return {edge_idx, true};
		}

		template<typename... Args>
			requires(not std::same_as<EdgeProperty, void>) and std::constructible_from<EdgeProperty, Args...>
		constexpr auto add_edge(
			vertex_descriptor source,
			vertex_descriptor target,
			Args&&... args)
			-> std::pair<edge_descriptor, bool>
		{
			auto [edge, inserted] = add_edge(source, target);
			if (inserted)
			{
				edge_properties_[edge] = EdgeProperty(std::forward<Args>(args)...);
			}
			return {edge, inserted};
		}

		[[nodiscard]] constexpr auto num_edges() const noexcept -> edges_size_type
		{
			return num_edges_;
		}

		[[nodiscard]] constexpr auto edges() const noexcept
		{
			return std::views::iota(edge_descriptor{0}, num_edges_);
		}

		// ========================================================================
		// Incidence (Out-Edge Traversal)
		// ========================================================================

		[[nodiscard]] constexpr auto out_degree(vertex_descriptor v) const noexcept -> degree_size_type
		{
			return row_offsets_[v + 1] - row_offsets_[v];
		}

		// Returns range of edge descriptors for out-edges of vertex v
		[[nodiscard]] constexpr auto out_edges(vertex_descriptor v) const noexcept
		{
			return std::views::iota(row_offsets_[v], row_offsets_[v + 1]);
		}

		[[nodiscard]] constexpr auto source(edge_descriptor e) const noexcept -> vertex_descriptor
		{
			// Binary search to find which vertex owns this edge
			// O(log V) but typically not called in hot paths
			auto it = std::ranges::upper_bound(row_offsets_ | std::views::take(num_vertices_ + 1), e);
			return static_cast<vertex_descriptor>(std::distance(row_offsets_.begin(), it) - 1);
		}

		[[nodiscard]] constexpr auto target(edge_descriptor e) const noexcept -> vertex_descriptor
		{
			return targets_[e];
		}

		// ========================================================================
		// Adjacency (Direct Neighbor Access)
		// ========================================================================

		[[nodiscard]] constexpr auto adjacent_vertices(vertex_descriptor v) const noexcept
		{
			return std::views::iota(row_offsets_[v], row_offsets_[v + 1]) | std::views::transform(
				       [this](edge_descriptor e)
				       {
					       return targets_[e];
				       });
		}

		// ========================================================================
		// Property Access (O(1) via descriptor index)
		// ========================================================================

		[[nodiscard]] constexpr decltype(auto) operator[](vertex_descriptor v)
			requires(not std::same_as<VertexProperty, void>)
		{
			return (vertex_properties_[v]);
		}

		[[nodiscard]] constexpr decltype(auto) operator[](vertex_descriptor v) const
			requires(not std::same_as<VertexProperty, void>)
		{
			return (vertex_properties_[v]);
		}

		[[nodiscard]] constexpr decltype(auto) operator[](edge_descriptor e)
			requires(not std::same_as<EdgeProperty, void>)
		{
			return (edge_properties_[e]);
		}

		[[nodiscard]] constexpr decltype(auto) operator[](edge_descriptor e) const
			requires(not std::same_as<EdgeProperty, void>)
		{
			return (edge_properties_[e]);
		}

		// ========================================================================
		// Subgraph Composition Support
		// ========================================================================

		// Mark a vertex as a boundary vertex (entry/exit point for composition)
		constexpr void mark_boundary(vertex_descriptor v)
		{
			boundary_[num_boundary_++] = v;
		}

		[[nodiscard]] constexpr auto boundary_vertices() const noexcept
		{
			return std::span{boundary_.data(), num_boundary_};
		}

		[[nodiscard]] constexpr auto num_boundary_vertices() const noexcept -> vertices_size_type
		{
			return num_boundary_;
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
