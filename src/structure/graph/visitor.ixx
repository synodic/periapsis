export module synodic.periapsis:structure.graph.visitor;

import :structure.graph.concepts;

import std;

export namespace peri::graph
{
	/**
	 * @brief Result of attempting to advance a visitor
	 */
	enum class AdvanceResult
	{
		MOVED, // Successfully moved to a new vertex
		NO_EDGE, // No edge matched the selection criteria
		BLOCKED, // Edge found but blocked by guard/predicate
		INVALID_STATE // Walker has no current position
	};

	// ============================================================================
	// Visitor Concept - Common interface for graph traversal
	// ============================================================================

	/**
	 * @brief Concept for any type that can visit/traverse a graph structure.
	 *        Both Visitor (graph) and Runner (FSM) satisfy this concept.
	 */
	template<typename V>
	concept visitor = requires(V& v, const V& cv)
	{
		typename V::VertexDescriptor;
		typename V::EdgeDescriptor;

		// Position query
		{ cv.position() } -> std::same_as<std::optional<typename V::VertexDescriptor>>;
		{ cv.is_positioned() } -> std::convertible_to<bool>;

		// Position control
		{ v.move_to(std::declval<typename V::VertexDescriptor>()) };
		{ v.reset() };
	};

	/**
	 * @brief Visitor that can advance without arguments (e.g., first-edge traversal)
	 */
	template<typename V>
	concept simple_visitor = visitor<V> and requires(V& v)
	{
		{ v.advance() } -> std::same_as<AdvanceResult>;
	};

	/**
	 * @brief Visitor that advances based on an event/key
	 */
	template<typename V, typename Event>
	concept event_visitor = visitor<V> and requires(V& v, const V& cv, const Event& e)
	{
		{ v.advance(e) } -> std::same_as<AdvanceResult>;
		{ cv.can_advance(e) } -> std::convertible_to<bool>;
	};

	// ============================================================================
	// Edge Selector Concepts
	// ============================================================================

	/**
	 * @brief An edge selector determines which edge to take from a vertex
	 */
	template<typename S, typename Graph, typename... Args>
	concept edge_selector =
		requires(const S& selector, const Graph& graph, typename Graph::VertexDescriptor v, Args&&... args)
		{
			{
				selector.select(graph, v, std::forward<Args>(args)...)
			} -> std::same_as<std::optional<typename Graph::EdgeDescriptor>>;
		};

	// ============================================================================
	// Built-in Selectors
	// ============================================================================

	/**
	 * @brief Selects the first edge (if any) from a vertex
	 *        Useful for simple traversal or as a base for other selectors
	 */
	struct FirstEdgeSelector
	{
		template<typename Graph>
		[[nodiscard]] constexpr auto select(
			const Graph& graph,
			Graph::VertexDescriptor v) const noexcept
			-> std::optional<typename Graph::EdgeDescriptor>
		{
			auto edges = graph.OutEdges(v);
			auto it    = std::ranges::begin(edges);
			if (it != std::ranges::end(edges))
			{
				return *it;
			}
			return std::nullopt;
		}
	};

	/**
	 * @brief Selects the first in-edge (if any) to a vertex.
	 *        Enables reverse/backward traversal for bidirectional graphs.
	 *        Only available for graphs with bidirectional storage.
	 */
	struct BackwardEdgeSelector
	{
		template<typename Graph>
			requires bidirectional_storage<typename Graph::StorageType>
		[[nodiscard]] constexpr auto select(
			const Graph& graph,
			Graph::VertexDescriptor v) const noexcept
			-> std::optional<typename Graph::EdgeDescriptor>
		{
			auto edges = graph.InEdges(v);
			auto it    = std::ranges::begin(edges);
			if (it != std::ranges::end(edges))
			{
				return *it;
			}
			return std::nullopt;
		}
	};

	/**
	 * @brief Selects an edge based on a predicate
	 */
	template<typename Predicate>
	struct PredicateSelector
	{
		Predicate predicate;

		template<typename Graph>
		[[nodiscard]] constexpr auto select(
			const Graph& graph,
			Graph::VertexDescriptor v) const
			-> std::optional<typename Graph::EdgeDescriptor>
		{
			for (auto e: graph.OutEdges(v))
			{
				if (predicate(graph, e))
				{
					return e;
				}
			}
			return std::nullopt;
		}
	};

	template<typename P>
	PredicateSelector(P) -> PredicateSelector<P>;

