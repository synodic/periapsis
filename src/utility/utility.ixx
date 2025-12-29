export module synodic.periapsis:utility;

import std;

namespace peri
{
	export template<std::size_t N>
	constexpr decltype(auto) nth_element(auto&&... args)
		requires(N < sizeof...(args))
	{
		return std::get<N>(std::forward_as_tuple(std::forward<decltype(args)>(args)...));
	}
}
