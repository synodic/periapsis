export module synodic.periapsis:type.traits;

import std;

namespace peri
{
	namespace detail
	{
		template<std::size_t N, typename... Ts>
		struct MinimalIntegerImpl;

		template<std::size_t N>
		struct MinimalIntegerImpl<N>
		{
			using Type = void; // No suitable type found
		};

		template<std::size_t N, typename T, typename... Rest>
		struct MinimalIntegerImpl<N, T, Rest...>
		{
			using Type = std::
			conditional_t<N <= std::numeric_limits<T>::max(), T, typename MinimalIntegerImpl<N, Rest...>::Type>;
		};
	}

	/**
	 *	Evaluates the given integer and returns the minimally sized unsigned integral type that can hold the value.
	 *		If the value cannot be held by any integral type, the alias resolves to void.
	 */
	export template<std::size_t N>
	using MinimalUnsignedType =
	detail::MinimalIntegerImpl<N, std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t>::Type;

	/**
	 *	Evaluates the given integer and returns the minimally sized signed integral type that can hold the value.
	 *		If the value cannot be held by any signed integral type, the alias resolves to void.
	 */
	export template<std::size_t N>
	using MinimalSignedType =
	detail::MinimalIntegerImpl<N, std::int8_t, std::int16_t, std::int32_t, std::int64_t>::Type;

	/**
	 *	@deprecated Use MinimalUnsignedType instead
	 */
	export template<std::size_t N>
	using MinimalIntegerType = MinimalUnsignedType<N>;
}
