export module synodic.periapsis:type.concepts.algebraic;

import std;

export namespace peri
{
	/**
	 * @brief A type that supports associative binary addition operation.
	 * 
	 * Can't verify algebraic associativity
	 */
	template<typename T>
	concept semigroup = requires(T a, T b)
	{
		{ a + b } -> std::same_as<T>;
	};
}
