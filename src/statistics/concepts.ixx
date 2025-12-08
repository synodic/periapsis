/**
 * @file concepts.ixx
 * @brief Core concepts for the statistics accumulator framework
 *
 * @details Provides the fundamental concepts that define statistical sample types,
 *          accumulators, features, and their dependencies. This module forms the
 *          type-theoretic foundation for the accumulator framework.
 */

export module synodic.library:statistics.concepts;

import std;

export namespace synodic::statistics
{
	// ============================================================================
	// Sample Type Concepts
	// ============================================================================

	/**
	 * @brief A type suitable for statistical accumulation
	 * @details Must support arithmetic operations needed for statistical computations
	 */
	template<typename T>
	concept statistical_sample = std::is_arithmetic_v<T> and std::is_trivially_copyable_v<T>;

	/**
	 * @brief A floating-point type for statistical results
	 */
	template<typename T>
	concept statistical_result = std::floating_point<T>;

	// ============================================================================
	// Accumulator Concepts
	// ============================================================================

	/**
	 * @brief Basic accumulator that can receive samples and produce results
	 */
	template<typename A, typename Sample>
	concept basic_accumulator = requires(A& acc, const A& cacc, Sample s) {
		// Can accumulate a sample
		{ acc(s) } -> std::same_as<void>;
		// Can extract result
		{ cacc.result() };
		// Can reset to initial state
		{ acc.reset() } -> std::same_as<void>;
	};

	/**
	 * @brief Accumulator that can also accept weighted samples
	 * @details The weight is optional - accumulator works with or without weights
	 */
	template<typename A, typename Sample, typename Weight = Sample>
	concept weighted_accumulator = basic_accumulator<A, Sample> and requires(A& acc, Sample s, Weight w) {
		// Can accumulate a weighted sample (same function, optional weight)
		{ acc(s, w) } -> std::same_as<void>;
	};

	/**
	 * @brief Accumulator that maintains a rolling window of samples
	 */
	template<typename A, typename Sample>
	concept rolling_accumulator = basic_accumulator<A, Sample> and requires(const A& acc) {
		// Query the window size
		{ A::window_size } -> std::convertible_to<std::size_t>;
		// Check if window is full
		{ acc.window_full() } -> std::convertible_to<bool>;
	};

	// ============================================================================
	// Feature Tag Concepts
	// ============================================================================

	/**
	 * @brief A feature tag that identifies a statistical computation
	 * @details Features define what statistics to compute and their dependencies
	 */
	template<typename F>
	concept feature_tag = requires {
		// The accumulator type template this feature uses
		typename F::template accumulator_type<double>;
	};

	/**
	 * @brief A feature with explicit dependencies on other features
	 */
	template<typename F>
	concept dependent_feature = feature_tag<F> and requires {
		// Tuple of dependent feature tags
		typename F::dependencies;
	};

	// ============================================================================
	// Result Extractor Concepts
	// ============================================================================

	/**
	 * @brief Can extract a specific feature's result from an accumulator set
	 */
	template<typename E, typename AccSet, typename Feature>
	concept result_extractor = requires(const AccSet& acc) {
		{ E::template extract<Feature>(acc) };
	};

	// ============================================================================
	// Composable Accumulator Concepts
	// ============================================================================

	/**
	 * @brief An accumulator that can be composed with others in a set
	 */
	template<typename A, typename Sample>
	concept composable_accumulator = basic_accumulator<A, Sample> and requires {
		// Has a feature tag type
		typename A::feature_type;
	};
}
