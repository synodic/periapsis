export module synodic.periapsis:structure.graph.concepts;

import std;

export namespace peri::graph
{
	// ============================================================================
	// Descriptor Types
	// ============================================================================

	// A vertex descriptor provides O(1) access to vertex data
	//	For performance, descriptors should be trivially copyable indices or pointers
	template<typename D>
	concept vertex_descriptor = std::regular<D> and std::totally_ordered<D>;

	// An edge descriptor provides O(1) access to edge data
	template<typename D>
	concept edge_descriptor = std::regular<D>;

	// ============================================================================
	// Core Graph Storage Concepts
	// ============================================================================

	// Minimal requirements for any graph storage backend
	//	Storage is responsible for holding vertices, edges, and their relationships
	template<typename S>
	concept graph_storage = requires {
		typename S::vertex_descriptor;
		typename S::edge_descriptor;
		typename S::vertices_size_type;
		typename S::edges_size_type;
		typename S::degree_size_type;

		requires vertex_descriptor<typename S::vertex_descriptor>;
		requires edge_descriptor<typename S::edge_descriptor>;
		requires std::unsigned_integral<typename S::vertices_size_type>;
		requires std::unsigned_integral<typename S::edges_size_type>;
		requires std::unsigned_integral<typename S::degree_size_type>;
	};

	// Storage that provides vertex enumeration
	template<typename S>
	concept vertex_list_storage = graph_storage<S> and requires(const S& storage) {
		{ storage.vertices() } -> std::ranges::input_range;
		{ storage.num_vertices() } -> std::same_as<typename S::vertices_size_type>;
	};

	// Storage that provides edge enumeration
	template<typename S>
	concept edge_list_storage = graph_storage<S> and requires(const S& storage) {
		{ storage.edges() } -> std::ranges::input_range;
		{ storage.num_edges() } -> std::same_as<typename S::edges_size_type>;
	};

	// Storage supporting out-edge traversal (directed graphs)
	//	Provides O(1) access to outgoing edges from a vertex
	template<typename S>
	concept incidence_storage =
		graph_storage<S> and
		requires(const S& storage, typename S::vertex_descriptor v, typename S::edge_descriptor e) {
			{ storage.out_edges(v) } -> std::ranges::input_range;
			{ storage.out_degree(v) } -> std::same_as<typename S::degree_size_type>;
			{ storage.source(e) } -> std::same_as<typename S::vertex_descriptor>;
			{ storage.target(e) } -> std::same_as<typename S::vertex_descriptor>;
		};

	// Storage supporting in-edge traversal (bidirectional graphs)
	//	Required for efficient predecessor queries
	template<typename S>
	concept bidirectional_storage =
		incidence_storage<S> and requires(const S& storage, typename S::vertex_descriptor v) {
			{ storage.in_edges(v) } -> std::ranges::input_range;
			{ storage.in_degree(v) } -> std::same_as<typename S::degree_size_type>;
		};

	// Storage supporting adjacency queries without edge descriptors
	//	More efficient when edge properties are not needed
	template<typename S>
	concept adjacency_storage = graph_storage<S> and requires(const S& storage, typename S::vertex_descriptor v) {
		{ storage.adjacent_vertices(v) } -> std::ranges::input_range;
	};

	// ============================================================================
	// Mutable Storage Concepts
	// ============================================================================

	// Storage supporting vertex addition (append-only, no removal required)
	template<typename S>
	concept appendable_vertex_storage = graph_storage<S> and requires(S& storage) {
		{ storage.add_vertex() } -> std::same_as<typename S::vertex_descriptor>;
	};

	// Storage supporting edge addition (append-only, no removal required)
	template<typename S>
	concept appendable_edge_storage =
		graph_storage<S> and requires(S& storage, typename S::vertex_descriptor u, typename S::vertex_descriptor v) {
			{ storage.add_edge(u, v) } -> std::same_as<std::pair<typename S::edge_descriptor, bool>>;
		};

	// Storage supporting vertex addition and removal
	template<typename S>
	concept mutable_vertex_storage = appendable_vertex_storage<S> and requires(S& storage) {
		{ storage.remove_vertex(std::declval<typename S::vertex_descriptor>()) };
		{ storage.clear_vertices() };
	};

	// Storage supporting edge addition and removal
	template<typename S>
	concept mutable_edge_storage =
		appendable_edge_storage<S> and
		requires(S& storage, typename S::vertex_descriptor u, typename S::vertex_descriptor v) {
			{ storage.remove_edge(u, v) };
			{ storage.remove_edge(std::declval<typename S::edge_descriptor>()) };
			{ storage.clear_edges() };
		};

	// Fully mutable storage (add and remove)
	template<typename S>
	concept mutable_storage = mutable_vertex_storage<S> and mutable_edge_storage<S>;

	// Append-only storage (add but no remove - like StaticStorage)
	template<typename S>
	concept appendable_storage = appendable_vertex_storage<S> and appendable_edge_storage<S>;

	// ============================================================================
	// Property Access Concepts
	// ============================================================================

	// Storage providing O(1) vertex property access via descriptor
	template<typename S>
	concept vertex_property_storage = graph_storage<S> and requires { typename S::vertex_property_type; } and
									  requires(S& storage, const S& constStorage, typename S::vertex_descriptor v) {
										  { storage[v] } -> std::same_as<typename S::vertex_property_type&>;
										  { constStorage[v] } -> std::same_as<const typename S::vertex_property_type&>;
									  };

	// Storage providing O(1) edge property access via descriptor
	template<typename S>
	concept edge_property_storage = graph_storage<S> and requires { typename S::edge_property_type; } and
									requires(S& storage, const S& constStorage, typename S::edge_descriptor e) {
										{ storage[e] } -> std::same_as<typename S::edge_property_type&>;
										{ constStorage[e] } -> std::same_as<const typename S::edge_property_type&>;
									};

	// ============================================================================
	// Compile-Time Graph Concepts
	// ============================================================================

	// Storage that can be constructed and manipulated at compile-time
	template<typename S>
	concept constexpr_storage = graph_storage<S> and requires { requires S::is_constexpr_capable; };

	// Storage that can be composed into a larger graph
	//	Subgraphs expose boundary vertices for connection points
	template<typename S>
	concept composable_storage = graph_storage<S> and requires(const S& storage) {
		{ storage.boundary_vertices() } -> std::ranges::input_range;
	};

	// ============================================================================
	// Graph Direction Concepts
	// ============================================================================

	template<typename S>
	concept directed_storage = graph_storage<S> and requires { requires S::is_directed; };

	template<typename S>
	concept undirected_storage = graph_storage<S> and requires { requires not S::is_directed; };

	// ============================================================================
	// Combined Graph Concepts (for algorithm constraints)
	// ============================================================================

	// A complete directed graph with all traversal capabilities
	template<typename S>
	concept traversable_directed_storage =
		directed_storage<S> and vertex_list_storage<S> and edge_list_storage<S> and incidence_storage<S>;

	// A complete bidirectional graph
	template<typename S>
	concept traversable_bidirectional_storage = traversable_directed_storage<S> and bidirectional_storage<S>;

	// A graph suitable for flow algorithms (needs edge properties for capacity/flow)
	template<typename S>
	concept flow_network_storage =
		traversable_bidirectional_storage<S> and edge_property_storage<S> and mutable_edge_storage<S>;
}
