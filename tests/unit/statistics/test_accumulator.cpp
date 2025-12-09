/**
 * @brief Unit tests for statistics accumulator types
 */

#include <gtest/gtest.h>

import std;
import synodic.library;

using namespace synodic::statistics;

namespace
{
	// Helper for floating-point comparison
	template<std::floating_point T>
	bool ApproxEqual(T a, T b, T epsilon = T {0.0001})
	{
		return std::abs(a - b) < epsilon;
	}

	// ============================================================================
	// Count Tests
	// ============================================================================

	TEST(StatisticsCount, EmptyCount)
	{
		constexpr Accumulator<double, count> acc;
		static_assert(acc.get<count>().result() == 0);
	}

	TEST(StatisticsCount, SingleSample)
	{
		constexpr auto result = []
		{
			Accumulator<double, count> acc;
			acc.Push(1.0);
			return acc.get<count>().result();
		}();
		static_assert(result == 1);
	}

	TEST(StatisticsCount, MultipleSamples)
	{
		constexpr auto result = []
		{
			Accumulator<double, count> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			return acc.get<count>().result();
		}();
		static_assert(result == 3);
	}

	TEST(StatisticsCount, Reset)
	{
		constexpr auto result = []
		{
			Accumulator<double, count> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.reset();
			return acc.get<count>().result();
		}();
		static_assert(result == 0);
	}

	// ============================================================================
	// Sum Tests
	// ============================================================================

	TEST(StatisticsSum, EmptySum)
	{
		constexpr Accumulator<double, sum> acc;
		static_assert(acc.get<sum>().result() == 0.0);
	}

	TEST(StatisticsSum, SingleSample)
	{
		constexpr auto result = []
		{
			Accumulator<double, sum> acc;
			acc.Push(5.0);
			return acc.get<sum>().result();
		}();
		static_assert(result == 5.0);
	}

	TEST(StatisticsSum, MultipleSamples)
	{
		constexpr auto result = []
		{
			Accumulator<double, sum> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			return acc.get<sum>().result();
		}();
		static_assert(result == 6.0);
	}

	TEST(StatisticsSum, IntegerSamples)
	{
		constexpr auto result = []
		{
			Accumulator<int, sum> acc;
			acc.Push(10);
			acc.Push(20);
			acc.Push(30);
			return acc.get<sum>().result();
		}();
		static_assert(result == 60.0);
	}

	// ============================================================================
	// Min/Max Tests
	// ============================================================================

	TEST(StatisticsMinMax, EmptyMin)
	{
		constexpr Accumulator<double, min> acc;
		static_assert(!acc.get<min>().result().has_value());
	}

	TEST(StatisticsMinMax, EmptyMax)
	{
		constexpr Accumulator<double, max> acc;
		static_assert(!acc.get<max>().result().has_value());
	}

	TEST(StatisticsMinMax, SingleSampleMin)
	{
		constexpr auto result = []
		{
			Accumulator<double, min> acc;
			acc.Push(5.0);
			return acc.get<min>().result();
		}();
		static_assert(result.has_value());
		static_assert(*result == 5.0);
	}

	TEST(StatisticsMinMax, MultipleSamplesMin)
	{
		constexpr auto result = []
		{
			Accumulator<double, min> acc;
			acc.Push(3.0);
			acc.Push(1.0);
			acc.Push(4.0);
			acc.Push(1.5);
			return acc.get<min>().result();
		}();
		static_assert(*result == 1.0);
	}

	TEST(StatisticsMinMax, MultipleSamplesMax)
	{
		constexpr auto result = []
		{
			Accumulator<double, max> acc;
			acc.Push(3.0);
			acc.Push(1.0);
			acc.Push(4.0);
			acc.Push(1.5);
			return acc.get<max>().result();
		}();
		static_assert(*result == 4.0);
	}

	TEST(StatisticsMinMax, MinMaxCombined)
	{
		constexpr auto result = []
		{
			Accumulator<double, min, max> acc;
			acc.Push(5.0);
			acc.Push(2.0);
			acc.Push(8.0);
			acc.Push(1.0);
			return std::pair {*acc.get<min>().result(), *acc.get<max>().result()};
		}();
		static_assert(result.first == 1.0);
		static_assert(result.second == 8.0);
	}

