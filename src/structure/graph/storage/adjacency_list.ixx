export module synodic.library:structure.graph.storage.adjacency_list;

import :structure.graph.concepts;
import :structure.graph.storage_traits;

import std;

export namespace synodic::graph::storage
{
	/**
	 * @brief Dynamic adjacency list storage supporting runtime vertex/edge add/remove.
	 *        Ideal for scheduler graphs that host compile-time subgraphs and need runtime modification.
	 *
	 *        Memory layout:
	 *        - Vertex properties stored contiguously in a vector (O(1) access by descriptor)
	 *        - Each vertex has a separate edge list (out-edges)
	 *        - Optional: in-edges stored for bidirectional traversal
	 *
	 *        Performance characteristics:
	 *        - O(1) vertex property access
	 *        - O(1) add vertex (amortized)
	 *        - O(1) add edge (amortized)
	 *        - O(degree) remove edge
	 *        - O(E) remove vertex (must update edge references)
	 *
	 * @tparam VertexProperty Type stored per vertex
	 * @tparam EdgeProperty Type stored per edge
	 * @tparam VertexContainer Container template for vertex storage
	 * @tparam EdgeContainer Container template for per-vertex edge lists
	 * @tparam IndexType Unsigned integer type for descriptors
	 * @tparam Bidirectional If true, maintain in-edge lists for O(1) predecessor access
	 */
	template<
		typename VertexProperty,
		typename EdgeProperty,
		template<typename...> typename VertexContainer = std::vector,
		template<typename...> typename EdgeContainer   = std::vector,
		std::unsigned_integral IndexType			   = std::size_t,
		bool Bidirectional							   = false>
	class AdjacencyListStorage
	{
	public:
		// ========================================================================
		// Type Definitions
		// ========================================================================

		using vertex_descriptor	 = IndexType;
		using vertices_size_type = IndexType;
		using edges_size_type	 = IndexType;
		using degree_size_type	 = IndexType;

		// Edge descriptor: (source vertex, index within source's edge list)
		// This provides O(1) access to edge properties
		struct edge_descriptor
		{
			vertex_descriptor source;
			IndexType edge_index;

			constexpr bool operator==(const edge_descriptor&) const noexcept  = default;
			constexpr auto operator<=>(const edge_descriptor&) const noexcept = default;
		};

		using vertex_property_type = VertexProperty;
		using edge_property_type   = EdgeProperty;

		// Compile-time properties
		static constexpr bool is_directed					= true;
		static constexpr bool allows_parallel_edges			= true;
		static constexpr bool is_constexpr_capable			= false;
		static constexpr bool has_stable_vertex_descriptors = false;  // Invalidated on remove
		static constexpr bool has_stable_edge_descriptors	= false;
		static constexpr bool is_bidirectional				= Bidirectional;

		// Rebind to different property types
		template<typename VP, typename EP>
		using rebind = AdjacencyListStorage<VP, EP, VertexContainer, EdgeContainer, IndexType, Bidirectional>;

	private:
		// Edge storage: target vertex + optional property
		struct EdgeData
		{
			vertex_descriptor target;
			[[no_unique_address]] std::conditional_t<std::same_as<EdgeProperty, void>, std::monostate, EdgeProperty>
				property;
		};

		// Per-vertex data: optional property + out-edge list + optional in-edge list
		struct VertexData
		{
			[[no_unique_address]] std::conditional_t<std::same_as<VertexProperty, void>, std::monostate, VertexProperty>
				property;

			EdgeContainer<EdgeData> out_edges;

			// In-edges stored as (source_vertex, edge_index_in_source) pairs
			[[no_unique_address]] std::conditional_t<Bidirectional, EdgeContainer<edge_descriptor>, std::monostate>
				in_edges;
		};

		VertexContainer<VertexData> vertices_;
		edges_size_type total_edges_ {0};

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		AdjacencyListStorage() = default;

		explicit AdjacencyListStorage(vertices_size_type initial_capacity)
		{
			vertices_.reserve(initial_capacity);
		}

		// ========================================================================
		// Vertex Operations
		// ========================================================================

