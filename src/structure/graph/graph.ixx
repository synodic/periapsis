export module synodic.periapsis:structure.graph.graph;

import :structure.graph.concepts;
import :structure.graph.storage_traits;
import :structure.graph.storage.adjacency_list;
import :structure.graph.storage.static_storage;

import std;

export namespace peri::graph
{
	// ============================================================================
	// Storage Selection Helper
	// ============================================================================

	namespace detail
	{
		/// Sentinel value indicating dynamic/growable storage
		inline constexpr std::size_t dynamic_extent = std::numeric_limits<std::size_t>::max();

		/// Select storage type based on size parameters
		template<typename VertexProperty, typename EdgeProperty, std::size_t MaxVertices, std::size_t MaxEdges>
		struct storage_selector
		{
			using type = std::conditional_t<
				(MaxVertices == dynamic_extent || MaxEdges == dynamic_extent),
				storage::AdjacencyListStorage<VertexProperty, EdgeProperty>,
				storage::StaticStorage<VertexProperty, EdgeProperty, MaxVertices, MaxEdges>>;
		};

		template<typename VertexProperty, typename EdgeProperty, std::size_t MaxVertices, std::size_t MaxEdges>
		using storage_selector_t = typename storage_selector<VertexProperty, EdgeProperty, MaxVertices, MaxEdges>::type;
	}

	// ============================================================================
	// Graph Class
	// ============================================================================

	/**
	 * @brief A graph data structure with automatic storage selection.
	 *
	 *        - Fixed size (MaxV, MaxE specified): Uses constexpr-capable static storage
	 *        - Dynamic size (defaults): Uses growable adjacency list storage
	 *
	 *        Constexpr graphs become immutable when stored in constexpr variables
	 *        (language enforces const). Dynamic graphs can be mutated at runtime.
	 *
	 *        A dynamic graph can be seeded from a constexpr graph via constructor.
	 *
	 * @tparam VertexProperty Type stored per vertex (void for none)
	 * @tparam EdgeProperty Type stored per edge (void for none)
	 * @tparam MaxVertices Maximum vertices (dynamic_extent for growable)
	 * @tparam MaxEdges Maximum edges (dynamic_extent for growable)
	 */
	template<
		typename VertexProperty = void,
		typename EdgeProperty	= void,
		std::size_t MaxVertices = detail::dynamic_extent,
		std::size_t MaxEdges	= detail::dynamic_extent>
	class Graph
	{
	public:
		// ========================================================================
		// Type Definitions
		// ========================================================================

		using storage_type = detail::storage_selector_t<VertexProperty, EdgeProperty, MaxVertices, MaxEdges>;
		using traits	   = storage_traits<storage_type>;

		using vertex_descriptor	   = typename traits::vertex_descriptor;
		using edge_descriptor	   = typename traits::edge_descriptor;
		using vertices_size_type   = typename traits::vertices_size_type;
		using edges_size_type	   = typename traits::edges_size_type;
		using degree_size_type	   = typename storage_type::degree_size_type;
		using vertex_property_type = VertexProperty;
		using edge_property_type   = EdgeProperty;

		// Compile-time properties
		static constexpr bool is_directed			= traits::is_directed;
		static constexpr bool allows_parallel_edges = traits::allows_parallel_edges;
		static constexpr bool is_constexpr_capable	= traits::is_constexpr_capable;
		static constexpr bool is_dynamic			= (MaxVertices == detail::dynamic_extent);

		static constexpr std::size_t max_vertices = MaxVertices;
		static constexpr std::size_t max_edges	  = MaxEdges;

	private:
		storage_type storage_;

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		constexpr Graph() = default;

		/// Construct with explicit storage
		constexpr explicit Graph(storage_type storage) :
			storage_(std::move(storage))
		{
		}

		/// Seed from a fixed-size graph (enables constexpr -> dynamic extension)
		template<std::size_t OtherMaxV, std::size_t OtherMaxE>
			requires(is_dynamic) and (OtherMaxV != detail::dynamic_extent)
		Graph(const Graph<VertexProperty, EdgeProperty, OtherMaxV, OtherMaxE>& source)
		{
			// Copy all vertices
			for (auto v: source.vertices())
			{
				if constexpr (not std::same_as<VertexProperty, void>)
				{
					add_vertex(source[v]);
				}
				else
				{
					add_vertex();
				}
			}

			// Copy all edges
			for (auto e: source.edges())
			{
				auto src = source.source(e);
				auto tgt = source.target(e);
				if constexpr (not std::same_as<EdgeProperty, void>)
				{
					add_edge(src, tgt, source[e]);
				}
				else
				{
					add_edge(src, tgt);
				}
			}
		}

		// ========================================================================
		// Vertex Operations
		// ========================================================================

		[[nodiscard]] constexpr auto num_vertices() const noexcept -> vertices_size_type
		{
			return storage_.num_vertices();
		}

		[[nodiscard]] constexpr auto vertices() const noexcept
		{
			return storage_.vertices();
		}

		constexpr auto add_vertex() -> vertex_descriptor
		{
			return storage_.add_vertex();
		}

		template<typename... Args>
			requires(not std::same_as<VertexProperty, void>) and std::constructible_from<VertexProperty, Args...>
		constexpr auto add_vertex(Args&&... args) -> vertex_descriptor
		{
			return storage_.add_vertex(std::forward<Args>(args)...);
		}

		constexpr void remove_vertex(vertex_descriptor v)
			requires mutable_vertex_storage<storage_type>
		{
			storage_.remove_vertex(v);
		}