	TEST(StatisticsMinMax, ResultOrDefault)
	{
		constexpr Accumulator<double, min> acc;
		static_assert(acc.get<min>().result_or(-1.0) == -1.0);
	}

	// ============================================================================
	// Mean Tests
	// ============================================================================

	TEST(StatisticsMean, EmptyMean)
	{
		constexpr Accumulator<double, mean> acc;
		static_assert(acc.get<mean>().result() == 0.0);
	}

	TEST(StatisticsMean, SingleSample)
	{
		constexpr auto result = []
		{
			Accumulator<double, mean> acc;
			acc.Push(5.0);
			return acc.get<mean>().result();
		}();
		static_assert(result == 5.0);
	}

	TEST(StatisticsMean, TwoSamples)
	{
		constexpr auto result = []
		{
			Accumulator<double, mean> acc;
			acc.Push(2.0);
			acc.Push(4.0);
			return acc.get<mean>().result();
		}();
		static_assert(result == 3.0);
	}

	TEST(StatisticsMean, MultipleSamples)
	{
		constexpr auto result = []
		{
			Accumulator<double, mean> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			acc.Push(4.0);
			acc.Push(5.0);
			return acc.get<mean>().result();
		}();
		static_assert(result == 3.0);
	}

	TEST(StatisticsMean, IntegerSamples)
	{
		constexpr auto result = []
		{
			Accumulator<int, mean> acc;
			acc.Push(2);
			acc.Push(4);
			acc.Push(6);
			return acc.get<mean>().result();
		}();
		static_assert(result == 4.0);
	}

	// ============================================================================
	// Variance Tests
	// ============================================================================

	TEST(StatisticsVariance, EmptyVariance)
	{
		constexpr Accumulator<double, variance> acc;
		static_assert(acc.get<variance>().result() == 0.0);
	}

	TEST(StatisticsVariance, SingleSampleVariance)
	{
		constexpr auto result = []
		{
			Accumulator<double, variance> acc;
			acc.Push(5.0);
			return acc.get<variance>().result();
		}();
		static_assert(result == 0.0);  // Variance of single sample is 0
	}

	TEST(StatisticsVariance, TwoSamplesVariance)
	{
		constexpr auto result = []
		{
			Accumulator<double, variance> acc;
			acc.Push(2.0);
			acc.Push(4.0);
			return acc.get<variance>().result();
		}();
		// Sample variance: sum((x - mean)^2) / (n-1)
		// Mean = 3, deviations = [-1, 1], sum of squares = 2, variance = 2/1 = 2
		static_assert(result == 2.0);
	}

	TEST(StatisticsVariance, PopulationVariance)
	{
		constexpr auto result = []
		{
			Accumulator<double, variance> acc;
			acc.Push(2.0);
			acc.Push(4.0);
			return acc.get<variance>().population_variance();
		}();
		// Population variance: sum((x - mean)^2) / n = 2/2 = 1
		static_assert(result == 1.0);
	}

	TEST(StatisticsVariance, KnownDataset)
	{
		// Dataset: 2, 4, 4, 4, 5, 5, 7, 9
		// Mean = 5, Sample Variance = 4.571..., Population Variance = 4
		Accumulator<double, variance> acc;
		acc.Push(2.0);
		acc.Push(4.0);
		acc.Push(4.0);
		acc.Push(4.0);
		acc.Push(5.0);
		acc.Push(5.0);
		acc.Push(7.0);
		acc.Push(9.0);

		EXPECT_TRUE(ApproxEqual(acc.get<variance>().mean(), 5.0));
		EXPECT_TRUE(ApproxEqual(acc.get<variance>().population_variance(), 4.0));
		EXPECT_TRUE(ApproxEqual(acc.get<variance>().result(), 32.0 / 7.0));	 // ~4.571
	}

	TEST(StatisticsVariance, Stddev)
	{
		Accumulator<double, variance> acc;
		acc.Push(2.0);
		acc.Push(4.0);
		acc.Push(4.0);
		acc.Push(4.0);
		acc.Push(5.0);
		acc.Push(5.0);
		acc.Push(7.0);
		acc.Push(9.0);

		EXPECT_TRUE(ApproxEqual(acc.get<variance>().population_stddev(), 2.0));
	}

