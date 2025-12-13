/**
 * @brief Unit tests for statistics accumulator types
 */

#include <gtest/gtest.h>

import std;
import synodic.periapsis;

using namespace peri::statistics;

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
		constexpr Accumulator<double, Count> acc;
		static_assert(acc.Get<Count>().Result() == 0);
	}

	TEST(StatisticsCount, SingleSample)
	{
		constexpr auto result = []
		{
			Accumulator<double, Count> acc;
			acc.Push(1.0);
			return acc.Get<Count>().Result();
		}();
		static_assert(result == 1);
	}

	TEST(StatisticsCount, MultipleSamples)
	{
		constexpr auto result = []
		{
			Accumulator<double, Count> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			return acc.Get<Count>().Result();
		}();
		static_assert(result == 3);
	}

	TEST(StatisticsCount, Reset)
	{
		constexpr auto result = []
		{
			Accumulator<double, Count> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Reset();
			return acc.Get<Count>().Result();
		}();
		static_assert(result == 0);
	}

	// ============================================================================
	// Sum Tests
	// ============================================================================

	TEST(StatisticsSum, EmptySum)
	{
		constexpr Accumulator<double, Sum> acc;
		static_assert(acc.Get<Sum>().Result() == 0.0);
	}

	TEST(StatisticsSum, SingleSample)
	{
		constexpr auto result = []
		{
			Accumulator<double, Sum> acc;
			acc.Push(5.0);
			return acc.Get<Sum>().Result();
		}();
		static_assert(result == 5.0);
	}

	TEST(StatisticsSum, MultipleSamples)
	{
		constexpr auto result = []
		{
			Accumulator<double, Sum> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			return acc.Get<Sum>().Result();
		}();
		static_assert(result == 6.0);
	}

	TEST(StatisticsSum, IntegerSamples)
	{
		constexpr auto result = []
		{
			Accumulator<int, Sum> acc;
			acc.Push(10);
			acc.Push(20);
			acc.Push(30);
			return acc.Get<Sum>().Result();
		}();
		static_assert(result == 60.0);
	}

	// ============================================================================
	// Min/Max Tests
	// ============================================================================

	TEST(StatisticsMinMax, EmptyMin)
	{
		constexpr Accumulator<double, Min> acc;
		static_assert(!acc.Get<Min>().Result().has_value());
	}

	TEST(StatisticsMinMax, EmptyMax)
	{
		constexpr Accumulator<double, Max> acc;
		static_assert(!acc.Get<Max>().Result().has_value());
	}

	TEST(StatisticsMinMax, SingleSampleMin)
	{
		constexpr auto result = []
		{
			Accumulator<double, Min> acc;
			acc.Push(5.0);
			return acc.Get<Min>().Result();
		}();
		static_assert(result.has_value());
		static_assert(*result == 5.0);
	}

	TEST(StatisticsMinMax, MultipleSamplesMin)
	{
		constexpr auto result = []
		{
			Accumulator<double, Min> acc;
			acc.Push(3.0);
			acc.Push(1.0);
			acc.Push(4.0);
			acc.Push(1.5);
			return acc.Get<Min>().Result();
		}();
		static_assert(*result == 1.0);
	}

	TEST(StatisticsMinMax, MultipleSamplesMax)
	{
		constexpr auto result = []
		{
			Accumulator<double, Max> acc;
			acc.Push(3.0);
			acc.Push(1.0);
			acc.Push(4.0);
			acc.Push(1.5);
			return acc.Get<Max>().Result();
		}();
		static_assert(*result == 4.0);
	}

	TEST(StatisticsMinMax, MinMaxCombined)
	{
		constexpr auto result = []
		{
			Accumulator<double, Min, Max> acc;
			acc.Push(5.0);
			acc.Push(2.0);
			acc.Push(8.0);
			acc.Push(1.0);
			return std::pair {*acc.Get<Min>().Result(), *acc.Get<Max>().Result()};
		}();
		static_assert(result.first == 1.0);
		static_assert(result.second == 8.0);
	}

	TEST(StatisticsMinMax, ResultOrDefault)
	{
		constexpr Accumulator<double, Min> acc;
		static_assert(acc.Get<Min>().ResultOr(-1.0) == -1.0);
	}

	// ============================================================================
	// Mean Tests
	// ============================================================================

	TEST(StatisticsMean, EmptyMean)
	{
		constexpr Accumulator<double, Mean> acc;
		static_assert(acc.Get<Mean>().Result() == 0.0);
	}

	TEST(StatisticsMean, SingleSample)
	{
		constexpr auto result = []
		{
			Accumulator<double, Mean> acc;
			acc.Push(5.0);
			return acc.Get<Mean>().Result();
		}();
		static_assert(result == 5.0);
	}

	TEST(StatisticsMean, TwoSamples)
	{
		constexpr auto result = []
		{
			Accumulator<double, Mean> acc;
			acc.Push(2.0);
			acc.Push(4.0);
			return acc.Get<Mean>().Result();
		}();
		static_assert(result == 3.0);
	}

	TEST(StatisticsMean, MultipleSamples)
	{
		constexpr auto result = []
		{
			Accumulator<double, Mean> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			acc.Push(4.0);
			acc.Push(5.0);
			return acc.Get<Mean>().Result();
		}();
		static_assert(result == 3.0);
	}

	TEST(StatisticsMean, IntegerSamples)
	{
		constexpr auto result = []
		{
			Accumulator<int, Mean> acc;
			acc.Push(2);
			acc.Push(4);
			acc.Push(6);
			return acc.Get<Mean>().Result();
		}();
		static_assert(result == 4.0);
	}

	// ============================================================================
	// Variance Tests
	// ============================================================================

	TEST(StatisticsVariance, EmptyVariance)
	{
		constexpr Accumulator<double, Variance> acc;
		static_assert(acc.Get<Variance>().Result() == 0.0);
	}

	TEST(StatisticsVariance, SingleSampleVariance)
	{
		constexpr auto result = []
		{
			Accumulator<double, Variance> acc;
			acc.Push(5.0);
			return acc.Get<Variance>().Result();
		}();
		static_assert(result == 0.0);  // Variance of single sample is 0
	}

	TEST(StatisticsVariance, TwoSamplesVariance)
	{
		constexpr auto result = []
		{
			Accumulator<double, Variance> acc;
			acc.Push(2.0);
			acc.Push(4.0);
			return acc.Get<Variance>().Result();
		}();
		// Sample variance: sum((x - mean)^2) / (n-1)
		// Mean = 3, deviations = [-1, 1], sum of squares = 2, variance = 2/1 = 2
		static_assert(result == 2.0);
	}

	TEST(StatisticsVariance, PopulationVariance)
	{
		constexpr auto result = []
		{
			Accumulator<double, Variance> acc;
			acc.Push(2.0);
			acc.Push(4.0);
			return acc.Get<Variance>().PopulationVariance();
		}();
		// Population variance: sum((x - mean)^2) / n = 2/2 = 1
		static_assert(result == 1.0);
	}

	TEST(StatisticsVariance, KnownDataset)
	{
		// Dataset: 2, 4, 4, 4, 5, 5, 7, 9
		// Mean = 5, Sample Variance = 4.571..., Population Variance = 4
		Accumulator<double, Variance> acc;
		acc.Push(2.0);
		acc.Push(4.0);
		acc.Push(4.0);
		acc.Push(4.0);
		acc.Push(5.0);
		acc.Push(5.0);
		acc.Push(7.0);
		acc.Push(9.0);

		EXPECT_TRUE(ApproxEqual(acc.Get<Variance>().Mean(), 5.0));
		EXPECT_TRUE(ApproxEqual(acc.Get<Variance>().PopulationVariance(), 4.0));
		EXPECT_TRUE(ApproxEqual(acc.Get<Variance>().Result(), 32.0 / 7.0));	 // ~4.571
	}

	TEST(StatisticsVariance, StdDev)
	{
		Accumulator<double, Variance> acc;
		acc.Push(2.0);
		acc.Push(4.0);
		acc.Push(4.0);
		acc.Push(4.0);
		acc.Push(5.0);
		acc.Push(5.0);
		acc.Push(7.0);
		acc.Push(9.0);

		EXPECT_TRUE(ApproxEqual(acc.Get<Variance>().PopulationStdDev(), 2.0));
	}

	TEST(StatisticsVariance, VarianceTracksMean)
	{
		constexpr auto result = []
		{
			Accumulator<double, Variance> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			return acc.Get<Variance>().Mean();
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
			Accumulator<double, Count, Sum, Mean, Min, Max> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			acc.Push(4.0);
			acc.Push(5.0);
			return std::tuple {
				acc.Get<Count>().Result(),
				acc.Get<Sum>().Result(),
				acc.Get<Mean>().Result(),
				*acc.Get<Min>().Result(),
				*acc.Get<Max>().Result()};
		}();
		static_assert(std::get<0>(result) == 5);
		static_assert(std::get<1>(result) == 15.0);
		static_assert(std::get<2>(result) == 3.0);
		static_assert(std::get<3>(result) == 1.0);
		static_assert(std::get<4>(result) == 5.0);
	}

	TEST(StatisticsComposite, HasFeature)
	{
		using Stats = Accumulator<double, Count, Mean>;
		static_assert(Stats::Has<Count>());
		static_assert(Stats::Has<Mean>());
		static_assert(!Stats::Has<Variance>());
		static_assert(!Stats::Has<Sum>());
	}

	TEST(StatisticsComposite, BasicStatsAlias)
	{
		BasicStats<double> stats;
		stats.Push(1.0);
		stats.Push(2.0);
		stats.Push(3.0);

		EXPECT_EQ(stats.Get<Count>().Result(), 3);
		EXPECT_TRUE(ApproxEqual(stats.Get<Mean>().Result(), 2.0));
	}

	// ============================================================================
	// Weighted Statistics Tests
	// ============================================================================

	TEST(StatisticsWeighted, WeightedSum)
	{
		Accumulator<double, Sum> acc;
		acc.Push(2.0, 3.0);	 // value=2, weight=3 -> contributes 6
		acc.Push(4.0, 2.0);	 // value=4, weight=2 -> contributes 8

		EXPECT_TRUE(ApproxEqual(acc.Get<Sum>().Result(), 14.0));
	}

	TEST(StatisticsWeighted, WeightedMean)
	{
		Accumulator<double, Mean> acc;
		// Weighted mean of (1, weight=2) and (3, weight=2) should be 2
		acc.Push(1.0, 2.0);
		acc.Push(3.0, 2.0);

		EXPECT_TRUE(ApproxEqual(acc.Get<Mean>().Result(), 2.0));
	}

	TEST(StatisticsWeighted, MixedWeightedUnweighted)
	{
		Accumulator<double, Count> acc;
		acc.Push(1.0);		 // unweighted
		acc.Push(2.0, 2.0);	 // weighted

		EXPECT_EQ(acc.Get<Count>().Result(), 2);
	}

	// ============================================================================
	// Rolling Statistics Tests
	// ============================================================================

	TEST(StatisticsRolling, RollingMeanPartialWindow)
	{
		constexpr auto result = []
		{
			Accumulator<double, RollingMean<5>> acc;
			acc.Push(2.0);
			acc.Push(4.0);
			return acc.Get<RollingMean<5>>().Result();
		}();
		static_assert(result == 3.0);  // (2 + 4) / 2
	}

	TEST(StatisticsRolling, RollingMeanFullWindow)
	{
		constexpr auto result = []
		{
			Accumulator<double, RollingMean<3>> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			acc.Push(4.0);	// Pushes out 1.0
			acc.Push(5.0);	// Pushes out 2.0
			return acc.Get<RollingMean<3>>().Result();
		}();
		// Window contains [3, 4, 5], mean = 4
		static_assert(result == 4.0);
	}

	TEST(StatisticsRolling, WindowFullCheck)
	{
		constexpr auto result = []
		{
			Accumulator<double, RollingMean<3>> acc;
			bool before_full = acc.Get<RollingMean<3>>().WindowFull();
			acc.Push(1.0);
			acc.Push(2.0);
			bool still_not_full = acc.Get<RollingMean<3>>().WindowFull();
			acc.Push(3.0);
			bool now_full = acc.Get<RollingMean<3>>().WindowFull();
			return std::tuple {before_full, still_not_full, now_full};
		}();
		static_assert(!std::get<0>(result));
		static_assert(!std::get<1>(result));
		static_assert(std::get<2>(result));
	}

	TEST(StatisticsRolling, RollingVariance)
	{
		Accumulator<double, RollingVariance<4>> acc;
		acc.Push(2.0);
		acc.Push(4.0);
		acc.Push(4.0);
		acc.Push(4.0);

		// Window: [2, 4, 4, 4], mean = 3.5
		// Population variance = ((2-3.5)² + (4-3.5)² + (4-3.5)² + (4-3.5)²)/4 = 0.75
		// Sample variance = 0.75 * 4/3 = 1.0
		EXPECT_TRUE(ApproxEqual(acc.Get<RollingVariance<4>>().Result(), 1.0, 0.1));
	}

	// ============================================================================
	// Query Function Tests
	// ============================================================================

	TEST(StatisticsQuery, ExtractFunctions)
	{
		Accumulator<double, Count, Sum, Mean, Variance, Min, Max> acc;
		acc.Push(1.0);
		acc.Push(2.0);
		acc.Push(3.0);

		EXPECT_EQ(ExtractCount(acc), 3);
		EXPECT_TRUE(ApproxEqual(ExtractSum(acc), 6.0));
		EXPECT_TRUE(ApproxEqual(ExtractMean(acc), 2.0));
		EXPECT_TRUE(ApproxEqual(ExtractVariance(acc), 1.0));
		EXPECT_TRUE(ExtractMin(acc).has_value());
		EXPECT_EQ(*ExtractMin(acc), 1.0);
		EXPECT_EQ(*ExtractMax(acc), 3.0);
	}

	TEST(StatisticsQuery, StdDevExtract)
	{
		Accumulator<double, Variance> acc;
		acc.Push(2.0);
		acc.Push(4.0);

		// Sample stdDev of [2, 4] = sqrt(2) ≈ 1.414
		EXPECT_TRUE(ApproxEqual(ExtractStdDev(acc), std::sqrt(2.0)));
	}

	TEST(StatisticsQuery, RangeFunction)
	{
		Accumulator<double, Min, Max> acc;
		acc.Push(5.0);
		acc.Push(2.0);
		acc.Push(8.0);

		auto r = Range(acc);
		EXPECT_TRUE(r.has_value());
		EXPECT_TRUE(ApproxEqual(*r, 6.0));	// 8 - 2
	}

	TEST(StatisticsQuery, CoefficientOfVariation)
	{
		Accumulator<double, Variance> acc;
		acc.Push(10.0);
		acc.Push(20.0);
		acc.Push(30.0);

		// Mean = 20, stdDev ≈ 10, CV = 10/20 = 0.5
		auto cv = CoefficientOfVariation(acc);
		EXPECT_TRUE(ApproxEqual(cv, 0.5));
	}

	// ============================================================================
	// Reset Tests
	// ============================================================================

	TEST(StatisticsReset, ResetAllFeatures)
	{
		constexpr auto result = []
		{
			Accumulator<double, Count, Sum, Mean, Min, Max> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			acc.Reset();

			return std::tuple {
				acc.Get<Count>().Result(),
				acc.Get<Sum>().Result(),
				acc.Get<Mean>().Result(),
				acc.Get<Min>().Result().has_value(),
				acc.Get<Max>().Result().has_value()};
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
			Accumulator<double, Mean> acc;
			acc.Push(10.0);
			acc.Push(20.0);
			acc.Reset();
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			return acc.Get<Mean>().Result();
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
			Accumulator<double, Moment<1>, Mean> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			return std::pair {acc.Get<Moment<1>>().Result(), acc.Get<Mean>().Result()};
		}();
		static_assert(result.first == result.second);
	}

	TEST(StatisticsMoment, SecondMoment)
	{
		constexpr auto result = []
		{
			Accumulator<double, Moment<2>> acc;
			acc.Push(1.0);
			acc.Push(2.0);
			acc.Push(3.0);
			return acc.Get<Moment<2>>().Result();
		}();
		// E[X²] = (1 + 4 + 9) / 3 = 14/3 ≈ 4.667
		static_assert(result == 14.0 / 3.0);
	}

	// ============================================================================
	// Percentile Tests (P² Algorithm)
	// ============================================================================

	TEST(StatisticsPercentile, EmptyPercentile)
	{
		constexpr Accumulator<double, Percentile<50>> acc;
		static_assert(acc.Get<Percentile<50>>().Result() == 0.0);
	}

	TEST(StatisticsPercentile, SingleSample)
	{
		constexpr auto result = []
		{
			Accumulator<double, Percentile<50>> acc;
			acc.Push(5.0);
			return acc.Get<Percentile<50>>().Result();
		}();
		static_assert(result == 5.0);
	}

	TEST(StatisticsPercentile, TwoSamples)
	{
		Accumulator<double, Percentile<50>> acc;
		acc.Push(2.0);
		acc.Push(8.0);
		const auto result = acc.Get<Percentile<50>>().Result();
		// Median of [2, 8] should be around 5.0 (P² is an approximation)
		EXPECT_TRUE(ApproxEqual(result, 5.0, 0.5));
	}

	TEST(StatisticsPercentile, FiveSamplesMedian)
	{
		Accumulator<double, Percentile<50>> acc;
		acc.Push(1.0);
		acc.Push(2.0);
		acc.Push(3.0);
		acc.Push(4.0);
		acc.Push(5.0);
		const auto result = acc.Get<Percentile<50>>().Result();
		// Median of [1,2,3,4,5] should be around 3.0 (P² is an approximation)
		EXPECT_TRUE(ApproxEqual(result, 3.0, 0.5));
	}

	TEST(StatisticsPercentile, MedianAlias)
	{
		Accumulator<double, Median> acc;
		acc.Push(1.0);
		acc.Push(2.0);
		acc.Push(3.0);
		acc.Push(4.0);
		acc.Push(5.0);
		const auto result = acc.Get<Median>().Result();
		// Median of [1,2,3,4,5] should be around 3.0 (P² is an approximation)
		EXPECT_TRUE(ApproxEqual(result, 3.0, 0.5));
	}

	TEST(StatisticsPercentile, LargerDataset)
	{
		// Test P² algorithm with more data points
		Accumulator<double, Percentile<50>> acc;

		// Push values 1-100
		for (int i = 1; i <= 100; ++i)
		{
			acc.Push(static_cast<double>(i));
		}

		// Median should be approximately 50.5
		const auto result = acc.Get<Percentile<50>>().Result();
		EXPECT_TRUE(ApproxEqual(result, 50.5, 1.0));  // Allow some error due to approximation
	}

	TEST(StatisticsPercentile, Percentile25)
	{
		Accumulator<double, Percentile<25>> acc;

		// Push values 1-100
		for (int i = 1; i <= 100; ++i)
		{
			acc.Push(static_cast<double>(i));
		}

		// 25th percentile should be approximately 25.75
		const auto result = acc.Get<Percentile<25>>().Result();
		EXPECT_TRUE(ApproxEqual(result, 25.75, 2.0));  // P² has some approximation error
	}

	TEST(StatisticsPercentile, Percentile75)
	{
		Accumulator<double, Percentile<75>> acc;

		// Push values 1-100
		for (int i = 1; i <= 100; ++i)
		{
			acc.Push(static_cast<double>(i));
		}

		// 75th percentile should be approximately 75.25
		const auto result = acc.Get<Percentile<75>>().Result();
		EXPECT_TRUE(ApproxEqual(result, 75.25, 2.0));
	}

	TEST(StatisticsPercentile, Percentile0)
	{
		Accumulator<double, Percentile<0>> acc;

		acc.Push(5.0);
		acc.Push(10.0);
		acc.Push(15.0);
		acc.Push(20.0);
		acc.Push(25.0);

		// 0th percentile is the minimum
		const auto result = acc.Get<Percentile<0>>().Result();
		EXPECT_TRUE(ApproxEqual(result, 5.0, 0.1));
	}

	TEST(StatisticsPercentile, Percentile100)
	{
		Accumulator<double, Percentile<100>> acc;

		acc.Push(5.0);
		acc.Push(10.0);
		acc.Push(15.0);
		acc.Push(20.0);
		acc.Push(25.0);

		// 100th percentile is the maximum
		const auto result = acc.Get<Percentile<100>>().Result();
		EXPECT_TRUE(ApproxEqual(result, 25.0, 0.1));
	}

	TEST(StatisticsPercentile, GaussianDistribution)
	{
		// Approximate test with samples that should give predictable percentiles
		// Using a simple deterministic sequence that mimics sorted data
		Accumulator<double, Percentile<10>, Percentile<50>, Percentile<90>> acc;

		// Push 1000 values: 0.001, 0.002, ..., 1.0
		for (int i = 1; i <= 1000; ++i)
		{
			acc.Push(static_cast<double>(i) / 1000.0);
		}

		const auto p10 = acc.Get<Percentile<10>>().Result();
		const auto p50 = acc.Get<Percentile<50>>().Result();
		const auto p90 = acc.Get<Percentile<90>>().Result();

		// P10 should be around 0.1, P50 around 0.5, P90 around 0.9
		EXPECT_TRUE(ApproxEqual(p10, 0.1, 0.02));
		EXPECT_TRUE(ApproxEqual(p50, 0.5, 0.02));
		EXPECT_TRUE(ApproxEqual(p90, 0.9, 0.02));
	}

	TEST(StatisticsPercentile, MedianStatsAlias)
	{
		MedianStats<double> stats;
		stats.Push(1.0);
		stats.Push(2.0);
		stats.Push(3.0);
		stats.Push(4.0);
		stats.Push(5.0);

		EXPECT_EQ(stats.Get<Count>().Result(), 5);
		EXPECT_TRUE(ApproxEqual(stats.Get<Mean>().Result(), 3.0));
		EXPECT_TRUE(ApproxEqual(stats.Get<Median>().Result(), 3.0));
	}

	TEST(StatisticsPercentile, QuartileStatsAlias)
	{
		QuartileStats<double> stats;

		// Push values 1-100
		for (int i = 1; i <= 100; ++i)
		{
			stats.Push(static_cast<double>(i));
		}

		EXPECT_EQ(stats.Get<Count>().Result(), 100);
		EXPECT_EQ(*stats.Get<Min>().Result(), 1.0);
		EXPECT_EQ(*stats.Get<Max>().Result(), 100.0);

		// Quartiles (with P² approximation tolerance)
		EXPECT_TRUE(ApproxEqual(stats.Get<Percentile<25>>().Result(), 25.75, 2.0));
		EXPECT_TRUE(ApproxEqual(stats.Get<Percentile<50>>().Result(), 50.5, 1.0));
		EXPECT_TRUE(ApproxEqual(stats.Get<Percentile<75>>().Result(), 75.25, 2.0));
	}

	TEST(StatisticsPercentile, ReversedInput)
	{
		// Test with data arriving in reverse order (worst case for some algorithms)
		Accumulator<double, Percentile<50>> acc;

		for (int i = 100; i >= 1; --i)
		{
			acc.Push(static_cast<double>(i));
		}

		const auto result = acc.Get<Percentile<50>>().Result();
		EXPECT_TRUE(ApproxEqual(result, 50.5, 1.5));
	}

	TEST(StatisticsPercentile, ConstantValues)
	{
		// All same values
		Accumulator<double, Percentile<50>> acc;
		for (int i = 0; i < 10; ++i)
		{
			acc.Push(42.0);
		}
		const auto result = acc.Get<Percentile<50>>().Result();
		EXPECT_TRUE(ApproxEqual(result, 42.0, 0.1));
	}
}
