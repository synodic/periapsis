/**
 * @file query.ixx
 * @brief Free functions for querying statistical results
 *
 * @details Provides convenient free functions to extract results from
 *          Accumulator instances. These are designed for ADL (Argument
 *          Dependent Lookup) so they can be called unqualified:
 *
 * @example
 *     using namespace synodic::statistics;
 *     Accumulator<double, mean, variance> stats;
 *     stats(1.0); stats(2.0); stats(3.0);
 *
 *     auto m = mean(stats);      // ADL finds synodic::statistics::mean()
 *     auto v = variance(stats);
 *     auto s = stddev(stats);
 */

export module synodic.library:statistics.query;

import :statistics.concepts;
import :statistics.features;
import :statistics.accumulator;

import std;

export namespace synodic::statistics
{
	// ============================================================================
	// Type Trait: Has Feature
	// ============================================================================

	namespace detail
	{
		template<typename Feature, typename AccSet>
		struct accumulator_has_feature : std::false_type
		{
		};

		template<typename Feature, statistical_sample Sample, typename... Features>
		struct accumulator_has_feature<Feature, Accumulator<Sample, Features...>> :
			std::bool_constant<(std::is_same_v<Feature, Features> || ...)>
		{
		};

		template<typename Feature, typename AccSet>
		inline constexpr bool accumulator_has_feature_v = accumulator_has_feature<Feature, AccSet>::value;
	}

	// ============================================================================
	// Basic Extractors
	// ============================================================================

	/**
	 * @brief Extract sample count
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<count, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto extract_count(const AccSet& acc) noexcept
	{
		return acc.template get<count>().result();
	}

	/**
	 * @brief Extract sum
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<sum, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto extract_sum(const AccSet& acc) noexcept
	{
		return acc.template get<sum>().result();
	}

	/**
	 * @brief Extract minimum value
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<min, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto extract_min(const AccSet& acc) noexcept
	{
		return acc.template get<min>().result();
	}

	/**
	 * @brief Extract maximum value
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<max, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto extract_max(const AccSet& acc) noexcept
	{
		return acc.template get<max>().result();
	}

	/**
	 * @brief Extract arithmetic mean
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<mean, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto extract_mean(const AccSet& acc) noexcept
	{
		return acc.template get<mean>().result();
	}

	/**
	 * @brief Extract sample variance
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<variance, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto extract_variance(const AccSet& acc) noexcept
	{
		return acc.template get<variance>().result();
	}

	/**
	 * @brief Extract population variance
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<variance, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto extract_population_variance(const AccSet& acc) noexcept
	{
		return acc.template get<variance>().population_variance();
	}

	/**
	 * @brief Extract standard deviation
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<variance, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto extract_stddev(const AccSet& acc) noexcept
	{
		return acc.template get<variance>().stddev();
	}

	/**
	 * @brief Extract population standard deviation
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<variance, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto extract_population_stddev(const AccSet& acc) noexcept
	{
		return acc.template get<variance>().population_stddev();
	}

	/**
	 * @brief Extract skewness
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<skewness, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto extract_skewness(const AccSet& acc) noexcept
	{
		return acc.template get<skewness>().result();
	}

	/**
	 * @brief Extract excess kurtosis
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<kurtosis, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto extract_kurtosis(const AccSet& acc) noexcept
	{
		return acc.template get<kurtosis>().result();
	}

	// ============================================================================
	// N-th Moment Extractor
	// ============================================================================

	/**
	 * @brief Extract N-th moment
	 */
	template<std::size_t N, typename AccSet>
		requires detail::accumulator_has_feature_v<moment<N>, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto extract_moment(const AccSet& acc) noexcept
	{
		return acc.template get<moment<N>>().result();
	}

	// ============================================================================
	// Rolling Statistics Extractors
	// ============================================================================

	/**
	 * @brief Extract rolling mean
	 */
	template<std::size_t WindowSize, typename AccSet>
		requires detail::accumulator_has_feature_v<rolling_mean<WindowSize>, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto extract_rolling_mean(const AccSet& acc) noexcept
	{
		return acc.template get<rolling_mean<WindowSize>>().result();
	}

	/**
	 * @brief Extract rolling variance
	 */
	template<std::size_t WindowSize, typename AccSet>
		requires detail::accumulator_has_feature_v<rolling_variance<WindowSize>, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto extract_rolling_variance(const AccSet& acc) noexcept
	{
		return acc.template get<rolling_variance<WindowSize>>().result();
	}

	/**
	 * @brief Extract rolling standard deviation
	 */
	template<std::size_t WindowSize, typename AccSet>
		requires detail::accumulator_has_feature_v<rolling_variance<WindowSize>, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto extract_rolling_stddev(const AccSet& acc) noexcept
	{
		return acc.template get<rolling_variance<WindowSize>>().stddev();
	}

	// ============================================================================
	// Composite Statistics Functions
	// ============================================================================

	/**
	 * @brief Compute coefficient of variation (CV = stddev / mean)
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<variance, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto coefficient_of_variation(const AccSet& acc) noexcept
	{
		const auto m = acc.template get<variance>().mean();
		if (m == decltype(m) {})
		{
			return decltype(m) {};
		}
		return acc.template get<variance>().stddev() / m;
	}

	/**
	 * @brief Compute standard error of the mean (SEM = stddev / sqrt(n))
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<variance, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto standard_error(const AccSet& acc) noexcept
	{
		const auto n = acc.template get<variance>().count();
		if (n == 0)
		{
			return decltype(acc.template get<variance>().stddev()) {};
		}
		return acc.template get<variance>().stddev() / std::sqrt(static_cast<double>(n));
	}

	/**
	 * @brief Get the range (max - min)
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<min, std::remove_cvref_t<AccSet>> and
				 detail::accumulator_has_feature_v<max, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto range(const AccSet& acc) noexcept
	{
		const auto min_val = acc.template get<min>().result();
		const auto max_val = acc.template get<max>().result();
		if (!min_val || !max_val)
		{
			return std::optional<typename std::remove_cvref_t<AccSet>::sample_type> {};
		}
		return std::optional {*max_val - *min_val};
	}

	// ============================================================================
	// Summary Statistics
	// ============================================================================

	/**
	 * @brief A snapshot of basic descriptive statistics
	 */
	template<typename T>
	struct StatsSummary
	{
		std::size_t count {};
		T sum {};
		T mean {};
		T variance {};
		T stddev {};
		std::optional<T> min {};
		std::optional<T> max {};
	};

	/**
	 * @brief Extract a summary of all basic statistics
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<count, std::remove_cvref_t<AccSet>> and
				 detail::accumulator_has_feature_v<sum, std::remove_cvref_t<AccSet>> and
				 detail::accumulator_has_feature_v<mean, std::remove_cvref_t<AccSet>> and
				 detail::accumulator_has_feature_v<variance, std::remove_cvref_t<AccSet>> and
				 detail::accumulator_has_feature_v<min, std::remove_cvref_t<AccSet>> and
				 detail::accumulator_has_feature_v<max, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto summary(const AccSet& acc) noexcept
	{
		using T		  = typename std::remove_cvref_t<AccSet>::sample_type;
		using ResultT = std::conditional_t<std::floating_point<T>, T, double>;

		return StatsSummary<ResultT> {
			.count	  = extract_count(acc),
			.sum	  = extract_sum(acc),
			.mean	  = extract_mean(acc),
			.variance = extract_variance(acc),
			.stddev	  = extract_stddev(acc),
			.min	  = extract_min(acc),
			.max	  = extract_max(acc)};
	}
}