		[[nodiscard]] auto num_vertices() const noexcept -> vertices_size_type
		{
			return static_cast<vertices_size_type>(vertices_.size());
		}

		[[nodiscard]] auto vertices() const noexcept
		{
			return std::views::iota(vertex_descriptor {0}, num_vertices());
		}

		auto add_vertex() -> vertex_descriptor
		{
			auto v = static_cast<vertex_descriptor>(vertices_.size());
			vertices_.emplace_back();
			return v;
		}

		template<typename... Args>
			requires(not std::same_as<VertexProperty, void>) and std::constructible_from<VertexProperty, Args...>
		auto add_vertex(Args&&... args) -> vertex_descriptor
		{
			auto v		  = static_cast<vertex_descriptor>(vertices_.size());
			auto& data	  = vertices_.emplace_back();
			data.property = VertexProperty(std::forward<Args>(args)...);
			return v;
		}

		void remove_vertex(vertex_descriptor v)
		{
			// Remove all edges incident to this vertex
			// First, remove out-edges from this vertex
			total_edges_ -= static_cast<edges_size_type>(vertices_[v].out_edges.size());
			vertices_[v].out_edges.clear();

			// Remove all edges targeting this vertex from other vertices
			for (auto& vertex_data: vertices_)
			{
				std::erase_if(
					vertex_data.out_edges,
					[v, this](const EdgeData& e)
					{
						if (e.target == v)
						{
							--total_edges_;
							return true;
						}
						return false;
					});
			}

			// Swap with last and pop (stable_vertex_descriptors = false)
			if (v != vertices_.size() - 1)
			{
				auto last = static_cast<vertex_descriptor>(vertices_.size() - 1);

				// Update all edges pointing to 'last' to point to 'v'
				for (auto& vertex_data: vertices_)
				{
					for (auto& edge: vertex_data.out_edges)
					{
						if (edge.target == last)
						{
							edge.target = v;
						}
					}
				}

				vertices_[v] = std::move(vertices_[last]);
			}
			vertices_.pop_back();
		}

		void clear_vertices()
		{
			vertices_.clear();
			total_edges_ = 0;
		}

		// ========================================================================
		// Edge Operations
		// ========================================================================

		[[nodiscard]] auto num_edges() const noexcept -> edges_size_type
		{
			return total_edges_;
		}

		auto add_edge(vertex_descriptor source, vertex_descriptor target) -> std::pair<edge_descriptor, bool>
		{
			auto edge_index = static_cast<IndexType>(vertices_[source].out_edges.size());
			vertices_[source].out_edges.push_back(EdgeData {target, {}});

			if constexpr (Bidirectional)
			{
				vertices_[target].in_edges.push_back(edge_descriptor {source, edge_index});
			}

			++total_edges_;
			return {
				edge_descriptor {source, edge_index},
				true
			};
		}

		template<typename... Args>
			requires(not std::same_as<EdgeProperty, void>) and std::constructible_from<EdgeProperty, Args...>
		auto add_edge(vertex_descriptor source, vertex_descriptor target, Args&&... args)
			-> std::pair<edge_descriptor, bool>
		{
			auto edge_index = static_cast<IndexType>(vertices_[source].out_edges.size());
			vertices_[source].out_edges.push_back(EdgeData {target, EdgeProperty(std::forward<Args>(args)...)});

			if constexpr (Bidirectional)
			{
				vertices_[target].in_edges.push_back(edge_descriptor {source, edge_index});
			}

			++total_edges_;
			return {
				edge_descriptor {source, edge_index},
				true
			};
		}

		void remove_edge(vertex_descriptor source, vertex_descriptor target)
		{
			auto& out = vertices_[source].out_edges;
			auto it	  = std::ranges::find_if(
				  out,
				  [target](const EdgeData& e)
				  {
					  return e.target == target;
				  });

			if (it != out.end())
			{
				auto edge_index = static_cast<IndexType>(std::distance(out.begin(), it));

				if constexpr (Bidirectional)
				{
					// Remove from target's in-edge list
					auto& in = vertices_[target].in_edges;
					std::erase_if(
						in,
						[source, edge_index](const edge_descriptor& e)
						{
							return e.source == source && e.edge_index == edge_index;
						});
				}

				out.erase(it);
				--total_edges_;
			}
		}