	/**
	 * @brief Selects an in-edge based on a predicate (backward traversal).
	 *        Only available for graphs with bidirectional storage.
	 */
	template<typename Predicate>
	struct BackwardPredicateSelector
	{
		Predicate predicate;

		template<typename Graph>
			requires bidirectional_storage<typename Graph::StorageType>
		[[nodiscard]] constexpr auto select(
			const Graph& graph,
			Graph::VertexDescriptor v) const
			-> std::optional<typename Graph::EdgeDescriptor>
		{
			for (auto e: graph.InEdges(v))
			{
				if (predicate(graph, e))
				{
					return e;
				}
			}
			return std::nullopt;
		}
	};

	template<typename P>
	BackwardPredicateSelector(P) -> BackwardPredicateSelector<P>;

	/**
	 * @brief Selects an edge by matching an event/key against edge properties
	 *        Core selector for FSM-style traversal
	 */
	template<typename Event, typename EventExtractor>
	struct EventSelector
	{
		EventExtractor extractor;

		template<typename Graph>
		[[nodiscard]] constexpr auto
			select(
				const Graph& graph,
				Graph::VertexDescriptor v,
				const Event& event) const
				-> std::optional<typename Graph::EdgeDescriptor>
		{
			for (auto e: graph.OutEdges(v))
			{
				if (extractor(graph, e) == event)
				{
					return e;
				}
			}
			return std::nullopt;
		}
	};

	template<typename E, typename Ex>
	EventSelector(Ex) -> EventSelector<E, Ex>;

	/**
	 * @brief Default event extractor that assumes edge property IS the event
	 */
	struct DefaultEventExtractor
	{
		template<typename Graph>
		[[nodiscard]] constexpr auto operator()(
			const Graph& graph,
			Graph::EdgeDescriptor e) const
			-> decltype(auto)
		{
			return graph[e];
		}
	};

	/**
	 * @brief Convenience alias for event selector with default extractor
	 */
	template<typename Event>
	using SimpleEventSelector = EventSelector<Event, DefaultEventExtractor>;

	// ============================================================================
	// Visitor Class
	// ============================================================================

	/**
	 * @brief A visitor traverses a graph structure, maintaining current position.
	 *        The graph can be const - the visitor only reads structure and tracks position.
	 *
	 *        Selector determines how edges are chosen during advance().
	 *
	 * @tparam Graph The graph type to traverse (can be const-qualified)
	 * @tparam Selector Edge selection policy (default: FirstEdgeSelector)
	 */
	template<typename Graph, typename Selector = FirstEdgeSelector>
	class Visitor
	{
	public:
		using GraphType        = std::remove_cvref_t<Graph>;
		using VertexDescriptor = GraphType::VertexDescriptor;
		using EdgeDescriptor   = GraphType::EdgeDescriptor;
		using SelectorType     = Selector;

	private:
		// Pointer to graph (const or non-const based on Graph template param)
		std::conditional_t<std::is_const_v<std::remove_reference_t<Graph>>, const GraphType*, GraphType*> graph_{
			nullptr};

		std::optional<VertexDescriptor> position_{};
		[[no_unique_address]] Selector selector_{};

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		constexpr Visitor() = default;

		/**
		 * @brief Construct visitor on a graph (const or non-const)
		 */
		constexpr explicit Visitor(Graph& graph) :
			graph_(&graph)
		{
		}

		/**
		 * @brief Construct visitor on a graph with custom selector
		 */
		constexpr Visitor(Graph& graph, Selector selector) :
			graph_(&graph),
			selector_(std::move(selector))
		{
		}

		/**
		 * @brief Construct visitor at a specific starting position
		 */
		constexpr Visitor(Graph& graph, VertexDescriptor start) :
			graph_(&graph),
			position_(start)
		{
		}

		/**
		 * @brief Construct visitor at a specific position with custom selector
		 */
		constexpr Visitor(Graph& graph, VertexDescriptor start, Selector selector) :
			graph_(&graph),
			position_(start),
			selector_(std::move(selector))
		{
		}

		// ========================================================================
		// Graph Access
		// ========================================================================

		[[nodiscard]] constexpr auto graph() const noexcept -> const GraphType&
		{
			return *graph_;
		}

		[[nodiscard]] constexpr auto graph() noexcept -> Graph& requires(not std::is_const_v<std::remove_reference_t<
			Graph>>)
		{
			return *graph_;
		}

