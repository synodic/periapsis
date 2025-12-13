export module synodic.periapsis:math.concepts;

import std;

export namespace peri::math
{
	/**
	 * @brief A type that can be used as a vector component
	 */
	template<typename T>
	concept vector_component = std::is_arithmetic_v<T> && std::is_trivially_copyable_v<T>;
}