		void remove_edge(edge_descriptor e)
		{
			auto& out = vertices_[e.source].out_edges;
			if (e.edge_index < out.size())
			{
				auto target = out[e.edge_index].target;

				if constexpr (Bidirectional)
				{
					auto& in = vertices_[target].in_edges;
					std::erase_if(
						in,
						[e](const edge_descriptor& ie)
						{
							return ie.source == e.source && ie.edge_index == e.edge_index;
						});
				}

				out.erase(out.begin() + e.edge_index);
				--total_edges_;
			}
		}

		void clear_edges()
		{
			for (auto& v: vertices_)
			{
				v.out_edges.clear();
				if constexpr (Bidirectional)
				{
					v.in_edges.clear();
				}
			}
			total_edges_ = 0;
		}

		// ========================================================================
		// Edge Enumeration
		// ========================================================================

		class EdgeIterator
		{
			const AdjacencyListStorage* storage_;
			vertex_descriptor current_vertex_;
			IndexType current_edge_;

			void AdvanceToValid()
			{
				while (current_vertex_ < storage_->num_vertices())
				{
					if (current_edge_ < storage_->vertices_[current_vertex_].out_edges.size())
					{
						return;
					}
					++current_vertex_;
					current_edge_ = 0;
				}
			}

		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type		= edge_descriptor;
			using difference_type	= std::ptrdiff_t;
			using pointer			= const edge_descriptor*;
			using reference			= edge_descriptor;

			EdgeIterator() = default;

			EdgeIterator(const AdjacencyListStorage* s, vertex_descriptor v, IndexType e) :
				storage_(s),
				current_vertex_(v),
				current_edge_(e)
			{
				AdvanceToValid();
			}

			auto operator*() const -> edge_descriptor
			{
				return {current_vertex_, current_edge_};
			}

			auto operator++() -> EdgeIterator&
			{
				++current_edge_;
				AdvanceToValid();
				return *this;
			}

			auto operator++(int) -> EdgeIterator
			{
				auto tmp = *this;
				++(*this);
				return tmp;
			}

			bool operator==(const EdgeIterator& other) const
			{
				return current_vertex_ == other.current_vertex_ && current_edge_ == other.current_edge_;
			}
		};

		[[nodiscard]] auto edges() const noexcept
		{
			return std::ranges::subrange(
				EdgeIterator {this, vertex_descriptor {0}, IndexType {0}},
				EdgeIterator {this, num_vertices(), IndexType {0}});
		}

		// ========================================================================
		// Incidence (Out-Edge Traversal)
		// ========================================================================

		[[nodiscard]] auto out_degree(vertex_descriptor v) const noexcept -> degree_size_type
		{
			return static_cast<degree_size_type>(vertices_[v].out_edges.size());
		}

		[[nodiscard]] auto out_edges(vertex_descriptor v) const noexcept
		{
			return std::views::iota(IndexType {0}, static_cast<IndexType>(vertices_[v].out_edges.size())) |
				   std::views::transform(
					   [v](IndexType i)
					   {
						   return edge_descriptor {v, i};
					   });
		}

		[[nodiscard]] auto source(edge_descriptor e) const noexcept -> vertex_descriptor
		{
			return e.source;
		}

		[[nodiscard]] auto target(edge_descriptor e) const noexcept -> vertex_descriptor
		{
			return vertices_[e.source].out_edges[e.edge_index].target;
		}

		// ========================================================================
		// Bidirectional (In-Edge Traversal)
		// ========================================================================

		[[nodiscard]] auto in_degree(vertex_descriptor v) const noexcept -> degree_size_type
			requires Bidirectional
		{
			return static_cast<degree_size_type>(vertices_[v].in_edges.size());
		}

		[[nodiscard]] auto in_edges(vertex_descriptor v) const noexcept
			requires Bidirectional
		{
			return std::span {vertices_[v].in_edges};
		}

		// ========================================================================
		// Adjacency (Direct Neighbor Access)
		// ========================================================================

