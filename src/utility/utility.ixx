export module synodic.periapsis:utility;

import std;

namespace peri
{
	// TODO: Don't export with module
	template<std::size_t Size>
	struct Any
	{
		template<typename T>
		constexpr Any(T);
	};

	template<std::size_t Size>
	template<typename T>
	constexpr Any<Size>::Any(T)
	{
	}

	export template<auto N>
	constexpr auto nth_element(auto... args)
	{
		return [&]<std::size_t... Ns>(std::index_sequence<Ns...>)
		{
			return [](Any<Ns>..., auto nth, auto...)
			{
				return nth;
			}(args...);
		}
		(std::make_index_sequence<N> {});
	}

}