	TEST(StatisticsVariance, VarianceTracksMean)
	{
		constexpr auto result = []
		{
			Accumulator<double, variance> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			return acc.get<variance>().mean();
		}();
		static_assert(result == 2.0);
	}

	// ============================================================================
	// Composite Accumulator Tests
	// ============================================================================

	TEST(StatisticsComposite, BasicStats)
	{
		constexpr auto result = []
		{
			Accumulator<double, count, sum, mean, min, max> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			acc.Push(4.0);
			acc.Push(5.0);
			return std::tuple {
				acc.get<count>().result(),
				acc.get<sum>().result(),
				acc.get<mean>().result(),
				*acc.get<min>().result(),
				*acc.get<max>().result()};
		}();
		static_assert(std::get<0>(result) == 5);
		static_assert(std::get<1>(result) == 15.0);
		static_assert(std::get<2>(result) == 3.0);
		static_assert(std::get<3>(result) == 1.0);
		static_assert(std::get<4>(result) == 5.0);
	}

	TEST(StatisticsComposite, HasFeature)
	{
		using Stats = Accumulator<double, count, mean>;
		static_assert(Stats::has<count>());
		static_assert(Stats::has<mean>());
		static_assert(!Stats::has<variance>());
		static_assert(!Stats::has<sum>());
	}

	TEST(StatisticsComposite, BasicStatsAlias)
	{
		BasicStats<double> stats;
		stats.Push(1.0);
		stats.Push(2.0);
		stats.Push(3.0);

		EXPECT_EQ(stats.get<count>().result(), 3);
		EXPECT_TRUE(ApproxEqual(stats.get<mean>().result(), 2.0));
	}

	// ============================================================================
	// Weighted Statistics Tests
	// ============================================================================

	TEST(StatisticsWeighted, WeightedSum)
	{
		Accumulator<double, sum> acc;
		acc.Push(2.0, 3.0);	 // value=2, weight=3 -> contributes 6
		acc.Push(4.0, 2.0);	 // value=4, weight=2 -> contributes 8

		EXPECT_TRUE(ApproxEqual(acc.get<sum>().result(), 14.0));
	}

	TEST(StatisticsWeighted, WeightedMean)
	{
		Accumulator<double, mean> acc;
		// Weighted mean of (1, weight=2) and (3, weight=2) should be 2
		acc.Push(1.0, 2.0);
		acc.Push(3.0, 2.0);

		EXPECT_TRUE(ApproxEqual(acc.get<mean>().result(), 2.0));
	}

	TEST(StatisticsWeighted, MixedWeightedUnweighted)
	{
		Accumulator<double, count> acc;
		acc.Push(1.0);	// unweighted
		acc.Push(2.0, 2.0);	 // weighted

		EXPECT_EQ(acc.get<count>().result(), 2);
	}

	// ============================================================================
	// Rolling Statistics Tests
	// ============================================================================

	TEST(StatisticsRolling, RollingMeanPartialWindow)
	{
		constexpr auto result = []
		{
			Accumulator<double, rolling_mean<5>> acc;
			acc.Push(2.0);
			acc.Push(4.0);
			return acc.get<rolling_mean<5>>().result();
		}();
		static_assert(result == 3.0);  // (2 + 4) / 2
	}

	TEST(StatisticsRolling, RollingMeanFullWindow)
	{
		constexpr auto result = []
		{
			Accumulator<double, rolling_mean<3>> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			acc.Push(4.0);	// Pushes out 1.0
			acc.Push(5.0);	// Pushes out 2.0
			return acc.get<rolling_mean<3>>().result();
		}();
		// Window contains [3, 4, 5], mean = 4
		static_assert(result == 4.0);
	}

	TEST(StatisticsRolling, WindowFullCheck)
	{
		constexpr auto result = []
		{
			Accumulator<double, rolling_mean<3>> acc;
			bool before_full = acc.get<rolling_mean<3>>().window_full();
			acc.Push(1.0);
			acc.Push(2.0);
			bool still_not_full = acc.get<rolling_mean<3>>().window_full();
			acc.Push(3.0);
			bool now_full = acc.get<rolling_mean<3>>().window_full();
			return std::tuple {before_full, still_not_full, now_full};
		}();
		static_assert(!std::get<0>(result));
		static_assert(!std::get<1>(result));
		static_assert(std::get<2>(result));
	}

