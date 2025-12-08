# Statistics

The Synodic Template Library provides a modern, constexpr-capable statistics accumulator framework inspired by Boost.Accumulators but redesigned for C++20/23.

## Overview

The statistics module provides:

- **Fully constexpr** - compute statistics at compile-time
- **Online algorithms** - numerically stable incremental computation (Welford's, West's)
- **Composable features** - mix and match statistical computations
- **Unified weighted API** - same interface for weighted and unweighted samples
- **Rolling windows** - uses `FixedRingBuffer` for efficient windowed statistics

## Quick Start

```cpp
import synodic.library;
using namespace synodic::statistics;

// Basic usage
Accumulator<double, mean, variance, min, max> stats;
stats(1.0);
stats(2.0);
stats(3.0);

auto m = stats.get<mean>().result();      // 2.0
auto v = stats.get<variance>().result();  // 1.0 (sample variance)
auto s = stats.get<variance>().stddev();  // 1.0
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
stats.get<variance>().stddev();              // Sample standard deviation
stats.get<variance>().population_stddev();   // Population standard deviation
stats.get<variance>().mean();                // Also tracks mean internally
```

### Distribution Shape

| Feature | Description |
|---------|-------------|
| `skewness` | 3rd standardized moment (asymmetry) |
| `kurtosis` | Excess kurtosis (tailedness, normal = 0) |

```cpp
Accumulator<double, mean, variance, skewness, kurtosis> dist;
for (double x : data) dist(x);

auto skew = stats.get<skewness>().result();  // Positive = right-tailed
auto kurt = stats.get<kurtosis>().result();  // >0 heavy tails, <0 light tails
```

### N-th Moment

```cpp
Accumulator<double, moment<1>, moment<2>, moment<3>> moments;
// moment<1> = mean, moment<2> = mean of squares, etc.
```

### Rolling/Moving Statistics

Rolling statistics use `FixedRingBuffer` for O(1) updates:

```cpp
Accumulator<double, rolling_mean<10>, rolling_variance<10>> rolling;

for (double sample : streaming_data) {
    rolling(sample);
    
    auto current_mean = rolling.get<rolling_mean<10>>().result();
    auto current_std  = rolling.get<rolling_variance<10>>().stddev();
    
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
stats(1.0);
stats(2.0);

// Weighted (same accumulator)
stats(3.0, 2.0);  // value=3.0, weight=2.0
stats(4.0, 0.5);  // value=4.0, weight=0.5

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
auto sd = extract_stddev(stats);
auto mn = extract_min(stats);   // std::optional
auto mx = extract_max(stats);   // std::optional

// Composite statistics
auto cv = coefficient_of_variation(stats);  // stddev / mean
auto se = standard_error(stats);            // stddev / sqrt(n)
auto r  = range(stats);                     // max - min
```

### Summary Snapshot

```cpp
auto s = summary(stats);
// Returns StatsSummary<double> with:
//   .count, .sum, .mean, .variance, .stddev, .min, .max
```

## Convenience Type Aliases

```cpp
// Basic descriptive statistics
BasicStats<double> basic;  // count, sum, min, max, mean, variance

// Distribution shape analysis
DistributionStats<double> dist;  // count, mean, variance, skewness, kurtosis

// Rolling window statistics
RollingStats<10, double> rolling;  // rolling_mean<10>, rolling_variance<10>
```

## Constexpr Usage

All accumulators are fully constexpr:

```cpp
constexpr auto compute_stats() {
    Accumulator<double, mean, variance> acc;
    acc(1.0);
    acc(2.0);
    acc(3.0);
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
    { acc(s) } -> std::same_as<void>;
    { cacc.result() };
    { acc.reset() } -> std::same_as<void>;
};

template<typename A, typename Sample, typename Weight = Sample>
concept weighted_accumulator = basic_accumulator<A, Sample> && 
    requires(A& acc, Sample s, Weight w) {
        { acc(s, w) } -> std::same_as<void>;
    };
```
