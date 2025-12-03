export module synodic.library:structure.graph.graph;

import :structure.graph.concepts;
import :structure.graph.storage_traits;
import :structure.graph.storage.adjacency_list;

import std;

export namespace synodic::graph
{
	/**
	 * @brief A graph class template that wraps a storage backend.
	 *        Provides a unified interface regardless of the underlying storage strategy.
	 *        Storage is selected via template parameter, allowing compile-time optimization.
	 *
	 * @tparam Storage The storage backend
	 */
	template<graph_storage Storage>
	class Graph
	{
	public:
		// ========================================================================
		// Type Definitions (forwarded from storage)
		// ========================================================================

		using storage_type = Storage;
		using traits	   = storage_traits<Storage>;

		using vertex_descriptor	   = typename traits::vertex_descriptor;
		using edge_descriptor	   = typename traits::edge_descriptor;
		using vertices_size_type   = typename traits::vertices_size_type;
		using edges_size_type	   = typename traits::edges_size_type;
		using degree_size_type	   = typename Storage::degree_size_type;
		using vertex_property_type = typename traits::vertex_property_type;
		using edge_property_type   = typename traits::edge_property_type;

		// Compile-time properties exposed at graph level
		static constexpr bool is_directed			= traits::is_directed;
		static constexpr bool allows_parallel_edges = traits::allows_parallel_edges;
		static constexpr bool is_constexpr_capable	= traits::is_constexpr_capable;
		static constexpr bool is_mutable			= traits::is_mutable;

	private:
		Storage storage_;

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		constexpr Graph() = default;

		// Construct with pre-built storage (e.g., from #embed data)
		constexpr explicit Graph(Storage storage) :
			storage_(std::move(storage))
		{
		}

		// Forward construction arguments to storage
		template<typename... Args>
			requires std::constructible_from<Storage, Args...>
		constexpr explicit Graph(Args&&... args) :
			storage_(std::forward<Args>(args)...)
		{
		}

		// ========================================================================
		// Storage Access
		// ========================================================================

		[[nodiscard]] constexpr auto storage() noexcept -> Storage&
		{
			return storage_;
		}

		[[nodiscard]] constexpr auto storage() const noexcept -> const Storage&
		{
			return storage_;
		}

		// ========================================================================
		// Vertex Operations
		// ========================================================================

		[[nodiscard]] constexpr auto num_vertices() const noexcept -> vertices_size_type
			requires vertex_list_storage<Storage>
		{
			return storage_.num_vertices();
		}

		[[nodiscard]] constexpr auto vertices() const noexcept
			requires vertex_list_storage<Storage>
		{
			return storage_.vertices();
		}

		constexpr auto add_vertex() -> vertex_descriptor
			requires appendable_vertex_storage<Storage>
		{
			return storage_.add_vertex();
		}

		template<typename... Args>
			requires appendable_vertex_storage<Storage> and vertex_property_storage<Storage> and
					 std::constructible_from<vertex_property_type, Args...>
		constexpr auto add_vertex(Args&&... args) -> vertex_descriptor
		{
			return storage_.add_vertex(std::forward<Args>(args)...);
		}

		constexpr void remove_vertex(vertex_descriptor v)
			requires mutable_vertex_storage<Storage>
		{
			storage_.remove_vertex(v);
		}

		constexpr void clear_vertices()
			requires mutable_vertex_storage<Storage>
		{
			storage_.clear_vertices();
		}

		// ========================================================================
		// Edge Operations
		// ========================================================================

		[[nodiscard]] constexpr auto num_edges() const noexcept -> edges_size_type
			requires edge_list_storage<Storage>
		{
			return storage_.num_edges();
		}

		[[nodiscard]] constexpr auto edges() const noexcept
			requires edge_list_storage<Storage>
		{
			return storage_.edges();
		}

		constexpr auto add_edge(vertex_descriptor source, vertex_descriptor target) -> std::pair<edge_descriptor, bool>
			requires appendable_edge_storage<Storage>
		{
			return storage_.add_edge(source, target);
		}

		template<typename... Args>
			requires appendable_edge_storage<Storage> and edge_property_storage<Storage> and
					 std::constructible_from<edge_property_type, Args...>
		constexpr auto add_edge(vertex_descriptor source, vertex_descriptor target, Args&&... args)
			-> std::pair<edge_descriptor, bool>
		{
			return storage_.add_edge(source, target, std::forward<Args>(args)...);
		}

