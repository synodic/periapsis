/**
 * @brief Unit tests for statistics accumulator types
 */

import std;
import synodic.honesty.test;
import synodic.library;

using namespace honesty::test;
using namespace honesty::test::literals;
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

	Suite COUNT_SUITE(
		"statistics_count",
		[]() -> Generator
		{
			co_yield "empty_count"_test = [](const Requirements& requirements)
			{
				constexpr Accumulator<double, count> acc;
				static_assert(acc.get<count>().result() == 0);
			};

			co_yield "single_sample"_test = [](const Requirements& requirements)
			{
				constexpr auto result = []
				{
					Accumulator<double, count> acc;
					acc.Push(1.0);
					return acc.get<count>().result();
				}();
				static_assert(result == 1);
			};

			co_yield "multiple_samples"_test = [](const Requirements& requirements)
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
			};

			co_yield "reset"_test = [](const Requirements& requirements)
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
			};
		});

	// ============================================================================
	// Sum Tests
	// ============================================================================

	Suite SUM_SUITE(
		"statistics_sum",
		[]() -> Generator
		{
			co_yield "empty_sum"_test = [](const Requirements& requirements)
			{
				constexpr Accumulator<double, sum> acc;
				static_assert(acc.get<sum>().result() == 0.0);
			};

			co_yield "single_sample"_test = [](const Requirements& requirements)
			{
				constexpr auto result = []
				{
					Accumulator<double, sum> acc;
					acc.Push(5.0);
					return acc.get<sum>().result();
				}();
				static_assert(result == 5.0);
			};

			co_yield "multiple_samples"_test = [](const Requirements& requirements)
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
			};

			co_yield "integer_samples"_test = [](const Requirements& requirements)
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
			};
		});

	// ============================================================================
	// Min/Max Tests
	// ============================================================================

	Suite MINMAX_SUITE(
		"statistics_minmax",
		[]() -> Generator
		{
			co_yield "empty_min"_test = [](const Requirements& requirements)
			{
				constexpr Accumulator<double, min> acc;
				static_assert(!acc.get<min>().result().has_value());
			};

			co_yield "empty_max"_test = [](const Requirements& requirements)
			{
				constexpr Accumulator<double, max> acc;
				static_assert(!acc.get<max>().result().has_value());
			};

			co_yield "single_sample_min"_test = [](const Requirements& requirements)
			{
				constexpr auto result = []
				{
					Accumulator<double, min> acc;
					acc.Push(5.0);
					return acc.get<min>().result();
				}();
				static_assert(result.has_value());
				static_assert(*result == 5.0);
			};

			co_yield "multiple_samples_min"_test = [](const Requirements& requirements)
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
			};

			co_yield "multiple_samples_max"_test = [](const Requirements& requirements)
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
			};

			co_yield "min_max_combined"_test = [](const Requirements& requirements)
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
			};

			co_yield "result_or_default"_test = [](const Requirements& requirements)
			{
				constexpr Accumulator<double, min> acc;
				static_assert(acc.get<min>().result_or(-1.0) == -1.0);
			};
		});

	// ============================================================================
	// Mean Tests
	// ============================================================================

	Suite MEAN_SUITE(
		"statistics_mean",
		[]() -> Generator
		{
			co_yield "empty_mean"_test = [](const Requirements& requirements)
			{
				constexpr Accumulator<double, mean> acc;
				static_assert(acc.get<mean>().result() == 0.0);
			};

			co_yield "single_sample"_test = [](const Requirements& requirements)
			{
				constexpr auto result = []
				{
					Accumulator<double, mean> acc;
					acc.Push(5.0);
					return acc.get<mean>().result();
				}();
				static_assert(result == 5.0);
			};

			co_yield "two_samples"_test = [](const Requirements& requirements)
			{
				constexpr auto result = []
				{
					Accumulator<double, mean> acc;
					acc.Push(2.0);
					acc.Push(4.0);
					return acc.get<mean>().result();
				}();
				static_assert(result == 3.0);
			};

			co_yield "multiple_samples"_test = [](const Requirements& requirements)
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
			};

			co_yield "integer_samples"_test = [](const Requirements& requirements)
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
			};
		});

	// ============================================================================
	// Variance Tests
	// ============================================================================

	Suite VARIANCE_SUITE(
		"statistics_variance",
		[]() -> Generator
		{
			co_yield "empty_variance"_test = [](const Requirements& requirements)
			{
				constexpr Accumulator<double, variance> acc;
				static_assert(acc.get<variance>().result() == 0.0);
			};

			co_yield "single_sample_variance"_test = [](const Requirements& requirements)
			{
				constexpr auto result = []
				{
					Accumulator<double, variance> acc;
					acc.Push(5.0);
					return acc.get<variance>().result();
				}();
				static_assert(result == 0.0);  // Variance of single sample is 0
			};

			co_yield "two_samples_variance"_test = [](const Requirements& requirements)
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
			};

			co_yield "population_variance"_test = [](const Requirements& requirements)
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
			};

			co_yield "known_dataset"_test = [](const Requirements& requirements)
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

				requirements.Expect(ApproxEqual(acc.get<variance>().mean(), 5.0));
				requirements.Expect(ApproxEqual(acc.get<variance>().population_variance(), 4.0));
				requirements.Expect(ApproxEqual(acc.get<variance>().result(), 32.0 / 7.0));	 // ~4.571
			};

			co_yield "stddev"_test = [](const Requirements& requirements)
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

				requirements.Expect(ApproxEqual(acc.get<variance>().population_stddev(), 2.0));
			};

			co_yield "variance_tracks_mean"_test = [](const Requirements& requirements)
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
			};
		});

	// ============================================================================
	// Composite Accumulator Tests
	// ============================================================================

	Suite COMPOSITE_SUITE(
		"statistics_composite",
		[]() -> Generator
		{
			co_yield "basic_stats"_test = [](const Requirements& requirements)
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
			};

			co_yield "has_feature"_test = [](const Requirements& requirements)
			{
				using Stats = Accumulator<double, count, mean>;
				static_assert(Stats::has<count>());
				static_assert(Stats::has<mean>());
				static_assert(!Stats::has<variance>());
				static_assert(!Stats::has<sum>());
			};

			co_yield "basic_stats_alias"_test = [](const Requirements& requirements)
			{
				BasicStats<double> stats;
				stats.Push(1.0);
				stats.Push(2.0);
				stats.Push(3.0);

				requirements.Expect(stats.get<count>().result() == 3);
				requirements.Expect(ApproxEqual(stats.get<mean>().result(), 2.0));
			};
		});

	// ============================================================================
	// Weighted Statistics Tests
	// ============================================================================

	Suite WEIGHTED_SUITE(
		"statistics_weighted",
		[]() -> Generator
		{
			co_yield "weighted_sum"_test = [](const Requirements& requirements)
			{
				Accumulator<double, sum> acc;
				acc.Push(2.0, 3.0);	// value=2, weight=3 -> contributes 6
				acc.Push(4.0, 2.0);	// value=4, weight=2 -> contributes 8

				requirements.Expect(ApproxEqual(acc.get<sum>().result(), 14.0));
			};

			co_yield "weighted_mean"_test = [](const Requirements& requirements)
			{
				Accumulator<double, mean> acc;
				// Weighted mean of (1, weight=2) and (3, weight=2) should be 2
				acc.Push(1.0, 2.0);
				acc.Push(3.0, 2.0);

				requirements.Expect(ApproxEqual(acc.get<mean>().result(), 2.0));
			};

			co_yield "mixed_weighted_unweighted"_test = [](const Requirements& requirements)
			{
				Accumulator<double, count> acc;
				acc.Push(1.0);  // unweighted
				acc.Push(2.0, 2.0);	// weighted

				requirements.Expect(acc.get<count>().result() == 2);
			};
		});

	// ============================================================================
	// Rolling Statistics Tests
	// ============================================================================

	Suite ROLLING_SUITE(
		"statistics_rolling",
		[]() -> Generator
		{
			co_yield "rolling_mean_partial_window"_test = [](const Requirements& requirements)
			{
				constexpr auto result = []
				{
					Accumulator<double, rolling_mean<5>> acc;
					acc.Push(2.0);
					acc.Push(4.0);
					return acc.get<rolling_mean<5>>().result();
				}();
				static_assert(result == 3.0);  // (2 + 4) / 2
			};

			co_yield "rolling_mean_full_window"_test = [](const Requirements& requirements)
			{
				constexpr auto result = []
				{
					Accumulator<double, rolling_mean<3>> acc;
					acc.Push(1.0);
					acc.Push(2.0);
					acc.Push(3.0);
					acc.Push(4.0);  // Pushes out 1.0
					acc.Push(5.0);  // Pushes out 2.0
					return acc.get<rolling_mean<3>>().result();
				}();
				// Window contains [3, 4, 5], mean = 4
				static_assert(result == 4.0);
			};

			co_yield "window_full_check"_test = [](const Requirements& requirements)
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
			};

			co_yield "rolling_variance"_test = [](const Requirements& requirements)
			{
				Accumulator<double, rolling_variance<4>> acc;
				acc.Push(2.0);
				acc.Push(4.0);
				acc.Push(4.0);
				acc.Push(4.0);

				// Window: [2, 4, 4, 4], mean = 3.5, variance = 1
				requirements.Expect(ApproxEqual(acc.get<rolling_variance<4>>().mean(), 3.5));
			};
		});

	// ============================================================================
	// Query Function Tests
	// ============================================================================

	Suite QUERY_SUITE(
		"statistics_query",
		[]() -> Generator
		{
			co_yield "extract_functions"_test = [](const Requirements& requirements)
			{
				Accumulator<double, count, sum, mean, variance, min, max> acc;
				acc.Push(1.0);
				acc.Push(2.0);
				acc.Push(3.0);

				requirements.Expect(extract_count(acc) == 3);
				requirements.Expect(ApproxEqual(extract_sum(acc), 6.0));
				requirements.Expect(ApproxEqual(extract_mean(acc), 2.0));
				requirements.Expect(ApproxEqual(extract_variance(acc), 1.0));
				requirements.Expect(extract_min(acc).has_value());
				requirements.Expect(*extract_min(acc) == 1.0);
				requirements.Expect(*extract_max(acc) == 3.0);
			};

			co_yield "stddev_extract"_test = [](const Requirements& requirements)
			{
				Accumulator<double, variance> acc;
				acc.Push(2.0);
				acc.Push(4.0);

				// Sample stddev of [2, 4] = sqrt(2) ≈ 1.414
				requirements.Expect(ApproxEqual(extract_stddev(acc), std::sqrt(2.0)));
			};

			co_yield "range_function"_test = [](const Requirements& requirements)
			{
				Accumulator<double, min, max> acc;
				acc.Push(5.0);
				acc.Push(2.0);
				acc.Push(8.0);

				auto r = range(acc);
				requirements.Expect(r.has_value());
				requirements.Expect(ApproxEqual(*r, 6.0));	// 8 - 2
			};

			co_yield "coefficient_of_variation"_test = [](const Requirements& requirements)
			{
				Accumulator<double, variance> acc;
				acc.Push(10.0);
				acc.Push(20.0);
				acc.Push(30.0);

				// Mean = 20, stddev ≈ 10, CV = 10/20 = 0.5
				auto cv = coefficient_of_variation(acc);
				requirements.Expect(ApproxEqual(cv, 0.5));
			};
		});

	// ============================================================================
	// Reset Tests
	// ============================================================================

	Suite RESET_SUITE(
		"statistics_reset",
		[]() -> Generator
		{
			co_yield "reset_all_features"_test = [](const Requirements& requirements)
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
			};

			co_yield "accumulate_after_reset"_test = [](const Requirements& requirements)
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
			};
		});

	// ============================================================================
	// Moment Tests
	// ============================================================================

	Suite MOMENT_SUITE(
		"statistics_moment",
		[]() -> Generator
		{
			co_yield "first_moment_equals_mean"_test = [](const Requirements& requirements)
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
			};

			co_yield "second_moment"_test = [](const Requirements& requirements)
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
			};
		});
}
