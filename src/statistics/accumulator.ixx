/**
 * @file accumulator.ixx
 * @brief Composable statistical accumulator container
 *
 * @details The Accumulator is a compile-time composable container that
 *          aggregates multiple statistical accumulators. It provides:
 *          - Unified sample accumulation across all features
 *          - Type-safe feature extraction via get<Feature>()
 *          - Support for weighted samples through the same API
 *          - Full constexpr capability for compile-time statistics
 */

export module synodic.library:statistics.accumulator;

import :statistics.concepts;
import :statistics.features;

import std;

export namespace synodic::statistics
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
		struct feature_index;

		template<typename Target, typename First, typename... Rest>
		struct feature_index<Target, First, Rest...>
		{
			static constexpr std::size_t value =
				std::is_same_v<Target, First> ? 0 : 1 + feature_index<Target, Rest...>::value;
		};

		template<typename Target>
		struct feature_index<Target>
		{
			static constexpr std::size_t value = 0;	 // Will cause compile error if used
		};

		template<typename Target, typename... Features>
		inline constexpr std::size_t feature_index_v = feature_index<Target, Features...>::value;

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
		using sample_type = Sample;
		using features	  = std::tuple<Features...>;

		static constexpr std::size_t feature_count = sizeof...(Features);

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
				[value](auto&... accs)
				{
					(accs.Push(value), ...);
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
				[value, weight](auto&... accs)
				{
					(accs.Push(value, weight), ...);
				},
				accumulators_);
		}

		/**
		 * @brief Accumulate multiple samples from a range
		 * @param range A range of sample values
		 */
		template<std::ranges::input_range Range>
			requires std::convertible_to<std::ranges::range_value_t<Range>, Sample>
		constexpr void accumulate(Range&& range)
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
		constexpr void accumulate(ValueRange&& values, WeightRange&& weights)
		{
			auto value_it  = std::ranges::begin(values);
			auto weight_it = std::ranges::begin(weights);

			while (value_it != std::ranges::end(values) && weight_it != std::ranges::end(weights))
			{
				Push(*value_it, *weight_it);
				++value_it;
				++weight_it;
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
		[[nodiscard]] constexpr auto get() noexcept -> auto&
		{
			return std::get<detail::feature_index_v<Feature, Features...>>(accumulators_);
		}

		/**
		 * @brief Get the accumulator for a specific feature (const)
		 */
		template<typename Feature>
			requires detail::has_feature_v<Feature, Features...>
		[[nodiscard]] constexpr auto get() const noexcept -> const auto&
		{
			return std::get<detail::feature_index_v<Feature, Features...>>(accumulators_);
		}

		/**
		 * @brief Check if this set contains a specific feature
		 */
		template<typename Feature>
		[[nodiscard]] static constexpr auto has() noexcept -> bool
		{
			return detail::has_feature_v<Feature, Features...>;
		}

		// ========================================================================
		// State Management
		// ========================================================================

		/**
		 * @brief Reset all accumulators to their initial state
		 */
		constexpr void reset() noexcept
		{
			std::apply(
				[](auto&... accs)
				{
					(accs.reset(), ...);
				},
				accumulators_);
		}

	private:
		std::tuple<typename Features::template accumulator_type<Sample>...> accumulators_ {};
	};

	// ============================================================================
	// Convenience Type Aliases
	// ============================================================================

	/**
	 * @brief Common accumulator for basic descriptive statistics
	 */
	template<statistical_sample Sample = double>
	using BasicStats = Accumulator<Sample, count, sum, min, max, mean, variance>;

	/**
	 * @brief Accumulator for distribution shape analysis
	 */
	template<statistical_sample Sample = double>
	using DistributionStats = Accumulator<Sample, count, mean, variance, skewness, kurtosis>;

	/**
	 * @brief Rolling statistics with configurable window size
	 */
	template<std::size_t WindowSize, statistical_sample Sample = double>
	using RollingStats = Accumulator<Sample, rolling_mean<WindowSize>, rolling_variance<WindowSize>>;
}
