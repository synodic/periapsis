export module synodic.periapsis:structure.graph.graph;

import :structure.graph.concepts;
import :structure.graph.storage_traits;
import :structure.graph.storage.adjacency_list;
import :structure.graph.storage.static_storage;
import :structure.graph.storage.bidirectional_fixed;
import :structure.graph.storage.undirected_adjacency_list;
import :structure.graph.storage.undirected_fixed;

import std;

export namespace peri::graph
{
	// ============================================================================
	// Storage Policy Types
	// ============================================================================

	/**
	 * @brief Specifies the directionality of a graph.
	 */
	enum class Directionality
	{
		DIRECTED, // Forward-only edge traversal
		BIDIRECTIONAL, // Both forward and backward edge traversal
		UNDIRECTED // Edges are symmetric
	};

	/**
	 * @brief Policy for dynamic graph storage.
	 * @tparam Dir Edge directionality semantics
	 */
	template<Directionality Dir = Directionality::DIRECTED>
	struct Dynamic
	{
		static constexpr bool IS_DYNAMIC               = true;
		static constexpr Directionality DIRECTIONALITY = Dir;
	};

	/**
	 * @brief Policy for fixed-size constexpr-capable graph storage.
	 * @tparam MaxV Maximum number of vertices
	 * @tparam MaxE Maximum number of edges
	 * @tparam Dir Edge directionality semantics
	 */
	template<std::size_t MaxV, std::size_t MaxE, Directionality Dir = Directionality::DIRECTED>
	struct Fixed
	{
		static constexpr bool IS_DYNAMIC               = false;
		static constexpr std::size_t MAX_VERTICES      = MaxV;
		static constexpr std::size_t MAX_EDGES         = MaxE;
		static constexpr Directionality DIRECTIONALITY = Dir;
	};

	// ============================================================================
	// Storage Selection
	// ============================================================================

	namespace detail
	{
		/**
		 * @brief Select storage type based on policy
		 */
		template<typename VertexProperty, typename EdgeProperty, typename Policy>
		struct StorageSelector;

		// Dynamic storage specialization
		template<typename VertexProperty, typename EdgeProperty, Directionality Dir>
		struct StorageSelector<VertexProperty, EdgeProperty, Dynamic<Dir>>
		{
			static constexpr bool IS_BIDIRECTIONAL = Dir == Directionality::BIDIRECTIONAL;
			static constexpr bool IS_UNDIRECTED    = Dir == Directionality::UNDIRECTED;

			using Type = std::conditional_t<
				IS_UNDIRECTED,
				storage::UndirectedAdjacencyList<VertexProperty, EdgeProperty, std::vector, std::vector, std::size_t>,
				std::conditional_t<
					IS_BIDIRECTIONAL,
					storage::
					AdjacencyListStorage<VertexProperty, EdgeProperty, std::vector, std::vector, std::size_t, true>,
					storage::AdjacencyListStorage<
						VertexProperty,
						EdgeProperty,
						std::vector,
						std::vector,
						std::size_t,
						false>>>;
		};

		// Fixed storage specialization
		template<typename VertexProperty, typename EdgeProperty, std::size_t MaxV, std::size_t MaxE, Directionality Dir>
		struct StorageSelector<VertexProperty, EdgeProperty, Fixed<MaxV, MaxE, Dir>>
		{
			static constexpr bool IS_BIDIRECTIONAL = Dir == Directionality::BIDIRECTIONAL;
			static constexpr bool IS_UNDIRECTED    = Dir == Directionality::UNDIRECTED;

			using Type = std::conditional_t<
				IS_UNDIRECTED,
				storage::UndirectedFixed<VertexProperty, EdgeProperty, MaxV, MaxE>,
				std::conditional_t<
					IS_BIDIRECTIONAL,
					storage::BidirectionalFixed<VertexProperty, EdgeProperty, MaxV, MaxE>,
					storage::StaticStorage<VertexProperty, EdgeProperty, MaxV, MaxE>>>;
		};

		template<typename VertexProperty, typename EdgeProperty, typename Policy>
		using StorageSelector_t = typename StorageSelector<VertexProperty, EdgeProperty, Policy>::Type;
	}

	// ============================================================================
	// Graph Class
	// ============================================================================

