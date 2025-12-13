# Statistics

The Periapsis provides a modern, constexpr-capable statistics accumulator framework inspired by Boost.Accumulators but redesigned for C++20/23.

## Overview

The statistics module provides:

- **Fully constexpr** - compute statistics at compile-time
- **Online algorithms** - numerically stable incremental computation (Welford's, West's)
- **Composable features** - mix and match statistical computations
- **Unified weighted API** - same interface for weighted and unweighted samples
- **Rolling windows** - uses `FixedRingBuffer` for efficient windowed statistics

## Quick Start

```cpp
import synodic.periapsis;
using namespace peri::statistics;

// Basic usage
Accumulator<double, mean, variance, min, max> stats;
stats.Push(1.0);
stats.Push(2.0);
stats.Push(3.0);

auto m = stats.get<mean>().result();      // 2.0
auto v = stats.get<variance>().result();  // 1.0 (sample variance)
auto s = stats.get<variance>().stdDev();  // 1.0
```

## Features

### Basic Statistics

| Feature | Description | Result Type |
|---------|-------------|-------------|
| `count` | Number of samples | `std::size_t` |
| `sum` | Sum of all samples | `double` (or `Sample` if floating) |
| `min` | Minimum value | `std::optional<Sample>` |
| `max` | Maximum value | `std::optional<Sample>` |

### Central Tendency & Dispersion

| Feature | Description | Algorithm |
|---------|-------------|-----------|
| `mean` | Arithmetic mean | Welford's online algorithm |
| `variance` | Sample variance | Welford's online algorithm |

The `variance` accumulator provides multiple accessors:

```cpp
Accumulator<double, variance> stats;
// ... accumulate samples ...

stats.get<variance>().result();              // Sample variance (N-1)
stats.get<variance>().population_variance(); // Population variance (N)
stats.get<variance>().stdDev();              // Sample standard deviation
stats.get<variance>().population_stdDev();   // Population standard deviation
stats.get<variance>().mean();                // Also tracks mean internally
```

### Distribution Shape

| Feature | Description |
|---------|-------------|
| `skewness` | 3rd standardized moment (asymmetry) |
| `kurtosis` | Excess kurtosis (tailedness, normal = 0) |

```cpp
Accumulator<double, mean, variance, skewness, kurtosis> dist;
for (double x : data) dist.Push(x);

auto skew = stats.get<skewness>().result();  // Positive = right-tailed
auto kurt = stats.get<kurtosis>().result();  // >0 heavy tails, <0 light tails
```

### N-th Moment

```cpp
Accumulator<double, moment<1>, moment<2>, moment<3>> moments;
// moment<1> = mean, moment<2> = mean of squares, etc.
```

### Percentile / Quantile Estimation

The `percentile<N>` feature provides approximate percentile estimation using the Extended P² algorithm that computes quantiles without storing observations:

```cpp
// Median (50th percentile) with default 5 markers
Accumulator<double, median> med_stats;  // 'median' is alias for percentile<50>

// High-accuracy median with 9 markers
Accumulator<double, median_accurate> accurate_med;  // alias for percentile<50, 9>

// Multiple percentiles
Accumulator<double, percentile<25>, percentile<50>, percentile<75>> quartiles;

// Custom marker count for better accuracy (must be odd, >= 5)
Accumulator<double, percentile<95, 11>> high_accuracy_p95;

for (double x : streaming_data) {
    quartiles.Push(x);
}

auto q1 = quartiles.get<percentile<25>>().result();  // First quartile
auto q2 = quartiles.get<percentile<50>>().result();  // Median
auto q3 = quartiles.get<percentile<75>>().result();  // Third quartile

// Also available: min/max from the percentile accumulator
auto min_val = quartiles.get<percentile<50>>().minimum();
auto max_val = quartiles.get<percentile<50>>().maximum();
```

The Extended P² algorithm:
- **Memory**: O(MarkerCount) - constant regardless of observation count
- **Time**: O(MarkerCount) per observation
- **Accuracy**: Approximate; typically within 1-2% of true value after 100+ samples.
  More markers provide finer granularity but similar asymptotic accuracy.
- **Supports**: Any percentile from 0 to 100

**References**:
- Raj Jain and Imrich Chlamtac. 1985. The P2 algorithm for dynamic calculation of quantiles and histograms without storing observations. Commun. ACM 28, 10 (Oct. 1985), 1076–1085. https://doi.org/10.1145/4372.4378
- Kimmo E. E. Raatikainen. 1987. Simultaneous estimation of several percentiles. Simulation 49, 4 (October 1, 1987), 159–164. https://doi.org/10.1177/003754978704900405

