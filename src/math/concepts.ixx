/**
 * @brief Math-related concepts for the Synodic Template Library
 */

export module synodic.library:math.concepts;

import std;

export namespace synodic::math
{
	/**
	 * @brief A type that can be used as a vector component
	 * @details Must be arithmetic and trivially copyable for SIMD compatibility
	 */
	template<typename T>
	concept vector_component = std::is_arithmetic_v<T> && std::is_trivially_copyable_v<T>;
}
