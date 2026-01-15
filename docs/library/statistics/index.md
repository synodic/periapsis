---
title: Statistics
description: Online statistical accumulators with composable features
---

# Statistics

A modern, constexpr-capable statistics accumulator framework inspired by Boost.Accumulators but redesigned for C++20/23.

## Overview

The statistics module provides:

- **Fully constexpr** — Compute statistics at compile-time
- **Online algorithms** — Numerically stable incremental computation
- **Composable features** — Mix and match statistical computations
- **Unified weighted API** — Same interface for weighted and unweighted samples
- **Rolling windows** — Efficient windowed statistics

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

---

## Accumulator

### Synopsis

```cpp
namespace peri::statistics {
    template<typename Sample, typename... Features>
    class Accumulator;
}
```

### Template Parameters

| Parameter | Description |
|-----------|-------------|
| `Sample` | The numeric type being accumulated |
| `Features...` | Statistical features to compute |

### Operations

| Method | Description |
|--------|-------------|
| `Push(sample)` | Add a sample |
| `Push(sample, weight)` | Add a weighted sample |
| `get<Feature>()` | Access a specific feature |
| `Reset()` | Clear all accumulated data |
| `Count()` | Number of samples |

---

## Features

### Basic Statistics

| Feature | Description | Result Type |
|---------|-------------|-------------|
| `count` | Number of samples | `std::size_t` |
| `sum` | Sum of all samples | `Sample` |
| `min` | Minimum value | `std::optional<Sample>` |
| `max` | Maximum value | `std::optional<Sample>` |

```cpp
Accumulator<double, count, sum, min, max> stats;
stats.Push(1.0);
stats.Push(5.0);
stats.Push(3.0);

auto n = stats.get<count>().result();   // 3
auto s = stats.get<sum>().result();     // 9.0
auto lo = stats.get<min>().result();    // 1.0
auto hi = stats.get<max>().result();    // 5.0
```

### Central Tendency & Dispersion

| Feature | Description | Algorithm |
|---------|-------------|-----------|
| `mean` | Arithmetic mean | Welford's online algorithm |
| `variance` | Sample variance | Welford's online algorithm |

The `variance` feature provides multiple accessors:

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

Compute raw moments:

```cpp
Accumulator<double, moment<1>, moment<2>, moment<3>> moments;
// moment<1> = mean, moment<2> = mean of squares, etc.
```

### Percentile / Quantile Estimation

Approximate percentile estimation using the Extended P² algorithm:

```cpp
// Median (50th percentile) with default 5 markers
Accumulator<double, median> med_stats;  // 'median' is alias for percentile<50>

// High-accuracy median with 9 markers
Accumulator<double, median_accurate> accurate_med;  // alias for percentile<50, 9>

// Multiple percentiles
Accumulator<double, percentile<25>, percentile<50>, percentile<75>> quartiles;

// Custom marker count for better accuracy (must be odd, >= 5)
Accumulator<double, percentile<95, 11>> high_accuracy_p95;
```

The P² algorithm:

- Computes quantiles without storing all observations
- Memory: O(markers) regardless of sample count
- Accuracy improves with more markers

---

## Feature Dependencies

Some features depend on others. Dependencies are automatically included:

| Feature | Depends On |
|---------|------------|
| `mean` | `count`, `sum` |
| `variance` | `count`, `mean` |
| `skewness` | `count`, `mean`, `variance` |
| `kurtosis` | `count`, `mean`, `variance` |

```cpp
// Only specify what you need - dependencies are automatic
Accumulator<double, variance> stats;  // Includes count and mean
```

---

## Weighted Samples

All features support weighted samples:

```cpp
Accumulator<double, mean, variance> stats;

stats.Push(1.0, 2.0);  // Value 1.0 with weight 2.0
stats.Push(3.0, 1.0);  // Value 3.0 with weight 1.0

auto m = stats.get<mean>().result();  // (1*2 + 3*1) / (2+1) = 5/3
```

---

## Constexpr Usage

Statistics can be computed at compile-time:

```cpp
constexpr auto compute_stats() {
    Accumulator<double, mean, variance> stats;
    stats.Push(1.0);
    stats.Push(2.0);
    stats.Push(3.0);
    stats.Push(4.0);
    stats.Push(5.0);
    return stats.get<mean>().result();
}

static_assert(compute_stats() == 3.0);
```

---

## Rolling Windows

Use `FixedRingBuffer` for rolling/windowed statistics:

```cpp
// Rolling mean over last 100 samples
Accumulator<double, mean> rolling;
FixedRingBuffer<double, 100> window;

void add_sample(double x) {
    if (window.full()) {
        // Remove oldest from accumulator
        rolling.Pop(window.front());
        window.pop_front();
    }
    window.push_back(x);
    rolling.Push(x);
}
```

---

## Numerical Stability

The accumulator uses numerically stable algorithms:

- **Welford's algorithm** for mean and variance — Avoids catastrophic cancellation
- **West's extension** for weighted samples
- **Two-pass compensation** for higher moments

This ensures accurate results even with:

- Large sample counts
- Values with large magnitudes
- Values with small differences

---

## Example: Full Analysis

```cpp
import synodic.periapsis;

using namespace peri::statistics;

// Comprehensive statistical analysis
Accumulator<double,
    count, sum, min, max,
    mean, variance,
    skewness, kurtosis,
    percentile<25>, percentile<50>, percentile<75>
> analysis;

// Accumulate data
for (double x : dataset) {
    analysis.Push(x);
}

// Report results
std::cout << "Count: " << analysis.get<count>().result() << "\n"
          << "Min: " << analysis.get<min>().result().value() << "\n"
          << "Max: " << analysis.get<max>().result().value() << "\n"
          << "Mean: " << analysis.get<mean>().result() << "\n"
          << "Std Dev: " << analysis.get<variance>().stdDev() << "\n"
          << "Skewness: " << analysis.get<skewness>().result() << "\n"
          << "Kurtosis: " << analysis.get<kurtosis>().result() << "\n"
          << "Q1: " << analysis.get<percentile<25>>().result() << "\n"
          << "Median: " << analysis.get<percentile<50>>().result() << "\n"
          << "Q3: " << analysis.get<percentile<75>>().result() << "\n";
```
