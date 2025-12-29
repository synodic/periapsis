export module synodic.periapsis:type.concepts.core;

import std;

export namespace peri
{
	/**
	 * @brief A type that is equal to its decayed form.
	 *
	 * @details Ensures the type has no cv-qualifiers, is not a reference,
	 *          and arrays/functions are converted to pointers.
	 */
	template<typename T>
	concept decayed = std::same_as<T, std::decay_t<T>>;
}