	TEST(StatisticsRolling, RollingVariance)
	{
		Accumulator<double, rolling_variance<4>> acc;
		acc.Push(2.0);
		acc.Push(4.0);
		acc.Push(4.0);
		acc.Push(4.0);

		// Window: [2, 4, 4, 4], mean = 3.5, variance = 1
		EXPECT_TRUE(ApproxEqual(acc.get<rolling_variance<4>>().mean(), 3.5));
	}

	// ============================================================================
	// Query Function Tests
	// ============================================================================

	TEST(StatisticsQuery, ExtractFunctions)
	{
		Accumulator<double, count, sum, mean, variance, min, max> acc;
		acc.Push(1.0);
		acc.Push(2.0);
		acc.Push(3.0);

		EXPECT_EQ(extract_count(acc), 3);
		EXPECT_TRUE(ApproxEqual(extract_sum(acc), 6.0));
		EXPECT_TRUE(ApproxEqual(extract_mean(acc), 2.0));
		EXPECT_TRUE(ApproxEqual(extract_variance(acc), 1.0));
		EXPECT_TRUE(extract_min(acc).has_value());
		EXPECT_EQ(*extract_min(acc), 1.0);
		EXPECT_EQ(*extract_max(acc), 3.0);
	}

	TEST(StatisticsQuery, StddevExtract)
	{
		Accumulator<double, variance> acc;
		acc.Push(2.0);
		acc.Push(4.0);

		// Sample stddev of [2, 4] = sqrt(2) ≈ 1.414
		EXPECT_TRUE(ApproxEqual(extract_stddev(acc), std::sqrt(2.0)));
	}

	TEST(StatisticsQuery, RangeFunction)
	{
		Accumulator<double, min, max> acc;
		acc.Push(5.0);
		acc.Push(2.0);
		acc.Push(8.0);

		auto r = range(acc);
		EXPECT_TRUE(r.has_value());
		EXPECT_TRUE(ApproxEqual(*r, 6.0));	// 8 - 2
	}

	TEST(StatisticsQuery, CoefficientOfVariation)
	{
		Accumulator<double, variance> acc;
		acc.Push(10.0);
		acc.Push(20.0);
		acc.Push(30.0);

		// Mean = 20, stddev ≈ 10, CV = 10/20 = 0.5
		auto cv = coefficient_of_variation(acc);
		EXPECT_TRUE(ApproxEqual(cv, 0.5));
	}

	// ============================================================================
	// Reset Tests
	// ============================================================================

	TEST(StatisticsReset, ResetAllFeatures)
	{
		constexpr auto result = []
		{
			Accumulator<double, count, sum, mean, min, max> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			acc.reset();

			return std::tuple {
				acc.get<count>().result(),
				acc.get<sum>().result(),
				acc.get<mean>().result(),
				acc.get<min>().result().has_value(),
				acc.get<max>().result().has_value()};
		}();

		static_assert(std::get<0>(result) == 0);
		static_assert(std::get<1>(result) == 0.0);
		static_assert(std::get<2>(result) == 0.0);
		static_assert(!std::get<3>(result));
		static_assert(!std::get<4>(result));
	}

	TEST(StatisticsReset, AccumulateAfterReset)
	{
		constexpr auto result = []
		{
			Accumulator<double, mean> acc;
			acc.Push(10.0);
			acc.Push(20.0);
			acc.reset();
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			return acc.get<mean>().result();
		}();
		static_assert(result == 2.0);
	}

	// ============================================================================
	// Moment Tests
	// ============================================================================

	TEST(StatisticsMoment, FirstMomentEqualsMean)
	{
		constexpr auto result = []
		{
			Accumulator<double, moment<1>, mean> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			return std::pair {acc.get<moment<1>>().result(), acc.get<mean>().result()};
		}();
		static_assert(result.first == result.second);
	}

	TEST(StatisticsMoment, SecondMoment)
	{
		constexpr auto result = []
		{
			Accumulator<double, moment<2>> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			return acc.get<moment<2>>().result();
		}();
		// E[X²] = (1 + 4 + 9) / 3 = 14/3 ≈ 4.667
		static_assert(result == 14.0 / 3.0);
	}

}  // namespace
