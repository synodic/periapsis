export module synodic.periapsis:statistics.accumulator;

import :statistics.concepts;
import :statistics.features;

import std;

export namespace peri::statistics
{
	// ============================================================================
	// Implementation Details
	// ============================================================================

	namespace detail
	{
		/**
		 * @brief Find the index of a feature type in a parameter pack
		 */
		template<typename Target, typename... Features>
		struct FeatureIndex;

		template<typename Target, typename First, typename... Rest>
		struct FeatureIndex<Target, First, Rest...>
		{
			static constexpr std::size_t VALUE =
				std::is_same_v<Target, First> ? 0 : 1 + FeatureIndex<Target, Rest...>::VALUE;
		};

		template<typename Target>
		struct FeatureIndex<Target>
		{
			static constexpr std::size_t VALUE = 0; // Will cause compile error if used
		};

		template<typename Target, typename... Features>
		inline constexpr std::size_t feature_index_v = FeatureIndex<Target, Features...>::VALUE;

		/**
		 * @brief Check if a feature is in the parameter pack
		 */
		template<typename Target, typename... Features>
		inline constexpr bool has_feature_v = (std::is_same_v<Target, Features> || ...);
	}

	// ============================================================================
	// Accumulator
	// ============================================================================

	/**
	 * @brief A compile-time composable set of statistical accumulators
	 *
	 * @tparam Sample The sample type (must satisfy statistical_sample)
	 * @tparam Features... The feature tags to include (count, mean, variance, etc.)
	 *
	 * @details Accumulator holds one accumulator for each requested feature.
	 *          Samples are forwarded to all accumulators when Push() is called.
	 *          Results are extracted using get<Feature>() to access the accumulator.
	 *
	 *          The same API supports both weighted and unweighted accumulation:
	 *          - acc.Push(value)          -> unweighted sample
	 *          - acc.Push(value, weight)  -> weighted sample
	 */
	template<statistical_sample Sample, typename... Features>
		requires(sizeof...(Features) > 0) and (feature_tag<Features> and ...)
	class Accumulator
	{
	public:
		using SampleType = Sample;
		using features    = std::tuple<Features...>;

		static constexpr std::size_t FEATURE_COUNT = sizeof...(Features);

		// ========================================================================
		// Construction
		// ========================================================================

		constexpr Accumulator() noexcept = default;

		// ========================================================================
		// Accumulation
		// ========================================================================

		/**
		 * @brief Accumulate an unweighted sample
		 * @param value The sample value to accumulate
		 */
		constexpr void Push(Sample value) noexcept
		{
			std::apply(
				[value](auto&... accumulators)
				{
					(accumulators.Push(value), ...);
				},
				accumulators_);
		}

		/**
		 * @brief Accumulate a weighted sample
		 * @param value The sample value to accumulate
		 * @param weight The weight for this sample
		 */
		template<typename Weight = Sample>
		constexpr void Push(Sample value, Weight weight) noexcept
		{
			std::apply(
				[value, weight](auto&... accumulators)
				{
					(accumulators.Push(value, weight), ...);
				},
				accumulators_);
		}

		/**
		 * @brief Accumulate multiple samples from a range
		 * @param range A range of sample values
		 */
		template<std::ranges::input_range Range>
			requires std::convertible_to<std::ranges::range_value_t<Range>, Sample>
		constexpr void Accumulate(Range&& range)
		{
			for (auto&& value: range)
			{
				Push(static_cast<Sample>(value));
			}
		}

		/**
		 * @brief Accumulate multiple weighted samples from ranges
		 * @param values Range of sample values
		 * @param weights Range of weights (must be same size as values)
		 */
		template<std::ranges::input_range ValueRange, std::ranges::input_range WeightRange>
			requires std::convertible_to<std::ranges::range_value_t<ValueRange>, Sample>
		constexpr void Accumulate(ValueRange&& values, WeightRange&& weights)
		{
			auto valueIt  = std::ranges::begin(values);
			auto weightIt = std::ranges::begin(weights);

			while (valueIt != std::ranges::end(values) && weightIt != std::ranges::end(weights))
			{
				Push(*valueIt, *weightIt);
				++valueIt;
				++weightIt;
			}
		}

		// ========================================================================
		// Feature Access
		// ========================================================================

		/**
		 * @brief Get the accumulator for a specific feature
		 * @tparam Feature The feature tag type
		 * @return Reference to the accumulator
		 */
		template<typename Feature>
			requires detail::has_feature_v<Feature, Features...>
		[[nodiscard]] constexpr auto Get() noexcept -> auto&
		{
			return std::get<detail::feature_index_v<Feature, Features...>>(accumulators_);
		}

		/**
		 * @brief Get the accumulator for a specific feature (const)
		 */
		template<typename Feature>
			requires detail::has_feature_v<Feature, Features...>
		[[nodiscard]] constexpr auto Get() const noexcept -> const auto&
		{
			return std::get<detail::feature_index_v<Feature, Features...>>(accumulators_);
		}

		/**
		 * @brief Check if this set contains a specific feature
		 */
		template<typename Feature>
		[[nodiscard]] static constexpr auto Has() noexcept -> bool
		{
			return detail::has_feature_v<Feature, Features...>;
		}

		// ========================================================================
		// State Management
		// ========================================================================

		/**
		 * @brief Reset all accumulators to their initial state
		 */
		constexpr void Reset() noexcept
		{
			std::apply(
				[](auto&... accumulators)
				{
					(accumulators.Reset(), ...);
				},
				accumulators_);
		}

	private:
		std::tuple<typename Features::template AccumulatorType<Sample>...> accumulators_{};
	};

	// ============================================================================
	// Convenience Type Aliases
	// ============================================================================

	/**
	 * @brief Common accumulator for basic descriptive statistics
	 */
	template<statistical_sample Sample = double>
	using BasicStats = Accumulator<Sample, Count, Sum, Min, Max, Mean, Variance>;

	/**
	 * @brief Accumulator for distribution shape analysis
	 */
	template<statistical_sample Sample = double>
	using DistributionStats = Accumulator<Sample, Count, Mean, Variance, Skewness, Kurtosis>;

	/**
	 * @brief Rolling statistics with configurable window size
	 */
	template<std::size_t WindowSize, statistical_sample Sample = double>
	using RollingStats = Accumulator<Sample, RollingMean<WindowSize>, RollingVariance<WindowSize>>;

	/**
	 * @brief Statistics including median estimation
	 */
	template<statistical_sample Sample = double>
	using MedianStats = Accumulator<Sample, Count, Mean, Variance, Median>;

	/**
	 * @brief Statistics with quartiles (25th, 50th, 75th percentiles)
	 */
	template<statistical_sample Sample = double>
	using QuartileStats = Accumulator<Sample, Count, Min, Max, Percentile<25>, Percentile<50>, Percentile<75>>;

	/**
	 * @brief High-accuracy quartile statistics using 9 markers per percentile
	 */
	template<statistical_sample Sample = double>
	using QuartileStatsAccurate =
	Accumulator<Sample, Count, Min, Max, Percentile<25, 9>, Percentile<50, 9>, Percentile<75, 9>>;
}