	/**
	 * @brief A graph data structure with policy-based storage selection.
	 *
	 * @tparam VertexProperty Type stored per vertex (void for none)
	 * @tparam EdgeProperty Type stored per edge (void for none)
	 * @tparam Policy Storage policy: Dynamic<Dir> or Fixed<MaxV, MaxE, Dir>
	 */
	template<typename VertexProperty = void, typename EdgeProperty = void, typename Policy = Dynamic<>>
	class Graph
	{
	public:
		// ========================================================================
		// Type Definitions
		// ========================================================================

		using PolicyType  = Policy;
		using StorageType = detail::StorageSelector_t<VertexProperty, EdgeProperty, Policy>;
		using Traits      = StorageTraits<StorageType>;

		using VertexDescriptor   = Traits::VertexDescriptor;
		using EdgeDescriptor     = Traits::EdgeDescriptor;
		using VerticesSizeType   = Traits::VerticesSizeType;
		using EdgesSizeType      = Traits::EdgesSizeType;
		using DegreeSizeType     = StorageType::DegreeSizeType;
		using VertexPropertyType = VertexProperty;
		using EdgePropertyType   = EdgeProperty;

		// Compile-time properties from policy
		static constexpr Directionality DIRECTIONALITY = Policy::DIRECTIONALITY;
		static constexpr bool IS_DYNAMIC               = Policy::IS_DYNAMIC;
		static constexpr bool IS_UNDIRECTED            = DIRECTIONALITY == Directionality::UNDIRECTED;
		static constexpr bool IS_BIDIRECTIONAL         = DIRECTIONALITY == Directionality::BIDIRECTIONAL;

		// Compile-time properties from traits
		static constexpr bool IS_DIRECTED           = Traits::IS_DIRECTED;
		static constexpr bool ALLOWS_PARALLEL_EDGES = Traits::ALLOWS_PARALLEL_EDGES;

	private:
		StorageType storage_;

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		constexpr Graph() = default;

		/**
		 * @brief Construct with explicit storage
		 */
		constexpr explicit Graph(StorageType storage) :
			storage_(std::move(storage))
		{
		}

		/**
		 * @brief Seed from another graph
		 */
		template<typename OtherPolicy>
			requires IS_DYNAMIC and (not OtherPolicy::IS_DYNAMIC)
		Graph(const Graph<VertexProperty, EdgeProperty, OtherPolicy>& source)
		{
			// Copy all vertices
			for (auto v: source.vertices())
			{
				if constexpr (not std::same_as<VertexProperty, void>)
				{
					AddVertex(source.VertexProperty(v));
				}
				else
				{
					AddVertex();
				}
			}

			// Copy all edges
			for (auto e: source.Edges())
			{
				auto src = source.Source(e);
				auto tgt = source.Target(e);
				if constexpr (not std::same_as<EdgeProperty, void>)
				{
					AddEdge(src, tgt, source.EdgeProperty(e));
				}
				else
				{
					AddEdge(src, tgt);
				}
			}
		}

		// ========================================================================
		// Vertex Operations
		// ========================================================================

		[[nodiscard]] constexpr auto NumVertices() const noexcept -> VerticesSizeType
		{
			return storage_.NumVertices();
		}

		[[nodiscard]] constexpr auto Vertices() const noexcept
		{
			return storage_.Vertices();
		}

		constexpr auto AddVertex() -> VertexDescriptor
		{
			return storage_.AddVertex();
		}

		template<typename... Args>
			requires(not std::same_as<VertexProperty, void>) and std::constructible_from<VertexProperty, Args...>
		constexpr auto AddVertex(Args&&... args) -> VertexDescriptor
		{
			return storage_.AddVertex(std::forward<Args>(args)...);
		}

		constexpr void RemoveVertex(VertexDescriptor v)
			requires mutable_vertex_storage<StorageType>
		{
			storage_.RemoveVertex(v);
		}

		constexpr void ClearVertices()
			requires mutable_vertex_storage<StorageType>
		{
			storage_.ClearVertices();
		}

		// ========================================================================
		// Edge Operations
		// ========================================================================

		[[nodiscard]] constexpr auto NumEdges() const noexcept -> EdgesSizeType
		{
			return storage_.NumEdges();
		}

		[[nodiscard]] constexpr auto Edges() const noexcept
		{
			return storage_.Edges();
		}

		constexpr auto AddEdge(VertexDescriptor source, VertexDescriptor target) -> std::pair<EdgeDescriptor, bool>
		{
			return storage_.AddEdge(source, target);
		}

