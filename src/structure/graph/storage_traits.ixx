export module synodic.library:structure.graph.storage_traits;

import :structure.graph.concepts;

import std;

export namespace synodic::graph
{
	namespace detail
	{
		// Detection idiom for optional storage members
		template<typename S, typename = void>
		struct has_vertex_property_type : std::false_type
		{
		};

		template<typename S>
		struct has_vertex_property_type<S, std::void_t<typename S::vertex_property_type>> : std::true_type
		{
		};

		template<typename S, typename = void>
		struct has_edge_property_type : std::false_type
		{
		};

		template<typename S>
		struct has_edge_property_type<S, std::void_t<typename S::edge_property_type>> : std::true_type
		{
		};

		template<typename S>
		consteval bool get_is_directed() noexcept
		{
			if constexpr (requires {
							  { S::is_directed } -> std::convertible_to<bool>;
						  })
			{
				return S::is_directed;
			}
			else
			{
				return true;  // Default to directed
			}
		}

		template<typename S>
		consteval bool get_allows_parallel_edges() noexcept
		{
			if constexpr (requires {
							  { S::allows_parallel_edges } -> std::convertible_to<bool>;
						  })
			{
				return S::allows_parallel_edges;
			}
			else
			{
				return true;  // Default to allowing parallel edges
			}
		}

		template<typename S>
		consteval bool get_is_constexpr_capable() noexcept
		{
			if constexpr (requires {
							  { S::is_constexpr_capable } -> std::convertible_to<bool>;
						  })
			{
				return S::is_constexpr_capable;
			}
			else
			{
				return false;
			}
		}

		template<typename S>
		consteval bool get_has_stable_vertex_descriptors() noexcept
		{
			if constexpr (requires {
							  { S::has_stable_vertex_descriptors } -> std::convertible_to<bool>;
						  })
			{
				return S::has_stable_vertex_descriptors;
			}
			else
			{
				return false;
			}
		}

		template<typename S>
		consteval bool get_has_stable_edge_descriptors() noexcept
		{
			if constexpr (requires {
							  { S::has_stable_edge_descriptors } -> std::convertible_to<bool>;
						  })
			{
				return S::has_stable_edge_descriptors;
			}
			else
			{
				return false;
			}
		}
	}

	/**
	 * @brief Provides a uniform interface to graph storage types, similar to std::allocator_traits.
	 *        Extracts type information and compile-time properties, providing sensible defaults
	 *        for optional members.
	 * @tparam Storage The graph storage type to query
	 */
	template<graph_storage Storage>
	struct storage_traits
	{
		using storage_type = Storage;

		// Required types (must be provided by storage)
		using vertex_descriptor	 = typename Storage::vertex_descriptor;
		using edge_descriptor	 = typename Storage::edge_descriptor;
		using vertices_size_type = typename Storage::vertices_size_type;
		using edges_size_type	 = typename Storage::edges_size_type;
		using degree_size_type	 = typename Storage::degree_size_type;

		// Optional property types (void if not provided)
		using vertex_property_type = std::conditional_t<
			detail::has_vertex_property_type<Storage>::value,
			typename Storage::vertex_property_type,
			void>;

		using edge_property_type = std::
			conditional_t<detail::has_edge_property_type<Storage>::value, typename Storage::edge_property_type, void>;

		// Compile-time properties with defaults
		static constexpr bool is_directed			= detail::get_is_directed<Storage>();
		static constexpr bool allows_parallel_edges = detail::get_allows_parallel_edges<Storage>();
		static constexpr bool is_constexpr_capable	= detail::get_is_constexpr_capable<Storage>();

		// Descriptor stability (important for external handle storage)
		//	Stable descriptors remain valid after add/remove operations
		static constexpr bool has_stable_vertex_descriptors = detail::get_has_stable_vertex_descriptors<Storage>();
		static constexpr bool has_stable_edge_descriptors	= detail::get_has_stable_edge_descriptors<Storage>();

		// Capability flags derived from concepts
		static constexpr bool supports_vertex_list		 = vertex_list_storage<Storage>;
		static constexpr bool supports_edge_list		 = edge_list_storage<Storage>;
		static constexpr bool supports_incidence		 = incidence_storage<Storage>;
		static constexpr bool supports_bidirectional	 = bidirectional_storage<Storage>;
		static constexpr bool supports_adjacency		 = adjacency_storage<Storage>;
		static constexpr bool is_mutable				 = mutable_storage<Storage>;
		static constexpr bool supports_vertex_properties = vertex_property_storage<Storage>;
		static constexpr bool supports_edge_properties	 = edge_property_storage<Storage>;
		static constexpr bool is_composable				 = composable_storage<Storage>;

		// Rebind to different property types (if storage supports it)
		template<typename VertexProp, typename EdgeProp>
		using rebind_storage = std::conditional_t<requires {
			typename Storage::template rebind<VertexProp, EdgeProp>;
		}, typename Storage::template rebind<VertexProp, EdgeProp>, Storage>;
	};

	/**
	 * @brief Tag type indicating no property associated with vertices or edges
	 */
	struct no_property
	{
	};
}