		[[nodiscard]] constexpr auto has_graph() const noexcept -> bool
		{
			return graph_ != nullptr;
		}

		// ========================================================================
		// Position Management
		// ========================================================================

		/**
		 * @brief Get current position (nullopt if not positioned)
		 */
		[[nodiscard]] constexpr auto position() const noexcept -> std::optional<VertexDescriptor>
		{
			return position_;
		}

		/**
		 * @brief Check if visitor is positioned on a vertex
		 */
		[[nodiscard]] constexpr auto is_positioned() const noexcept -> bool
		{
			return position_.has_value();
		}

		/**
		 * @brief Move to a specific vertex
		 */
		constexpr void move_to(VertexDescriptor v)
		{
			position_ = v;
		}

		/**
		 * @brief Clear position (visitor becomes unpositioned)
		 */
		constexpr void reset()
		{
			position_ = std::nullopt;
		}

		// ========================================================================
		// Traversal
		// ========================================================================

		/**
		 * @brief Advance using selector with no additional arguments
		 */
		[[nodiscard]] constexpr auto advance() -> AdvanceResult requires edge_selector<Selector, GraphType>
		{
			if (!position_)
			{
				return AdvanceResult::INVALID_STATE;
			}

			auto edge = selector_.select(*graph_, *position_);
			if (!edge)
			{
				return AdvanceResult::NO_EDGE;
			}

			position_ = graph_->Target(*edge);
			return AdvanceResult::MOVED;
		}

		/**
		 * @brief Advance using selector with additional arguments (e.g., event for EventSelector)
		 */
		template<typename... Args>
			requires edge_selector<Selector, GraphType, Args...>
		[[nodiscard]] constexpr auto advance(Args&&... args) -> AdvanceResult
		{
			if (!position_)
			{
				return AdvanceResult::INVALID_STATE;
			}

			auto edge = selector_.select(*graph_, *position_, std::forward<Args>(args)...);
			if (!edge)
			{
				return AdvanceResult::NO_EDGE;
			}

			position_ = graph_->Target(*edge);
			return AdvanceResult::MOVED;
		}

		/**
		 * @brief Advance with callback on success (receives edge that was taken)
		 */
		template<typename Callback, typename... Args>
			requires edge_selector<Selector, GraphType, Args...> and std::invocable<Callback, EdgeDescriptor>
		[[nodiscard]] constexpr auto advance_with(Callback&& callback, Args&&... args) -> AdvanceResult
		{
			if (!position_)
			{
				return AdvanceResult::INVALID_STATE;
			}

			auto edge = selector_.select(*graph_, *position_, std::forward<Args>(args)...);
			if (!edge)
			{
				return AdvanceResult::NO_EDGE;
			}

			position_ = graph_->Target(*edge);
			std::invoke(std::forward<Callback>(callback), *edge);
			return AdvanceResult::MOVED;
		}

		/**
		 * @brief Check if advance would succeed without actually moving
		 */
		template<typename... Args>
			requires edge_selector<Selector, GraphType, Args...>
		[[nodiscard]] constexpr auto can_advance(Args&&... args) const -> bool
		{
			if (!position_)
			{
				return false;
			}
			return selector_.select(*graph_, *position_, std::forward<Args>(args)...).has_value();
		}

		/**
		 * @brief Get the edge that would be taken (without moving)
		 */
		template<typename... Args>
			requires edge_selector<Selector, GraphType, Args...>
		[[nodiscard]] constexpr auto peek(Args&&... args) const -> std::optional<EdgeDescriptor>
		{
			if (!position_)
			{
				return std::nullopt;
			}
			return selector_.select(*graph_, *position_, std::forward<Args>(args)...);
		}

		// ========================================================================
		// Selector Access
		// ========================================================================

		[[nodiscard]] constexpr auto selector() const noexcept -> const Selector&
		{
			return selector_;
		}

		[[nodiscard]] constexpr auto selector() noexcept -> Selector&
		{
			return selector_;
		}
	};

	// ============================================================================
	// Deduction Guides
	// ============================================================================

	template<typename G>
	Visitor(G&) -> Visitor<G, FirstEdgeSelector>;

	template<typename G, typename S>
	Visitor(G&, S) -> Visitor<G, S>;

	template<typename G, typename V>
		requires std::same_as<V, typename std::remove_cvref_t<G>::VertexDescriptor>
	Visitor(G&, V) -> Visitor<G, FirstEdgeSelector>;

