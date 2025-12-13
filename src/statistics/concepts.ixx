export module synodic.periapsis:statistics.concepts;

import std;

export namespace peri::statistics
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
	concept basic_accumulator = requires(A& acc, const A& constAcc, Sample s)
	{
		// Can accumulate a sample
		{ acc.Push(s) } -> std::same_as<void>;
		// Can extract result
		{ constAcc.Result() };
		// Can reset to initial state
		{ acc.Reset() } -> std::same_as<void>;
	};

	/**
	 * @brief Accumulator that can also accept weighted samples
	 * @details The weight is optional - accumulator works with or without weights
	 */
	template<typename A, typename Sample, typename Weight = Sample>
	concept weighted_accumulator = basic_accumulator<A, Sample> and requires(A& acc, Sample s, Weight w)
	{
		// Can accumulate a weighted sample
		{ acc.Push(s, w) } -> std::same_as<void>;
	};

	/**
	 * @brief Accumulator that maintains a rolling window of samples
	 */
	template<typename A, typename Sample>
	concept rolling_accumulator = basic_accumulator<A, Sample> and requires(const A& acc)
	{
		// Query the window size
		{ A::WINDOW_SIZE } -> std::convertible_to<std::size_t>;
		// Check if window is full
		{ acc.WindowFull() } -> std::convertible_to<bool>;
	};

	// ============================================================================
	// Feature Tag Concepts
	// ============================================================================

	/**
	 * @brief A feature tag that identifies a statistical computation
	 * @details Features define what statistics to compute and their dependencies
	 */
	template<typename F>
	concept feature_tag = requires
	{
		// The accumulator type template this feature uses
		typename F::template AccumulatorType<double>;
	};

	/**
	 * @brief A feature with explicit dependencies on other features
	 */
	template<typename F>
	concept dependent_feature = feature_tag<F> and requires
	{
		// Tuple of dependent feature tags
		typename F::Dependencies;
	};

	// ============================================================================
	// Result Extractor Concepts
	// ============================================================================

	/**
	 * @brief Can extract a specific feature's result from an accumulator set
	 */
	template<typename E, typename AccSet, typename Feature>
	concept result_extractor = requires(const AccSet& acc)
	{
		{ E::template Extract<Feature>(acc) };
	};

	// ============================================================================
	// Composable Accumulator Concepts
	// ============================================================================

	/**
	 * @brief An accumulator that can be composed with others in a set
	 */
	template<typename A, typename Sample>
	concept composable_accumulator = basic_accumulator<A, Sample> and requires
	{
		// Has a feature tag type
		typename A::FeatureType;
	};
}