		[[nodiscard]] auto adjacent_vertices(vertex_descriptor v) const noexcept
		{
			return vertices_[v].out_edges | std::views::transform(
												[](const EdgeData& e)
												{
													return e.target;
												});
		}

		// ========================================================================
		// Property Access - O(1)
		// ========================================================================

		template<typename Self>
		[[nodiscard]] auto&& vertex_property(this Self&& self, vertex_descriptor v)
			requires(not std::same_as<vertex_property_type, void>)
		{
			return std::forward<Self>(self).vertices_[v].property;
		}

		template<typename Self>
		[[nodiscard]] auto&& edge_property(this Self&& self, edge_descriptor e)
			requires(not std::same_as<edge_property_type, void>)
		{
			return std::forward<Self>(self).vertices_[e.source].out_edges[e.edge_index].property;
		}

		[[nodiscard]] decltype(auto) operator[](vertex_descriptor v)
			requires(not std::same_as<vertex_property_type, void>)
		{
			return vertex_property(v);
		}

		[[nodiscard]] decltype(auto) operator[](vertex_descriptor v) const
			requires(not std::same_as<vertex_property_type, void>)
		{
			return vertex_property(v);
		}

		[[nodiscard]] decltype(auto) operator[](edge_descriptor e)
			requires(not std::same_as<edge_property_type, void>)
		{
			return edge_property(e);
		}

		[[nodiscard]] decltype(auto) operator[](edge_descriptor e) const
			requires(not std::same_as<edge_property_type, void>)
		{
			return edge_property(e);
		}

		// ========================================================================
		// Subgraph Composition Support
		// ========================================================================

		/**
		 * @brief Merge a compile-time subgraph into this graph, returning handle mapping.
		 * @param subgraph The subgraph to merge
		 * @return Vector mapping subgraph vertex descriptors to this graph's descriptors
		 */
		template<composable_storage Subgraph>
		auto compose(const Subgraph& subgraph) -> VertexContainer<vertex_descriptor>
		{
			using SubgraphTraits = storage_traits<Subgraph>;

			VertexContainer<vertex_descriptor> handle_map;
			handle_map.reserve(subgraph.num_vertices());

			// Add all vertices from subgraph
			for (auto sv: subgraph.vertices())
			{
				vertex_descriptor new_v;
				if constexpr (SubgraphTraits::supports_vertex_properties and not std::same_as<VertexProperty, void>)
				{
					new_v = add_vertex(subgraph[sv]);
				}
				else
				{
					new_v = add_vertex();
				}
				handle_map.push_back(new_v);
			}

			// Add all edges from subgraph with remapped handles
			for (auto se: subgraph.edges())
			{
				auto src = handle_map[subgraph.source(se)];
				auto tgt = handle_map[subgraph.target(se)];

				if constexpr (SubgraphTraits::supports_edge_properties and not std::same_as<EdgeProperty, void>)
				{
					add_edge(src, tgt, subgraph[se]);
				}
				else
				{
					add_edge(src, tgt);
				}
			}

			return handle_map;
		}
	};

	// ============================================================================
	// Convenience Aliases
	// ============================================================================

	template<std::unsigned_integral IndexType = std::size_t>
	using AdjacencyListTopology = AdjacencyListStorage<void, void, std::vector, std::vector, IndexType, false>;

	template<typename VertexProperty, std::unsigned_integral IndexType = std::size_t>
	using AdjacencyListVertexStorage =
		AdjacencyListStorage<VertexProperty, void, std::vector, std::vector, IndexType, false>;

	template<typename EdgeProperty, std::unsigned_integral IndexType = std::size_t>
	using AdjacencyListEdgeStorage =
		AdjacencyListStorage<void, EdgeProperty, std::vector, std::vector, IndexType, false>;

	// Bidirectional variants
	template<std::unsigned_integral IndexType = std::size_t>
	using BidirectionalListTopology = AdjacencyListStorage<void, void, std::vector, std::vector, IndexType, true>;

	template<typename VertexProperty, typename EdgeProperty, std::unsigned_integral IndexType = std::size_t>
	using BidirectionalListStorage =
		AdjacencyListStorage<VertexProperty, EdgeProperty, std::vector, std::vector, IndexType, true>;
}