	template<typename G, typename V, typename S>
		requires std::same_as<V, typename std::remove_cvref_t<G>::VertexDescriptor>
	Visitor(G&, V, S) -> Visitor<G, S>;

	// ============================================================================
	// Factory Functions
	// ============================================================================

	/**
	 * @brief Create a visitor on a graph
	 */
	template<typename Graph>
	[[nodiscard]] constexpr auto make_visitor(Graph& graph)
	{
		return Visitor<Graph>{graph};
	}

	/**
	 * @brief Create a visitor with a custom selector
	 */
	template<typename Graph, typename Selector>
	[[nodiscard]] constexpr auto make_visitor(Graph& graph, Selector selector)
	{
		return Visitor<Graph, Selector>{graph, std::move(selector)};
	}

	/**
	 * @brief Create a visitor starting at a specific vertex
	 */
	template<typename Graph>
	[[nodiscard]] constexpr auto make_visitor(Graph& graph, typename std::remove_cvref_t<Graph>::VertexDescriptor start)
	{
		return Visitor<Graph>{graph, start};
	}

	/**
	 * @brief Create a backward visitor on a bidirectional graph
	 */
	template<typename Graph>
		requires bidirectional_storage<typename std::remove_cvref_t<Graph>::StorageType>
	[[nodiscard]] constexpr auto make_backward_visitor(Graph& graph)
	{
		return Visitor<Graph, BackwardEdgeSelector>{graph};
	}

	/**
	 * @brief Create a backward visitor starting at a specific vertex
	 */
	template<typename Graph>
		requires bidirectional_storage<typename std::remove_cvref_t<Graph>::StorageType>
	[[nodiscard]] constexpr auto
		make_backward_visitor(Graph& graph, typename std::remove_cvref_t<Graph>::VertexDescriptor start)
	{
		return Visitor<Graph, BackwardEdgeSelector>{graph, start};
	}

	// ============================================================================
	// Bidirectional Visitor
	// ============================================================================

	/**
	 * @brief Direction for bidirectional traversal
	 */
	enum class TraversalDirection
	{
		FORWARD, // Follow out-edges (source -> target)
		BACKWARD // Follow in-edges (target -> source)
	};

	/**
	 * @brief A visitor that can traverse in both directions on bidirectional graphs.
	 *        Maintains current position and can advance forward or backward.
	 *
	 * @tparam Graph The graph type to traverse (must have bidirectional storage)
	 * @tparam ForwardSelector Selector for forward (out-edge) traversal
	 * @tparam BackwardSelector Selector for backward (in-edge) traversal
	 */
	template<
		typename Graph,
		typename ForwardSelector = FirstEdgeSelector,
		typename BackwardSelector = BackwardEdgeSelector>
		requires bidirectional_storage<typename std::remove_cvref_t<Graph>::StorageType>
	class BidirectionalVisitor
	{
	public:
		using GraphType        = std::remove_cvref_t<Graph>;
		using VertexDescriptor = GraphType::VertexDescriptor;
		using EdgeDescriptor   = GraphType::EdgeDescriptor;

	private:
		std::conditional_t<std::is_const_v<std::remove_reference_t<Graph>>, const GraphType*, GraphType*> graph_{
			nullptr};

		std::optional<VertexDescriptor> position_{};
		[[no_unique_address]] ForwardSelector forwardSelector_{};
		[[no_unique_address]] BackwardSelector backwardSelector_{};

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		constexpr BidirectionalVisitor() = default;

		constexpr explicit BidirectionalVisitor(Graph& graph) :
			graph_(&graph)
		{
		}

		constexpr BidirectionalVisitor(Graph& graph, VertexDescriptor start) :
			graph_(&graph),
			position_(start)
		{
		}

		constexpr BidirectionalVisitor(Graph& graph, ForwardSelector fwdSelector, BackwardSelector bwdSelector) :
			graph_(&graph),
			forwardSelector_(std::move(fwdSelector)),
			backwardSelector_(std::move(bwdSelector))
		{
		}

		// ========================================================================
		// Position Management
		// ========================================================================

		[[nodiscard]] constexpr auto position() const noexcept -> std::optional<VertexDescriptor>
		{
			return position_;
		}

		[[nodiscard]] constexpr auto is_positioned() const noexcept -> bool
		{
			return position_.has_value();
		}