### Rolling/Moving Statistics

Rolling statistics use `FixedRingBuffer` for O(1) updates:

```cpp
Accumulator<double, rolling_mean<10>, rolling_variance<10>> rolling;

for (double sample : streaming_data) {
    rolling.Push(sample);

    auto current_mean = rolling.get<rolling_mean<10>>().result();
    auto current_std  = rolling.get<rolling_variance<10>>().stdDev();

    if (rolling.get<rolling_mean<10>>().window_full()) {
        // Window is now fully populated
    }
}
```

## Weighted Statistics

The same API supports weighted samples—no separate type needed:

```cpp
Accumulator<double, mean, variance> stats;

// Unweighted
stats.Push(1.0);
stats.Push(2.0);

// Weighted (same accumulator)
stats.Push(3.0, 2.0);  // value=3.0, weight=2.0
stats.Push(4.0, 0.5);  // value=4.0, weight=0.5

// Results account for weights
auto weighted_mean = stats.get<mean>().result();
auto weighted_var  = stats.get<variance>().weighted_result();
```

## Query Functions

Free functions provide convenient access to results:

```cpp
#include <synodic/statistics/query.ixx>

Accumulator<double, count, sum, mean, variance, min, max> stats;
// ... accumulate ...

auto n = extract_count(stats);
auto s = extract_sum(stats);
auto m = extract_mean(stats);
auto v = extract_variance(stats);
auto sd = extract_stdDev(stats);
auto mn = extract_min(stats);   // std::optional
auto mx = extract_max(stats);   // std::optional

// Composite statistics
auto cv = coefficient_of_variation(stats);  // stdDev / mean
auto se = standard_error(stats);            // stdDev / sqrt(n)
auto r  = range(stats);                     // max - min
```

### Summary Snapshot

```cpp
auto s = summary(stats);
// Returns StatsSummary<double> with:
//   .count, .sum, .mean, .variance, .stdDev, .min, .max
```

## Convenience Type Aliases

```cpp
// Basic descriptive statistics
BasicStats<double> basic;  // count, sum, min, max, mean, variance

// Distribution shape analysis
DistributionStats<double> dist;  // count, mean, variance, skewness, kurtosis

// Rolling window statistics
RollingStats<10, double> rolling;  // rolling_mean<10>, rolling_variance<10>

// Statistics with median estimation (P² algorithm)
MedianStats<double> med;  // count, mean, variance, median

// Quartile statistics
QuartileStats<double> quart;  // count, min, max, percentile<25>, percentile<50>, percentile<75>
```

## Constexpr Usage

All accumulators are fully constexpr:

```cpp
constexpr auto compute_stats() {
    Accumulator<double, mean, variance> acc;
    acc.Push(1.0);
    acc.Push(2.0);
    acc.Push(3.0);
    return acc.get<mean>().result();
}

static_assert(compute_stats() == 2.0);
```

## Algorithm Details

### Welford's Online Algorithm (Mean & Variance)

The `mean` and `variance` features use Welford's numerically stable online algorithm:

```
For each new sample x:
    n++
    delta = x - mean
    mean += delta / n
    delta2 = x - mean
    M2 += delta * delta2

variance = M2 / (n - 1)  // sample variance
```

This avoids catastrophic cancellation that occurs with the naive formula `Var = E[X²] - E[X]²`.

### West's Weighted Algorithm

For weighted samples, West's extension of Welford's algorithm is used:

```
For each (value, weight) pair:
    weight_sum += weight
    delta = value - mean
    R = (weight / weight_sum) * delta
    mean += R
    M2 += (weight_sum - weight) * delta * R
```

## Integration with FixedRingBuffer

Rolling statistics leverage the library's `FixedRingBuffer<T, N>` for efficient O(1) windowed computation:

```cpp
// Rolling mean maintains:
// - FixedRingBuffer<Sample, WindowSize> for samples
// - Running sum for O(1) mean computation

// When buffer is full, oldest sample is removed before adding new one
```

## Concepts

The module defines several concepts for generic programming:

```cpp
template<typename T>
concept statistical_sample = std::is_arithmetic_v<T> && std::is_trivially_copyable_v<T>;

template<typename A, typename Sample>
concept basic_accumulator = requires(A& acc, const A& cacc, Sample s) {
    { acc.Push(s) } -> std::same_as<void>;
    { cacc.result() };
    { acc.reset() } -> std::same_as<void>;
};

template<typename A, typename Sample, typename Weight = Sample>
concept weighted_accumulator = basic_accumulator<A, Sample> &&
    requires(A& acc, Sample s, Weight w) {
        { acc.Push(s, w) } -> std::same_as<void>;
    };
```
