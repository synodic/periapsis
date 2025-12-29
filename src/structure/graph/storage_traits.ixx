export module synodic.periapsis:structure.graph.storage_traits;

import :structure.graph.concepts;

import std;

export namespace peri::graph
{
	namespace detail
	{
		// Detection idiom for optional storage members
		template<typename S, typename = void>
		struct HasVertexPropertyType : std::false_type
		{
		};

		template<typename S>
		struct HasVertexPropertyType<S, std::void_t<typename S::VertexPropertyType>> : std::true_type
		{
		};

		template<typename S, typename = void>
		struct HasEdgePropertyType : std::false_type
		{
		};

		template<typename S>
		struct HasEdgePropertyType<S, std::void_t<typename S::EdgePropertyType>> : std::true_type
		{
		};

		template<typename S>
		consteval bool GetIsDirected() noexcept
		{
			if constexpr (requires
			{
				{ S::IS_DIRECTED } -> std::convertible_to<bool>;
			})
			{
				return S::IS_DIRECTED;
			}
			else
			{
				return true; // Default to directed
			}
		}

		template<typename S>
		consteval bool GetAllowsParallelEdges() noexcept
		{
			if constexpr (requires
			{
				{ S::ALLOWS_PARALLEL_EDGES } -> std::convertible_to<bool>;
			})
			{
				return S::ALLOWS_PARALLEL_EDGES;
			}
			else
			{
				return true; // Default to allowing parallel edges
			}
		}

		template<typename S>
		consteval bool GetIsConstexprCapable() noexcept
		{
			if constexpr (requires
			{
				{ S::IS_CONSTEXPR_CAPABLE } -> std::convertible_to<bool>;
			})
			{
				return S::IS_CONSTEXPR_CAPABLE;
			}
			else
			{
				return false;
			}
		}

		template<typename S>
		consteval bool GetHasStableVertexDescriptors() noexcept
		{
			if constexpr (requires
			{
				{ S::HAS_STABLE_VERTEX_DESCRIPTORS } -> std::convertible_to<bool>;
			})
			{
				return S::HAS_STABLE_VERTEX_DESCRIPTORS;
			}
			else
			{
				return false;
			}
		}

		template<typename S>
		consteval bool GetHasStableEdgeDescriptors() noexcept
		{
			if constexpr (requires
			{
				{ S::HAS_STABLE_EDGE_DESCRIPTORS } -> std::convertible_to<bool>;
			})
			{
				return S::HAS_STABLE_EDGE_DESCRIPTORS;
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
	struct StorageTraits
	{
		using StorageType = Storage;

		// Required types (must be provided by storage)
		using VertexDescriptor = Storage::VertexDescriptor;
		using EdgeDescriptor   = Storage::EdgeDescriptor;
		using VerticesSizeType = Storage::VerticesSizeType;
		using EdgesSizeType    = Storage::EdgesSizeType;
		using DegreeSizeType   = Storage::DegreeSizeType;

		// Optional property types (void if not provided)
		using VertexPropertyType = std::
		conditional_t<detail::HasVertexPropertyType<Storage>::value, typename Storage::VertexPropertyType, void>;

		using EdgePropertyType =
		std::conditional_t<detail::HasEdgePropertyType<Storage>::value, typename Storage::EdgePropertyType, void>;

		// Compile-time properties with defaults
		static constexpr bool IS_DIRECTED           = detail::GetIsDirected<Storage>();
		static constexpr bool ALLOWS_PARALLEL_EDGES = detail::GetAllowsParallelEdges<Storage>();

		// Descriptor stability (important for external handle storage)
		//	Stable descriptors remain valid after add/remove operations
		static constexpr bool HAS_STABLE_VERTEX_DESCRIPTORS = detail::GetHasStableVertexDescriptors<Storage>();
		static constexpr bool HAS_STABLE_EDGE_DESCRIPTORS   = detail::GetHasStableEdgeDescriptors<Storage>();

		// Capability flags derived from concepts
		static constexpr bool SUPPORTS_VERTEX_LIST       = vertex_list_storage<Storage>;
		static constexpr bool SUPPORTS_EDGE_LIST         = edge_list_storage<Storage>;
		static constexpr bool SUPPORTS_INCIDENCE         = incidence_storage<Storage>;
		static constexpr bool SUPPORTS_BIDIRECTIONAL     = bidirectional_storage<Storage>;
		static constexpr bool SUPPORTS_ADJACENCY         = adjacency_storage<Storage>;
		static constexpr bool IS_MUTABLE                 = mutable_storage<Storage>;
		static constexpr bool SUPPORTS_VERTEX_PROPERTIES = vertex_property_storage<Storage>;
		static constexpr bool SUPPORTS_EDGE_PROPERTIES   = edge_property_storage<Storage>;
		static constexpr bool IS_COMPOSABLE              = composable_storage<Storage>;

		// Rebind to different property types (if storage supports it)
		template<typename VertexProp, typename EdgeProp>
		using RebindStorage = Storage::template Rebind<VertexProp, EdgeProp>;
	};

	/**
	 * @brief Tag type indicating no property associated with vertices or edges
	 */
	struct NoProperty
	{
	};
}
