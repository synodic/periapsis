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
		Moved,	// Successfully moved to a new vertex
		NoEdge,	 // No edge matched the selection criteria
		Blocked,  // Edge found but blocked by guard/predicate
		InvalidState  // Walker has no current position
	};

	// ============================================================================
	// Visitor Concept - Common interface for graph traversal
	// ============================================================================

	/**
	 * @brief Concept for any type that can visit/traverse a graph structure.
	 *        Both Visitor (graph) and Runner (FSM) satisfy this concept.
	 */
	template<typename V>
	concept visitor = requires(V& v, const V& cv) {
		typename V::vertex_descriptor;
		typename V::edge_descriptor;

		// Position query
		{ cv.position() } -> std::same_as<std::optional<typename V::vertex_descriptor>>;
		{ cv.is_positioned() } -> std::convertible_to<bool>;

		// Position control
		{ v.move_to(std::declval<typename V::vertex_descriptor>()) };
		{ v.reset() };
	};

	/**
	 * @brief Visitor that can advance without arguments (e.g., first-edge traversal)
	 */
	template<typename V>
	concept simple_visitor = visitor<V> and requires(V& v) {
		{ v.advance() } -> std::same_as<AdvanceResult>;
	};

	/**
	 * @brief Visitor that advances based on an event/key
	 */
	template<typename V, typename Event>
	concept event_visitor = visitor<V> and requires(V& v, const V& cv, const Event& e) {
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
		requires(const S& selector, const Graph& graph, typename Graph::vertex_descriptor v, Args&&... args) {
			{
				selector.select(graph, v, std::forward<Args>(args)...)
			} -> std::same_as<std::optional<typename Graph::edge_descriptor>>;
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
		[[nodiscard]] constexpr auto select(const Graph& graph, typename Graph::vertex_descriptor v) const noexcept
			-> std::optional<typename Graph::edge_descriptor>
		{
			auto edges = graph.out_edges(v);
			auto it	   = std::ranges::begin(edges);
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
		[[nodiscard]] constexpr auto select(const Graph& graph, typename Graph::vertex_descriptor v) const
			-> std::optional<typename Graph::edge_descriptor>
		{
			for (auto e: graph.out_edges(v))
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
	 * @brief Selects an edge by matching an event/key against edge properties
	 *        Core selector for FSM-style traversal
	 */
	template<typename Event, typename EventExtractor>
	struct EventSelector
	{
		EventExtractor extractor;

		template<typename Graph>
		[[nodiscard]] constexpr auto
			select(const Graph& graph, typename Graph::vertex_descriptor v, const Event& event) const
			-> std::optional<typename Graph::edge_descriptor>
		{
			for (auto e: graph.out_edges(v))
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
		[[nodiscard]] constexpr auto operator()(const Graph& graph, typename Graph::edge_descriptor e) const
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
		using graph_type		= std::remove_cvref_t<Graph>;
		using vertex_descriptor = typename graph_type::vertex_descriptor;
		using edge_descriptor	= typename graph_type::edge_descriptor;
		using selector_type		= Selector;

	private:
		// Pointer to graph (const or non-const based on Graph template param)
		std::conditional_t<std::is_const_v<std::remove_reference_t<Graph>>, const graph_type*, graph_type*> graph_ {
			nullptr};

		std::optional<vertex_descriptor> position_ {};
		[[no_unique_address]] Selector selector_ {};

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		constexpr Visitor() = default;

		/// Construct visitor on a graph (const or non-const)
		constexpr explicit Visitor(Graph& graph) :
			graph_(&graph)
		{
		}

		/// Construct visitor on a graph with custom selector
		constexpr Visitor(Graph& graph, Selector selector) :
			graph_(&graph),
			selector_(std::move(selector))
		{
		}

		/// Construct visitor at a specific starting position
		constexpr Visitor(Graph& graph, vertex_descriptor start) :
			graph_(&graph),
			position_(start)
		{
		}

		/// Construct visitor at a specific position with custom selector
		constexpr Visitor(Graph& graph, vertex_descriptor start, Selector selector) :
			graph_(&graph),
			position_(start),
			selector_(std::move(selector))
		{
		}

		// ========================================================================
		// Graph Access
		// ========================================================================

		[[nodiscard]] constexpr auto graph() const noexcept -> const graph_type&
		{
			return *graph_;
		}

		[[nodiscard]] constexpr auto graph() noexcept -> Graph&
			requires(not std::is_const_v<std::remove_reference_t<Graph>>)
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

		/// Get current position (nullopt if not positioned)
		[[nodiscard]] constexpr auto position() const noexcept -> std::optional<vertex_descriptor>
		{
			return position_;
		}

		/// Check if visitor is positioned on a vertex
		[[nodiscard]] constexpr auto is_positioned() const noexcept -> bool
		{
			return position_.has_value();
		}

		/// Move to a specific vertex
		constexpr void move_to(vertex_descriptor v)
		{
			position_ = v;
		}

		/// Clear position (visitor becomes unpositioned)
		constexpr void reset()
		{
			position_ = std::nullopt;
		}

		// ========================================================================
		// Traversal
		// ========================================================================

		/// Advance using selector with no additional arguments
		[[nodiscard]] constexpr auto advance() -> AdvanceResult
			requires edge_selector<Selector, graph_type>
		{
			if (!position_)
			{
				return AdvanceResult::InvalidState;
			}

			auto edge = selector_.select(*graph_, *position_);
			if (!edge)
			{
				return AdvanceResult::NoEdge;
			}

			position_ = graph_->target(*edge);
			return AdvanceResult::Moved;
		}

		/// Advance using selector with additional arguments (e.g., event for EventSelector)
		template<typename... Args>
			requires edge_selector<Selector, graph_type, Args...>
		[[nodiscard]] constexpr auto advance(Args&&... args) -> AdvanceResult
		{
			if (!position_)
			{
				return AdvanceResult::InvalidState;
			}

			auto edge = selector_.select(*graph_, *position_, std::forward<Args>(args)...);
			if (!edge)
			{
				return AdvanceResult::NoEdge;
			}

			position_ = graph_->target(*edge);
			return AdvanceResult::Moved;
		}

		/// Advance with callback on success (receives edge that was taken)
		template<typename Callback, typename... Args>
			requires edge_selector<Selector, graph_type, Args...> and std::invocable<Callback, edge_descriptor>
		[[nodiscard]] constexpr auto advance_with(Callback&& callback, Args&&... args) -> AdvanceResult
		{
			if (!position_)
			{
				return AdvanceResult::InvalidState;
			}

			auto edge = selector_.select(*graph_, *position_, std::forward<Args>(args)...);
			if (!edge)
			{
				return AdvanceResult::NoEdge;
			}

			position_ = graph_->target(*edge);
			std::invoke(std::forward<Callback>(callback), *edge);
			return AdvanceResult::Moved;
		}

		/// Check if advance would succeed without actually moving
		template<typename... Args>
			requires edge_selector<Selector, graph_type, Args...>
		[[nodiscard]] constexpr auto can_advance(Args&&... args) const -> bool
		{
			if (!position_)
			{
				return false;
			}
			return selector_.select(*graph_, *position_, std::forward<Args>(args)...).has_value();
		}

		/// Get the edge that would be taken (without moving)
		template<typename... Args>
			requires edge_selector<Selector, graph_type, Args...>
		[[nodiscard]] constexpr auto peek(Args&&... args) const -> std::optional<edge_descriptor>
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
		requires std::same_as<V, typename std::remove_cvref_t<G>::vertex_descriptor>
	Visitor(G&, V) -> Visitor<G, FirstEdgeSelector>;

	template<typename G, typename V, typename S>
		requires std::same_as<V, typename std::remove_cvref_t<G>::vertex_descriptor>
	Visitor(G&, V, S) -> Visitor<G, S>;

	// ============================================================================
	// Factory Functions
	// ============================================================================

	/// Create a visitor on a graph
	template<typename Graph>
	[[nodiscard]] constexpr auto make_visitor(Graph& graph)
	{
		return Visitor<Graph> {graph};
	}

	/// Create a visitor with a custom selector
	template<typename Graph, typename Selector>
	[[nodiscard]] constexpr auto make_visitor(Graph& graph, Selector selector)
	{
		return Visitor<Graph, Selector> {graph, std::move(selector)};
	}

	/// Create a visitor starting at a specific vertex
	template<typename Graph>
	[[nodiscard]] constexpr auto
		make_visitor(Graph& graph, typename std::remove_cvref_t<Graph>::vertex_descriptor start)
	{
		return Visitor<Graph> {graph, start};
	}
}
