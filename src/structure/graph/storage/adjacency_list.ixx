export module synodic.periapsis:structure.graph.storage.adjacency_list;

import :structure.graph.concepts;
import :structure.graph.storage_traits;

import std;

export namespace peri::graph::storage
{
	/**
	 * @brief Dynamic adjacency list storage supporting runtime vertex/edge add/remove.
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
		template<typename...> typename EdgeContainer = std::vector,
		std::unsigned_integral IndexType = std::size_t,
		bool Bidirectional = false>
	class AdjacencyListStorage
	{
	public:
		// ========================================================================
		// Type Definitions
		// ========================================================================

		using VertexDescriptor = IndexType;
		using VerticesSizeType = IndexType;
		using EdgesSizeType    = IndexType;
		using DegreeSizeType   = IndexType;

		// Edge descriptor provides O(1) access to edge properties
		struct EdgeDescriptor
		{
			VertexDescriptor source;
			IndexType edgeIndex;

			constexpr bool operator==(const EdgeDescriptor&) const noexcept  = default;
			constexpr auto operator<=>(const EdgeDescriptor&) const noexcept = default;
		};

		using VertexPropertyType = VertexProperty;
		using EdgePropertyType   = EdgeProperty;

		// Compile-time properties
		static constexpr bool IS_DIRECTED                   = true;
		static constexpr bool ALLOWS_PARALLEL_EDGES         = true;
		static constexpr bool HAS_STABLE_VERTEX_DESCRIPTORS = false; // Invalidated on remove
		static constexpr bool HAS_STABLE_EDGE_DESCRIPTORS   = false;
		static constexpr bool IS_BIDIRECTIONAL              = Bidirectional;

		// Rebind to different property types
		template<typename VP, typename EP>
		using Rebind = AdjacencyListStorage<VP, EP, VertexContainer, EdgeContainer, IndexType, Bidirectional>;

	private:
		// Edge storage: target vertex + optional property
		struct EdgeData
		{
			VertexDescriptor target;
			[[no_unique_address]] std::
			conditional_t<std::same_as<EdgePropertyType, void>, std::monostate, EdgePropertyType> property;
		};

		// Per-vertex data: optional property + out-edge list + optional in-edge list
		struct VertexData
		{
			[[no_unique_address]] std::
			conditional_t<std::same_as<VertexPropertyType, void>, std::monostate, VertexPropertyType> property;

			EdgeContainer<EdgeData> outEdges;

			// In-edges stored as source vertex and edge index pairs
			[[no_unique_address]] std::conditional_t<Bidirectional, EdgeContainer<EdgeDescriptor>, std::monostate>
			inEdges;
		};

		VertexContainer<VertexData> vertices_;
		EdgesSizeType totalEdges_{0};

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		AdjacencyListStorage() = default;

		explicit AdjacencyListStorage(VerticesSizeType initialCapacity)
		{
			vertices_.reserve(initialCapacity);
		}

		// ========================================================================
		// Vertex Operations
		// ========================================================================

		[[nodiscard]] auto NumVertices() const noexcept -> VerticesSizeType
		{
			return static_cast<VerticesSizeType>(vertices_.size());
		}

		[[nodiscard]] auto Vertices() const noexcept
		{
			return std::views::iota(VertexDescriptor{0}, NumVertices());
		}

		auto AddVertex() -> VertexDescriptor
		{
			auto v = static_cast<VertexDescriptor>(vertices_.size());
			vertices_.emplace_back();
			return v;
		}

		template<typename... Args>
			requires(not std::same_as<VertexPropertyType, void>) and
			        std::constructible_from<VertexPropertyType, Args...>
		auto AddVertex(Args&&... args) -> VertexDescriptor
		{
			auto v        = static_cast<VertexDescriptor>(vertices_.size());
			auto& data    = vertices_.emplace_back();
			data.property = VertexPropertyType(std::forward<Args>(args)...);
			return v;
		}

		void RemoveVertex(VertexDescriptor v)
		{
			// Remove all edges incident to this vertex
			// First, remove out-edges from this vertex
			totalEdges_ -= static_cast<EdgesSizeType>(vertices_[v].outEdges.size());
			vertices_[v].outEdges.clear();

			// Remove all edges targeting this vertex from other vertices
			for (auto& vertexData: vertices_)
			{
				std::erase_if(
					vertexData.outEdges,
					[v, this](const EdgeData& e)
					{
						if (e.target == v)
						{
							--totalEdges_;
							return true;
						}
						return false;
					});
			}

			// Swap with last and pop since stable vertex descriptors is false
			if (v != vertices_.size() - 1)
			{
				auto last = static_cast<VertexDescriptor>(vertices_.size() - 1);

				// Update all edges pointing to 'last' to point to 'v'
				for (auto& vertexData: vertices_)
				{
					for (auto& edge: vertexData.outEdges)
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

		void ClearVertices()
		{
			vertices_.clear();
			totalEdges_ = 0;
		}

		// ========================================================================
		// Edge Operations
		// ========================================================================

		[[nodiscard]] auto NumEdges() const noexcept -> EdgesSizeType
		{
			return totalEdges_;
		}

		auto AddEdge(VertexDescriptor source, VertexDescriptor target) -> std::pair<EdgeDescriptor, bool>
		{
			auto edgeIndex = static_cast<IndexType>(vertices_[source].outEdges.size());
			vertices_[source].outEdges.push_back(EdgeData{target, {}});

			if constexpr (Bidirectional)
			{
				vertices_[target].inEdges.push_back(EdgeDescriptor{source, edgeIndex});
			}

			++totalEdges_;
			return {
				EdgeDescriptor{source, edgeIndex},
				true
			};
		}

		template<typename... Args>
			requires(not std::same_as<EdgePropertyType, void>) and std::constructible_from<EdgePropertyType, Args...>
		auto AddEdge(
			VertexDescriptor source,
			VertexDescriptor target,
			Args&&... args)
			-> std::pair<EdgeDescriptor, bool>
		{
			auto edgeIndex = static_cast<IndexType>(vertices_[source].outEdges.size());
			vertices_[source].outEdges.push_back(EdgeData{target, EdgePropertyType(std::forward<Args>(args)...)});

			if constexpr (Bidirectional)
			{
				vertices_[target].inEdges.push_back(EdgeDescriptor{source, edgeIndex});
			}

			++totalEdges_;
			return {
				EdgeDescriptor{source, edgeIndex},
				true
			};
		}

		void RemoveEdge(VertexDescriptor source, VertexDescriptor target)
		{
			auto& out = vertices_[source].outEdges;
			auto it   = std::ranges::find_if(
				out,
				[target](const EdgeData& e)
				{
					return e.target == target;
				});

			if (it != out.end())
			{
				auto edgeIndex = static_cast<IndexType>(std::distance(out.begin(), it));

				if constexpr (Bidirectional)
				{
					// Remove from target's in-edge list
					auto& in = vertices_[target].inEdges;
					std::erase_if(
						in,
						[source, edgeIndex](const EdgeDescriptor& e)
						{
							return e.source == source && e.edgeIndex == edgeIndex;
						});
				}

				out.erase(it);
				--totalEdges_;
			}
		}

		void RemoveEdge(EdgeDescriptor e)
		{
			auto& out = vertices_[e.source].outEdges;
			if (e.edgeIndex < out.size())
			{
				auto target = out[e.edgeIndex].target;

				if constexpr (Bidirectional)
				{
					auto& in = vertices_[target].inEdges;
					std::erase_if(
						in,
						[e](const EdgeDescriptor& ie)
						{
							return ie.source == e.source && ie.edgeIndex == e.edgeIndex;
						});
				}

				out.erase(out.begin() + e.edgeIndex);
				--totalEdges_;
			}
		}

		void ClearEdges()
		{
			for (auto& v: vertices_)
			{
				v.outEdges.clear();
				if constexpr (Bidirectional)
				{
					v.inEdges.clear();
				}
			}
			totalEdges_ = 0;
		}

		// ========================================================================
		// Edge Enumeration
		// ========================================================================

		class EdgeIterator
		{
			const AdjacencyListStorage* storage_;
			VertexDescriptor currentVertex_;
			IndexType currentEdge_;

			void AdvanceToValid()
			{
				while (currentVertex_ < storage_->NumVertices())
				{
					if (currentEdge_ < storage_->vertices_[currentVertex_].outEdges.size())
					{
						return;
					}
					++currentVertex_;
					currentEdge_ = 0;
				}
			}

		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type        = EdgeDescriptor;
			using difference_type   = std::ptrdiff_t;
			using pointer           = const EdgeDescriptor*;
			using reference         = EdgeDescriptor;

			EdgeIterator() = default;

			EdgeIterator(const AdjacencyListStorage* s, VertexDescriptor v, IndexType e) :
				storage_(s),
				currentVertex_(v),
				currentEdge_(e)
			{
				AdvanceToValid();
			}

			auto operator*() const -> EdgeDescriptor
			{
				return {currentVertex_, currentEdge_};
			}

			auto operator++() -> EdgeIterator&
			{
				++currentEdge_;
				AdvanceToValid();
				return *this;
			}

			auto operator++(int) -> EdgeIterator
			{
				auto tmp = *this;
				++*this;
				return tmp;
			}

			bool operator==(const EdgeIterator& other) const
			{
				return currentVertex_ == other.currentVertex_ && currentEdge_ == other.currentEdge_;
			}
		};

		[[nodiscard]] auto Edges() const noexcept
		{
			return std::ranges::subrange(
				EdgeIterator{this, VertexDescriptor{0}, IndexType{0}},
				EdgeIterator{this, NumVertices(), IndexType{0}});
		}

		// ========================================================================
		// Incidence: Out-Edge Traversal
		// ========================================================================

		[[nodiscard]] auto OutDegree(VertexDescriptor v) const noexcept -> DegreeSizeType
		{
			return static_cast<DegreeSizeType>(vertices_[v].outEdges.size());
		}

		[[nodiscard]] auto OutEdges(VertexDescriptor v) const noexcept
		{
			return std::views::iota(IndexType{0}, static_cast<IndexType>(vertices_[v].outEdges.size())) |
			       std::views::transform(
				       [v](IndexType i)
				       {
					       return EdgeDescriptor{v, i};
				       });
		}

		[[nodiscard]] auto Source(EdgeDescriptor e) const noexcept -> VertexDescriptor
		{
			return e.source;
		}

		[[nodiscard]] auto Target(EdgeDescriptor e) const noexcept -> VertexDescriptor
		{
			return vertices_[e.source].outEdges[e.edgeIndex].target;
		}

		// ========================================================================
		// Bidirectional: In-Edge Traversal
		// ========================================================================

		[[nodiscard]] auto InDegree(VertexDescriptor v) const noexcept -> DegreeSizeType requires Bidirectional
		{
			return static_cast<DegreeSizeType>(vertices_[v].inEdges.size());
		}

		[[nodiscard]] auto InEdges(VertexDescriptor v) const noexcept
			requires Bidirectional
		{
			return std::span{vertices_[v].inEdges};
		}

		// ========================================================================
		// Adjacency: Direct Neighbor Access
		// ========================================================================

		[[nodiscard]] auto AdjacentVertices(VertexDescriptor v) const noexcept
		{
			return vertices_[v].outEdges | std::views::transform(
				       [](const EdgeData& e)
				       {
					       return e.target;
				       });
		}

		// ========================================================================
		// Property Access: O-1
		// ========================================================================

		template<typename Self>
		[[nodiscard]] auto&& VertexProperty(this Self&& self, VertexDescriptor v)
			requires(not std::same_as<VertexPropertyType, void>)
		{
			return std::forward<Self>(self).vertices_[v].property;
		}

		template<typename Self>
		[[nodiscard]] auto&& EdgeProperty(this Self&& self, EdgeDescriptor e)
			requires(not std::same_as<EdgePropertyType, void>)
		{
			return std::forward<Self>(self).vertices_[e.source].outEdges[e.edgeIndex].property;
		}

		[[nodiscard]] decltype(auto) operator[](VertexDescriptor v)
			requires(not std::same_as<VertexPropertyType, void>)
		{
			return VertexProperty(v);
		}

		[[nodiscard]] decltype(auto) operator[](VertexDescriptor v) const
			requires(not std::same_as<VertexPropertyType, void>)
		{
			return VertexProperty(v);
		}

		[[nodiscard]] decltype(auto) operator[](EdgeDescriptor e)
			requires(not std::same_as<EdgePropertyType, void>)
		{
			return EdgeProperty(e);
		}

		[[nodiscard]] decltype(auto) operator[](EdgeDescriptor e) const
			requires(not std::same_as<EdgePropertyType, void>)
		{
			return EdgeProperty(e);
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
		auto Compose(const Subgraph& subgraph) -> VertexContainer<VertexDescriptor>
		{
			using SubgraphTraits = StorageTraits<Subgraph>;

			VertexContainer<VertexDescriptor> handleMap;
			handleMap.reserve(subgraph.NumVertices());

			// Add all vertices from subgraph
			for (auto sv: subgraph.Vertices())
			{
				VertexDescriptor newV;
				if constexpr (SubgraphTraits::SUPPORTS_VERTEX_PROPERTIES and not std::same_as<VertexPropertyType, void>)
				{
					newV = AddVertex(subgraph[sv]);
				}
				else
				{
					newV = AddVertex();
				}
				handleMap.push_back(newV);
			}

			// Add all edges from subgraph with remapped handles
			for (auto se: subgraph.Edges())
			{
				auto src = handleMap[subgraph.Source(se)];
				auto tgt = handleMap[subgraph.Target(se)];

				if constexpr (SubgraphTraits::SUPPORTS_EDGE_PROPERTIES and not std::same_as<EdgePropertyType, void>)
				{
					AddEdge(src, tgt, subgraph[se]);
				}
				else
				{
					AddEdge(src, tgt);
				}
			}

			return handleMap;
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