		constexpr void clear_vertices()
			requires mutable_vertex_storage<storage_type>
		{
			storage_.clear_vertices();
		}

		// ========================================================================
		// Edge Operations
		// ========================================================================

		[[nodiscard]] constexpr auto num_edges() const noexcept -> edges_size_type
		{
			return storage_.num_edges();
		}

		[[nodiscard]] constexpr auto edges() const noexcept
		{
			return storage_.edges();
		}

		constexpr auto add_edge(vertex_descriptor source, vertex_descriptor target) -> std::pair<edge_descriptor, bool>
		{
			return storage_.add_edge(source, target);
		}

		template<typename... Args>
			requires(not std::same_as<EdgeProperty, void>) and std::constructible_from<EdgeProperty, Args...>
		constexpr auto add_edge(vertex_descriptor source, vertex_descriptor target, Args&&... args)
			-> std::pair<edge_descriptor, bool>
		{
			return storage_.add_edge(source, target, std::forward<Args>(args)...);
		}

		constexpr void remove_edge(vertex_descriptor source, vertex_descriptor target)
			requires mutable_edge_storage<storage_type>
		{
			storage_.remove_edge(source, target);
		}

		constexpr void remove_edge(edge_descriptor e)
			requires mutable_edge_storage<storage_type>
		{
			storage_.remove_edge(e);
		}

		constexpr void clear_edges()
			requires mutable_edge_storage<storage_type>
		{
			storage_.clear_edges();
		}

		// ========================================================================
		// Incidence (Out-Edge Traversal)
		// ========================================================================

		[[nodiscard]] constexpr auto out_degree(vertex_descriptor v) const noexcept -> degree_size_type
		{
			return storage_.out_degree(v);
		}

		[[nodiscard]] constexpr auto out_edges(vertex_descriptor v) const noexcept
		{
			return storage_.out_edges(v);
		}

		[[nodiscard]] constexpr auto source(edge_descriptor e) const noexcept -> vertex_descriptor
		{
			return storage_.source(e);
		}

		[[nodiscard]] constexpr auto target(edge_descriptor e) const noexcept -> vertex_descriptor
		{
			return storage_.target(e);
		}

		// ========================================================================
		// Bidirectional (In-Edge Traversal)
		// ========================================================================

		[[nodiscard]] constexpr auto in_degree(vertex_descriptor v) const noexcept -> degree_size_type
			requires bidirectional_storage<storage_type>
		{
			return storage_.in_degree(v);
		}

		[[nodiscard]] constexpr auto in_edges(vertex_descriptor v) const noexcept
			requires bidirectional_storage<storage_type>
		{
			return storage_.in_edges(v);
		}

		// ========================================================================
		// Adjacency (Direct Neighbor Access)
		// ========================================================================

		[[nodiscard]] constexpr auto adjacent_vertices(vertex_descriptor v) const noexcept
			requires adjacency_storage<storage_type>
		{
			return storage_.adjacent_vertices(v);
		}

		// ========================================================================
		// Property Access
		// ========================================================================

		[[nodiscard]] constexpr decltype(auto) operator[](vertex_descriptor v)
			requires(not std::same_as<VertexProperty, void>)
		{
			return storage_[v];
		}

		[[nodiscard]] constexpr decltype(auto) operator[](vertex_descriptor v) const
			requires(not std::same_as<VertexProperty, void>)
		{
			return storage_[v];
		}

		[[nodiscard]] constexpr decltype(auto) operator[](edge_descriptor e)
			requires(not std::same_as<EdgeProperty, void>)
		{
			return storage_[e];
		}

		[[nodiscard]] constexpr decltype(auto) operator[](edge_descriptor e) const
			requires(not std::same_as<EdgeProperty, void>)
		{
			return storage_[e];
		}

		// ========================================================================
		// Subgraph Composition
		// ========================================================================

		[[nodiscard]] constexpr auto boundary_vertices() const noexcept
			requires composable_storage<storage_type>
		{
			return storage_.boundary_vertices();
		}

		constexpr void mark_boundary(vertex_descriptor v)
			requires composable_storage<storage_type>
		{
			storage_.mark_boundary(v);
		}

		// ========================================================================
		// Advanced: Direct Storage Access (for power users)
		// ========================================================================

		[[nodiscard]] constexpr auto storage() noexcept -> storage_type&
		{
			return storage_;
		}

		[[nodiscard]] constexpr auto storage() const noexcept -> const storage_type&
		{
			return storage_;
		}
	};

	// ============================================================================
	// Deduction Guides
	// ============================================================================

	// From storage directly
	template<graph_storage Storage>
	Graph(Storage) -> Graph<
		typename storage_traits<Storage>::vertex_property_type,
		typename storage_traits<Storage>::edge_property_type>;

	// ============================================================================
	// Convenience Constants
	// ============================================================================

	/// Sentinel for dynamic/growable graphs
	inline constexpr std::size_t dynamic_extent = detail::dynamic_extent;

	// ============================================================================
	// Convenience Type Aliases (for backwards compatibility and ease of use)
	// ============================================================================

	/// Dynamic graph (default) - growable at runtime
	template<typename VertexProperty = void, typename EdgeProperty = void>
	using DynamicGraph = Graph<VertexProperty, EdgeProperty>;

	/// Fixed-size constexpr-capable graph
	template<typename VertexProperty, typename EdgeProperty, std::size_t MaxV, std::size_t MaxE>
	using FixedGraph = Graph<VertexProperty, EdgeProperty, MaxV, MaxE>;

	/// Topology-only dynamic graph (no properties)
	using DynamicTopology = Graph<void, void>;
}