		template<typename... Args>
			requires(not std::same_as<EdgeProperty, void>) and std::constructible_from<EdgeProperty, Args...>
		constexpr auto AddEdge(
			VertexDescriptor source,
			VertexDescriptor target,
			Args&&... args)
			-> std::pair<EdgeDescriptor, bool>
		{
			return storage_.AddEdge(source, target, std::forward<Args>(args)...);
		}

		constexpr void RemoveEdge(VertexDescriptor source, VertexDescriptor target)
			requires mutable_edge_storage<StorageType>
		{
			storage_.RemoveEdge(source, target);
		}

		constexpr void RemoveEdge(EdgeDescriptor e)
			requires mutable_edge_storage<StorageType>
		{
			storage_.RemoveEdge(e);
		}

		constexpr void ClearEdges()
			requires mutable_edge_storage<StorageType>
		{
			storage_.ClearEdges();
		}

		// ========================================================================
		// Incidence
		// ========================================================================

		[[nodiscard]] constexpr auto OutDegree(VertexDescriptor v) const noexcept -> DegreeSizeType
		{
			return storage_.OutDegree(v);
		}

		[[nodiscard]] constexpr auto OutEdges(VertexDescriptor v) const noexcept
		{
			return storage_.OutEdges(v);
		}

		[[nodiscard]] constexpr auto Source(EdgeDescriptor e) const noexcept -> VertexDescriptor
		{
			return storage_.Source(e);
		}

		[[nodiscard]] constexpr auto Target(EdgeDescriptor e) const noexcept -> VertexDescriptor
		{
			return storage_.Target(e);
		}

		// ========================================================================
		// Bidirectional
		// ========================================================================

		[[nodiscard]] constexpr auto InDegree(VertexDescriptor v) const noexcept -> DegreeSizeType requires
			bidirectional_storage<StorageType>
		{
			return storage_.InDegree(v);
		}

		[[nodiscard]] constexpr auto InEdges(VertexDescriptor v) const noexcept
			requires bidirectional_storage<StorageType>
		{
			return storage_.InEdges(v);
		}

		// ========================================================================
		// Undirected Graph Operations
		// ========================================================================

		// Total degree (number of incident edges)
		[[nodiscard]] constexpr auto Degree(VertexDescriptor v) const noexcept -> DegreeSizeType requires
			undirected_storage<StorageType>
		{
			return storage_.Degree(v);
		}

		// All edges incident to vertex v
		[[nodiscard]] constexpr auto IncidentEdges(VertexDescriptor v) const noexcept
			requires undirected_storage<StorageType>
		{
			return storage_.IncidentEdges(v);
		}

		// Get the other endpoint of edge e from vertex v
		[[nodiscard]] constexpr auto Opposite(EdgeDescriptor e, VertexDescriptor v) const noexcept -> VertexDescriptor
			requires undirected_storage<StorageType>
		{
			return storage_.Opposite(e, v);
		}

		// Get first endpoint of edge
		[[nodiscard]] constexpr auto Endpoint1(EdgeDescriptor e) const noexcept -> VertexDescriptor requires
			undirected_storage<StorageType>
		{
			return storage_.Endpoint1(e);
		}

		// Get second endpoint of edge
		[[nodiscard]] constexpr auto Endpoint2(EdgeDescriptor e) const noexcept -> VertexDescriptor requires
			undirected_storage<StorageType>
		{
			return storage_.Endpoint2(e);
		}

		// ========================================================================
		// Adjacency
		// ========================================================================

		[[nodiscard]] constexpr auto AdjacentVertices(VertexDescriptor v) const noexcept
			requires adjacency_storage<StorageType>
		{
			return storage_.AdjacentVertices(v);
		}

		// ========================================================================
		// Property Access
		// ========================================================================

		/// Access vertex property
		[[nodiscard]] constexpr decltype(auto) VertexProperty(VertexDescriptor v)
			requires(not std::same_as<VertexProperty, void>)
		{
			return storage_.VertexProperty(v);
		}

		[[nodiscard]] constexpr decltype(auto) VertexProperty(VertexDescriptor v) const
			requires(not std::same_as<VertexProperty, void>)
		{
			return storage_.VertexProperty(v);
		}

