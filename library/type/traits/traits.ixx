export module synodic.library:type.traits;

import std;

namespace synodic
{

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// RepresentationDeduction
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ConstructorParameters
	// TODO: Remove when reflection is available
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<typename T, size_t N>
	struct Tag
	{
		// Returns the type associated with the tag
		friend auto GetType(Tag<T, N>);

		// Returns the ID of the tag
		consteval friend size_t GetID(Tag<T, N>);
	};

	// The definitions of the friend functions elevating their scope. Prevent multiple
	// instantiations by requiring the types
	//		to be different
	template<typename T, typename U, size_t N>
		requires(!std::is_same_v<T, U>)
	struct FriendType
	{
		friend auto GetType(Tag<T, N>)
		{
			return U();
		}
		consteval friend size_t GetID(Tag<T, N>)
		{
			return 0;
		}
	};

	template<typename T, size_t N>
	struct TypeConversion
	{
		// Conversion helper
		template<typename U, size_t = sizeof(FriendType<T, U, N>)>
		consteval operator U()
		{
			return U();
		}
	};

	template<typename T, size_t... Ns>
	consteval auto ConstructorParameterCount(size_t) -> decltype(T(TypeConversion<T, Ns>()...), 0)
	{
		return sizeof...(Ns);
	}

	template<typename T, size_t... Ns>
	consteval size_t ConstructorParameterCount(...)
	{
		return ConstructorParameterCount<T, Ns..., sizeof...(Ns)>(0);
	}

	// Declaration required for specialization
	template<typename T, typename U>
	struct TaggedTuple;

	// Provide the only specialization, making the inputs an integer_sequence type
	template<typename T, size_t... Ns>
	struct TaggedTuple<T, std::index_sequence<Ns...>>
	{
		using Type = std::tuple<decltype(GetType(Tag<T, Ns>()))...>;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Exports
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Given a type T, returns a tuple of the parameters for the shortest constructor for that type
	export template<typename T>
	using ConstructorParameters =
		typename TaggedTuple<T, std::make_index_sequence<ConstructorParameterCount<T>(0)>>::Type;

}}
