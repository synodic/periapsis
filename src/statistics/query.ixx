export module synodic.periapsis:statistics.query;

import :statistics.concepts;
import :statistics.features;
import :statistics.accumulator;

import std;

export namespace peri::statistics
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
		requires detail::accumulator_has_feature_v<Count, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto ExtractCount(const AccSet& acc) noexcept
	{
		return acc.template Get<Count>().Result();
	}

	/**
	 * @brief Extract sum
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<Sum, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto ExtractSum(const AccSet& acc) noexcept
	{
		return acc.template Get<Sum>().Result();
	}

	/**
	 * @brief Extract minimum value
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<Min, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto ExtractMin(const AccSet& acc) noexcept
	{
		return acc.template Get<Min>().Result();
	}

	/**
	 * @brief Extract maximum value
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<Max, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto ExtractMax(const AccSet& acc) noexcept
	{
		return acc.template Get<Max>().Result();
	}

	/**
	 * @brief Extract arithmetic mean
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<Mean, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto ExtractMean(const AccSet& acc) noexcept
	{
		return acc.template Get<Mean>().Result();
	}

	/**
	 * @brief Extract sample variance
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<Variance, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto ExtractVariance(const AccSet& acc) noexcept
	{
		return acc.template Get<Variance>().Result();
	}

	/**
	 * @brief Extract population variance
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<Variance, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto ExtractPopulationVariance(const AccSet& acc) noexcept
	{
		return acc.template Get<Variance>().PopulationVariance();
	}

	/**
	 * @brief Extract standard deviation
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<Variance, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto ExtractStdDev(const AccSet& acc) noexcept
	{
		return acc.template Get<Variance>().StdDev();
	}

	/**
	 * @brief Extract population standard deviation
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<Variance, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto ExtractPopulationStdDev(const AccSet& acc) noexcept
	{
		return acc.template Get<Variance>().PopulationStdDev();
	}

	/**
	 * @brief Extract skewness
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<Skewness, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto ExtractSkewness(const AccSet& acc) noexcept
	{
		return acc.template Get<Skewness>().Result();
	}

	/**
	 * @brief Extract excess kurtosis
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<Kurtosis, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto ExtractKurtosis(const AccSet& acc) noexcept
	{
		return acc.template Get<Kurtosis>().Result();
	}

	// ============================================================================
	// N-th Moment Extractor
	// ============================================================================

	/**
	 * @brief Extract N-th moment
	 */
	template<std::size_t N, typename AccSet>
		requires detail::accumulator_has_feature_v<Moment<N>, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto ExtractMoment(const AccSet& acc) noexcept
	{
		return acc.template Get<Moment<N>>().Result();
	}

	// ============================================================================
	// Rolling Statistics Extractors
	// ============================================================================

	/**
	 * @brief Extract rolling mean
	 */
	template<std::size_t WindowSize, typename AccSet>
		requires detail::accumulator_has_feature_v<RollingMean<WindowSize>, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto ExtractRollingMean(const AccSet& acc) noexcept
	{
		return acc.template Get<RollingMean<WindowSize>>().Result();
	}

	/**
	 * @brief Extract rolling variance
	 */
	template<std::size_t WindowSize, typename AccSet>
		requires detail::accumulator_has_feature_v<RollingVariance<WindowSize>, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto ExtractRollingVariance(const AccSet& acc) noexcept
	{
		return acc.template Get<RollingVariance<WindowSize>>().Result();
	}

	/**
	 * @brief Extract rolling standard deviation
	 */
	template<std::size_t WindowSize, typename AccSet>
		requires detail::accumulator_has_feature_v<RollingVariance<WindowSize>, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto ExtractRollingStdDev(const AccSet& acc) noexcept
	{
		return acc.template Get<RollingVariance<WindowSize>>().StdDev();
	}

	// ============================================================================
	// Composite Statistics Functions
	// ============================================================================

	/**
	 * @brief Compute coefficient of variation (CV = stdDev / mean)
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<Variance, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto CoefficientOfVariation(const AccSet& acc) noexcept
	{
		const auto m = acc.template Get<Variance>().Mean();
		if (m == decltype(m){})
		{
			return decltype(m){};
		}
		return acc.template Get<Variance>().StdDev() / m;
	}

	/**
	 * @brief Compute standard error of the mean (SEM = stdDev / sqrt(n))
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<Variance, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto StandardError(const AccSet& acc) noexcept
	{
		const auto n = acc.template Get<Variance>().Count();
		if (n == 0)
		{
			return decltype(acc.template Get<Variance>().StdDev()){};
		}
		return acc.template Get<Variance>().StdDev() / std::sqrt(static_cast<double>(n));
	}

	/**
	 * @brief Get the range (max - min)
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<Min, std::remove_cvref_t<AccSet>> and
		         detail::accumulator_has_feature_v<Max, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto Range(const AccSet& acc) noexcept
	{
		const auto min_val = acc.template Get<Min>().Result();
		const auto max_val = acc.template Get<Max>().Result();
		if (!min_val || !max_val)
		{
			return std::optional<typename std::remove_cvref_t<AccSet>::SampleType>{};
		}
		return std::optional{*max_val - *min_val};
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
		std::size_t count{};
		T sum{};
		T mean{};
		T variance{};
		T stdDev{};
		std::optional<T> min{};
		std::optional<T> max{};
	};

	/**
	 * @brief Extract a summary of all basic statistics
	 */
	template<typename AccSet>
		requires detail::accumulator_has_feature_v<Count, std::remove_cvref_t<AccSet>> and
		         detail::accumulator_has_feature_v<Sum, std::remove_cvref_t<AccSet>> and
		         detail::accumulator_has_feature_v<Mean, std::remove_cvref_t<AccSet>> and
		         detail::accumulator_has_feature_v<Variance, std::remove_cvref_t<AccSet>> and
		         detail::accumulator_has_feature_v<Min, std::remove_cvref_t<AccSet>> and
		         detail::accumulator_has_feature_v<Max, std::remove_cvref_t<AccSet>>
	[[nodiscard]] constexpr auto Summary(const AccSet& acc) noexcept
	{
		using T       = typename std::remove_cvref_t<AccSet>::SampleType;
		using ResultT = std::conditional_t<std::floating_point<T>, T, double>;

		return StatsSummary<ResultT>{
			.count = ExtractCount(acc),
			.sum = ExtractSum(acc),
			.mean = ExtractMean(acc),
			.variance = ExtractVariance(acc),
			.stdDev = ExtractStdDev(acc),
			.min = ExtractMin(acc),
			.max = ExtractMax(acc)};
	}
}