		// Access edge property
		[[nodiscard]] constexpr decltype(auto) EdgeProperty(EdgeDescriptor e)
			requires(not std::same_as<EdgeProperty, void>)
		{
			return storage_.EdgeProperty(e);
		}

		[[nodiscard]] constexpr decltype(auto) EdgeProperty(EdgeDescriptor e) const
			requires(not std::same_as<EdgeProperty, void>)
		{
			return storage_.EdgeProperty(e);
		}

		// Convenience operator[] - use VertexProperty/EdgeProperty if ambiguous
		[[nodiscard]] constexpr decltype(auto) operator[](VertexDescriptor v)
			requires(not std::same_as<VertexPropertyType, void>)
		{
			return storage_[v];
		}

		[[nodiscard]] constexpr decltype(auto) operator[](VertexDescriptor v) const
			requires(not std::same_as<VertexPropertyType, void>)
		{
			return storage_[v];
		}

		[[nodiscard]] constexpr decltype(auto) operator[](EdgeDescriptor e)
			requires(not std::same_as<EdgePropertyType, void>)
		{
			return storage_[e];
		}

		[[nodiscard]] constexpr decltype(auto) operator[](EdgeDescriptor e) const
			requires(not std::same_as<EdgePropertyType, void>)
		{
			return storage_[e];
		}

		// ========================================================================
		// Advanced: Direct Storage Access
		// ========================================================================

		[[nodiscard]] constexpr auto Storage() noexcept -> StorageType&
		{
			return storage_;
		}

		[[nodiscard]] constexpr auto Storage() const noexcept -> const StorageType&
		{
			return storage_;
		}
	};

	// ============================================================================
	// ComposableGraph Wrapper
	// ============================================================================

	/**
	 * @brief Wrapper that adds subgraph composition support to a graph.
	 *        Boundary vertices define connection points for composing subgraphs.
	 */
	template<typename GraphType>
		requires composable_storage<typename GraphType::StorageType>
	class ComposableGraph : public GraphType
	{
	public:
		using typename GraphType::VertexDescriptor;

		using GraphType::GraphType; // Inherit constructors

		[[nodiscard]] constexpr auto BoundaryVertices() const noexcept
		{
			return this->Storage().BoundaryVertices();
		}

		constexpr void MarkBoundary(VertexDescriptor v)
		{
			this->Storage().MarkBoundary(v);
		}

		[[nodiscard]] constexpr auto NumBoundaryVertices() const noexcept
		{
			return this->Storage().NumBoundaryVertices();
		}
	};

	// ============================================================================
	// Deduction Guides
	// ============================================================================

	// From storage directly
	template<graph_storage Storage>
	Graph(
		Storage) -> Graph<
		typename StorageTraits<Storage>::VertexPropertyType,
		typename StorageTraits<Storage>::EdgePropertyType>;

	// ============================================================================
	// Convenience Type Aliases
	// ============================================================================

	// Dynamic directed graph (default)
	template<typename VertexProperty = void, typename EdgeProperty = void>
	using DynamicGraph = Graph<VertexProperty, EdgeProperty, Dynamic<>>;

	// Dynamic bidirectional graph
	template<typename VertexProperty = void, typename EdgeProperty = void>
	using BidirectionalGraph = Graph<VertexProperty, EdgeProperty, Dynamic<Directionality::BIDIRECTIONAL>>;

	// Dynamic undirected graph
	template<typename VertexProperty = void, typename EdgeProperty = void>
	using UndirectedGraph = Graph<VertexProperty, EdgeProperty, Dynamic<Directionality::UNDIRECTED>>;

	// Fixed-size directed graph
	template<typename VertexProperty, typename EdgeProperty, std::size_t MaxV, std::size_t MaxE>
	using FixedGraph = Graph<VertexProperty, EdgeProperty, Fixed<MaxV, MaxE>>;

	// Fixed-size bidirectional graph
	template<typename VertexProperty, typename EdgeProperty, std::size_t MaxV, std::size_t MaxE>
	using BidirectionalFixedGraph =
	Graph<VertexProperty, EdgeProperty, Fixed<MaxV, MaxE, Directionality::BIDIRECTIONAL>>;

	// Fixed-size undirected graph
	template<typename VertexProperty, typename EdgeProperty, std::size_t MaxV, std::size_t MaxE>
	using UndirectedFixedGraph = Graph<VertexProperty, EdgeProperty, Fixed<MaxV, MaxE, Directionality::UNDIRECTED>>;
}