		constexpr void remove_edge(vertex_descriptor source, vertex_descriptor target)
			requires mutable_edge_storage<Storage>
		{
			storage_.remove_edge(source, target);
		}

		constexpr void remove_edge(edge_descriptor e)
			requires mutable_edge_storage<Storage>
		{
			storage_.remove_edge(e);
		}

		constexpr void clear_edges()
			requires mutable_edge_storage<Storage>
		{
			storage_.clear_edges();
		}

		// ========================================================================
		// Incidence (Out-Edge Traversal)
		// ========================================================================

		[[nodiscard]] constexpr auto out_degree(vertex_descriptor v) const noexcept -> degree_size_type
			requires incidence_storage<Storage>
		{
			return storage_.out_degree(v);
		}

		[[nodiscard]] constexpr auto out_edges(vertex_descriptor v) const noexcept
			requires incidence_storage<Storage>
		{
			return storage_.out_edges(v);
		}

		[[nodiscard]] constexpr auto source(edge_descriptor e) const noexcept -> vertex_descriptor
			requires incidence_storage<Storage>
		{
			return storage_.source(e);
		}

		[[nodiscard]] constexpr auto target(edge_descriptor e) const noexcept -> vertex_descriptor
			requires incidence_storage<Storage>
		{
			return storage_.target(e);
		}

		// ========================================================================
		// Bidirectional (In-Edge Traversal)
		// ========================================================================

		[[nodiscard]] constexpr auto in_degree(vertex_descriptor v) const noexcept -> degree_size_type
			requires bidirectional_storage<Storage>
		{
			return storage_.in_degree(v);
		}

		[[nodiscard]] constexpr auto in_edges(vertex_descriptor v) const noexcept
			requires bidirectional_storage<Storage>
		{
			return storage_.in_edges(v);
		}

		// ========================================================================
		// Adjacency (Direct Neighbor Access)
		// ========================================================================

		[[nodiscard]] constexpr auto adjacent_vertices(vertex_descriptor v) const noexcept
			requires adjacency_storage<Storage>
		{
			return storage_.adjacent_vertices(v);
		}

		// ========================================================================
		// Property Access
		// ========================================================================

		[[nodiscard]] constexpr decltype(auto) operator[](vertex_descriptor v)
			requires vertex_property_storage<Storage>
		{
			return storage_[v];
		}

		[[nodiscard]] constexpr decltype(auto) operator[](vertex_descriptor v) const
			requires vertex_property_storage<Storage>
		{
			return storage_[v];
		}

		[[nodiscard]] constexpr decltype(auto) operator[](edge_descriptor e)
			requires edge_property_storage<Storage>
		{
			return storage_[e];
		}

		[[nodiscard]] constexpr decltype(auto) operator[](edge_descriptor e) const
			requires edge_property_storage<Storage>
		{
			return storage_[e];
		}

		// ========================================================================
		// Subgraph Composition
		// ========================================================================

		[[nodiscard]] constexpr auto boundary_vertices() const noexcept
			requires composable_storage<Storage>
		{
			return storage_.boundary_vertices();
		}

		constexpr void mark_boundary(vertex_descriptor v)
			requires composable_storage<Storage>
		{
			storage_.mark_boundary(v);
		}

		/**
		 * @brief Compose a subgraph into this graph.
		 * @param subgraph The subgraph to merge
		 * @return Container mapping subgraph vertex descriptors to this graph's descriptors
		 */
		template<graph_storage SubgraphStorage>
		auto compose(const Graph<SubgraphStorage>& subgraph)
			requires mutable_storage<Storage> and requires(Storage& s, const SubgraphStorage& sub) { s.compose(sub); }
		{
			return storage_.compose(subgraph.storage());
		}
	};

	// ============================================================================
	// Deduction Guides
	// ============================================================================

	template<graph_storage Storage>
	Graph(Storage) -> Graph<Storage>;

	// ============================================================================
	// Convenience Type Aliases
	// ============================================================================

	// Dynamic graph with vertex and edge properties (general purpose)
	template<typename VertexProperty, typename EdgeProperty = void>
	using DynamicGraph = Graph<storage::AdjacencyListStorage<VertexProperty, EdgeProperty>>;

	// Dynamic bidirectional graph
	template<typename VertexProperty, typename EdgeProperty = void>
	using BidirectionalGraph =
		Graph<storage::AdjacencyListStorage<VertexProperty, EdgeProperty, std::vector, std::vector, std::size_t, true>>;

	// Topology-only dynamic graph (no properties)
	using DynamicTopology = Graph<storage::AdjacencyListTopology<>>;
}