		constexpr void move_to(VertexDescriptor v)
		{
			position_ = v;
		}

		constexpr void reset()
		{
			position_ = std::nullopt;
		}

		// ========================================================================
		// Graph Access
		// ========================================================================

		[[nodiscard]] constexpr auto graph() const noexcept -> const GraphType&
		{
			return *graph_;
		}

		// ========================================================================
		// Bidirectional Traversal
		// ========================================================================

		/**
		 * @brief Advance forward (follow out-edges)
		 */
		[[nodiscard]] constexpr auto advance_forward() -> AdvanceResult requires edge_selector<
			ForwardSelector, GraphType>
		{
			if (!position_)
			{
				return AdvanceResult::INVALID_STATE;
			}

			auto edge = forwardSelector_.select(*graph_, *position_);
			if (!edge)
			{
				return AdvanceResult::NO_EDGE;
			}

			position_ = graph_->Target(*edge);
			return AdvanceResult::MOVED;
		}

		/**
		 * @brief Advance backward (follow in-edges)
		 */
		[[nodiscard]] constexpr auto advance_backward() -> AdvanceResult requires edge_selector<
			BackwardSelector, GraphType>
		{
			if (!position_)
			{
				return AdvanceResult::INVALID_STATE;
			}

			auto edge = backwardSelector_.select(*graph_, *position_);
			if (!edge)
			{
				return AdvanceResult::NO_EDGE;
			}

			position_ = graph_->Source(*edge);
			return AdvanceResult::MOVED;
		}

		/**
		 * @brief Advance in specified direction
		 */
		[[nodiscard]] constexpr auto advance(TraversalDirection dir) -> AdvanceResult
		{
			return dir == TraversalDirection::FORWARD ? advance_forward() : advance_backward();
		}

		/**
		 * @brief Check if forward advance would succeed
		 */
		[[nodiscard]] constexpr auto can_advance_forward() const -> bool requires edge_selector<
			ForwardSelector, GraphType>
		{
			if (!position_)
			{
				return false;
			}
			return forwardSelector_.select(*graph_, *position_).has_value();
		}

		/**
		 * @brief Check if backward advance would succeed
		 */
		[[nodiscard]] constexpr auto can_advance_backward() const -> bool requires edge_selector<
			BackwardSelector, GraphType>
		{
			if (!position_)
			{
				return false;
			}
			return backwardSelector_.select(*graph_, *position_).has_value();
		}

		/**
		 * @brief Get the out-degree at current position
		 */
		[[nodiscard]] constexpr auto OutDegree() const noexcept -> std::optional<typename GraphType::DegreeSizeType>
		{
			if (!position_)
			{
				return std::nullopt;
			}
			return graph_->OutDegree(*position_);
		}

		/**
		 * @brief Get the in-degree at current position
		 */
		[[nodiscard]] constexpr auto InDegree() const noexcept -> std::optional<typename GraphType::DegreeSizeType>
		{
			if (!position_)
			{
				return std::nullopt;
			}
			return graph_->InDegree(*position_);
		}
	};

	// Deduction guides for BidirectionalVisitor
	template<typename G>
		requires bidirectional_storage<typename std::remove_cvref_t<G>::StorageType>
	BidirectionalVisitor(G&) -> BidirectionalVisitor<G>;

	template<typename G, typename V>
		requires bidirectional_storage<typename std::remove_cvref_t<G>::StorageType> and
		         std::same_as<V, typename std::remove_cvref_t<G>::VertexDescriptor>
	BidirectionalVisitor(G&, V) -> BidirectionalVisitor<G>;

	/**
	 * @brief Factory function for bidirectional visitor
	 */
	template<typename Graph>
		requires bidirectional_storage<typename std::remove_cvref_t<Graph>::StorageType>
	[[nodiscard]] constexpr auto make_bidirectional_visitor(Graph& graph)
	{
		return BidirectionalVisitor<Graph>{graph};
	}

	/**
	 * @brief Factory function for bidirectional visitor at a specific position
	 */
	template<typename Graph>
		requires bidirectional_storage<typename std::remove_cvref_t<Graph>::StorageType>
	[[nodiscard]] constexpr auto
		make_bidirectional_visitor(Graph& graph, typename std::remove_cvref_t<Graph>::VertexDescriptor start)
	{
		return BidirectionalVisitor<Graph>{graph, start};
	}
}
