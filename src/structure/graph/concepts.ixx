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
	concept graph_storage = requires
	{
		typename S::VertexDescriptor;
		typename S::EdgeDescriptor;
		typename S::VerticesSizeType;
		typename S::EdgesSizeType;
		typename S::DegreeSizeType;

		requires vertex_descriptor<typename S::VertexDescriptor>;
		requires edge_descriptor<typename S::EdgeDescriptor>;
		requires std::unsigned_integral<typename S::VerticesSizeType>;
		requires std::unsigned_integral<typename S::EdgesSizeType>;
		requires std::unsigned_integral<typename S::DegreeSizeType>;
	};

	// Storage that provides vertex enumeration
	template<typename S>
	concept vertex_list_storage = graph_storage<S> and requires(const S& storage)
	{
		{ storage.Vertices() } -> std::ranges::input_range;
		{ storage.NumVertices() } -> std::same_as<typename S::VerticesSizeType>;
	};

	// Storage that provides edge enumeration
	template<typename S>
	concept edge_list_storage = graph_storage<S> and requires(const S& storage)
	{
		{ storage.Edges() } -> std::ranges::input_range;
		{ storage.NumEdges() } -> std::same_as<typename S::EdgesSizeType>;
	};

	// Storage supporting out-edge traversal
	template<typename S>
	concept incidence_storage =
		graph_storage<S> and requires(const S& storage, typename S::VertexDescriptor v, typename S::EdgeDescriptor e)
		{
			{ storage.OutEdges(v) } -> std::ranges::input_range;
			{ storage.OutDegree(v) } -> std::same_as<typename S::DegreeSizeType>;
			{ storage.Source(e) } -> std::same_as<typename S::VertexDescriptor>;
			{ storage.Target(e) } -> std::same_as<typename S::VertexDescriptor>;
		};

	// Storage supporting in-edge traversal
	template<typename S>
	concept bidirectional_storage =
		incidence_storage<S> and requires(const S& storage, typename S::VertexDescriptor v)
		{
			{ storage.InEdges(v) } -> std::ranges::input_range;
			{ storage.InDegree(v) } -> std::same_as<typename S::DegreeSizeType>;
		};

	// Storage supporting adjacency queries without edge descriptors
	//	More efficient when edge properties are not needed
	template<typename S>
	concept adjacency_storage = graph_storage<S> and requires(const S& storage, typename S::VertexDescriptor v)
	{
		{ storage.AdjacentVertices(v) } -> std::ranges::input_range;
	};

	// ============================================================================
	// Mutable Storage Concepts
	// ============================================================================

	// Storage supporting vertex addition
	template<typename S>
	concept appendable_vertex_storage = graph_storage<S> and requires(S& storage)
	{
		{ storage.AddVertex() } -> std::same_as<typename S::VertexDescriptor>;
	};

	// Storage supporting edge addition
	template<typename S>
	concept appendable_edge_storage =
		graph_storage<S> and requires(S& storage, typename S::VertexDescriptor u, typename S::VertexDescriptor v)
		{
			{ storage.AddEdge(u, v) } -> std::same_as<std::pair<typename S::EdgeDescriptor, bool>>;
		};

	// Storage supporting vertex addition and removal
	template<typename S>
	concept mutable_vertex_storage = appendable_vertex_storage<S> and requires(S& storage)
	{
		{ storage.RemoveVertex(std::declval<typename S::VertexDescriptor>()) };
		{ storage.ClearVertices() };
	};

	// Storage supporting edge addition and removal
	template<typename S>
	concept mutable_edge_storage =
		appendable_edge_storage<S> and
		requires(S& storage, typename S::VertexDescriptor u, typename S::VertexDescriptor v)
		{
			{ storage.RemoveEdge(u, v) };
			{ storage.RemoveEdge(std::declval<typename S::EdgeDescriptor>()) };
			{ storage.ClearEdges() };
		};

	// Fully mutable storage
	template<typename S>
	concept mutable_storage = mutable_vertex_storage<S> and mutable_edge_storage<S>;

	// Append-only storage
	template<typename S>
	concept appendable_storage = appendable_vertex_storage<S> and appendable_edge_storage<S>;

	// ============================================================================
	// Property Access Concepts
	// ============================================================================

	// Storage providing O(1) vertex property access via descriptor
	template<typename S>
	concept vertex_property_storage = graph_storage<S> and requires { typename S::VertexPropertyType; } and
	                                  requires(S& storage, const S& constStorage, typename S::VertexDescriptor v)
	                                  {
		                                  { storage[v] } -> std::same_as<typename S::VertexPropertyType&>;
		                                  { constStorage[v] } -> std::same_as<const typename S::VertexPropertyType&>;
	                                  };

	// Storage providing O(1) edge property access via descriptor
	template<typename S>
	concept edge_property_storage = graph_storage<S> and requires { typename S::EdgePropertyType; } and
	                                requires(S& storage, const S& constStorage, typename S::EdgeDescriptor e)
	                                {
		                                { storage[e] } -> std::same_as<typename S::EdgePropertyType&>;
		                                { constStorage[e] } -> std::same_as<const typename S::EdgePropertyType&>;
	                                };

	// ============================================================================
	// Composable Storage Concepts
	// ============================================================================

	// Storage that can be composed into a larger graph
	//	Subgraphs expose boundary vertices for connection points
	template<typename S>
	concept composable_storage = graph_storage<S> and requires(const S& storage)
	{
		{ storage.BoundaryVertices() } -> std::ranges::input_range;
	};

	// ============================================================================
	// Graph Direction Concepts
	// ============================================================================

	/**
	 * @brief Storage is undirected where edges connect vertices symmetrically.
	 * Undirected storage stores each edge once but makes it accessible from both endpoints.
	 * The opposite() function returns the other endpoint of an edge given one endpoint.
	 */
	template<typename S>
	concept undirected_storage = graph_storage<S> and requires
	                             {
		                             requires S::IS_UNDIRECTED;
	                             } and requires(
	                             const S& storage,
	                             typename S::VertexDescriptor v,
	                             typename S::EdgeDescriptor e)
	                             {
		                             // Total degree. Number of incident edges.
		                             { storage.Degree(v) } -> std::same_as<typename S::DegreeSizeType>;
		                             // All edges incident to vertex v
		                             { storage.IncidentEdges(v) } -> std::ranges::input_range;
		                             // Get the other endpoint of edge e from vertex v
		                             { storage.Opposite(e, v) } -> std::same_as<typename S::VertexDescriptor>;
		                             // Get the endpoints of an edge
		                             { storage.Endpoint1(e) } -> std::same_as<typename S::VertexDescriptor>;
		                             { storage.Endpoint2(e) } -> std::same_as<typename S::VertexDescriptor>;
	                             };

	// ============================================================================
	// Combined Graph Concepts
	// ============================================================================

	// A graph with full traversal capabilities: vertex list, edge list, and incidence
	template<typename S>
	concept traversable_storage = vertex_list_storage<S> and edge_list_storage<S> and incidence_storage<S>;

	// A traversable graph with bidirectional edge access
	template<typename S>
	concept traversable_bidirectional_storage = traversable_storage<S> and bidirectional_storage<S>;

	// A traversable undirected graph
	template<typename S>
	concept traversable_undirected_storage = undirected_storage<S> and vertex_list_storage<S> and edge_list_storage<S>;

	// A graph suitable for flow algorithms requiring edge properties for capacity and flow
	template<typename S>
	concept flow_network_storage =
		traversable_bidirectional_storage<S> and edge_property_storage<S> and mutable_edge_storage<S>;
}
