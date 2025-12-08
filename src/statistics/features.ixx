/**
 * @file features.ixx
 * @brief Statistical feature tags and their accumulator implementations
 *
 * @details This module defines the core statistical features:
 *          - count: Number of samples
 *          - sum: Sum of samples
 *          - min/max: Extreme values
 *          - mean: Arithmetic mean (lazy computation)
 *          - variance: Sample variance using Welford's algorithm
 *          - moment<N>: N-th central moment
 *
 *          Each feature is defined as a tag struct containing:
 *          - accumulator_type<Sample>: The accumulator implementation
 *          - dependencies (optional): Tuple of required feature tags
 *
 *          All accumulators support optional weights via a unified API.
 */

export module synodic.library:statistics.features;

import :statistics.concepts;
import :container.fixed.ring_buffer;

import std;

export namespace synodic::statistics
{
	// ============================================================================
	// Utility: Result Type Deduction
	// ============================================================================

	namespace detail
	{
		template<typename Sample>
		using default_result_t = std::conditional_t<std::floating_point<Sample>, Sample, double>;
	}

	// ============================================================================
	// Feature: Count
	// ============================================================================

	/**
	 * @brief Feature tag for sample count
	 */
	struct count
	{
		using dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class accumulator_type
		{
		public:
			using sample_type  = Sample;
			using result_type  = std::size_t;
			using feature_type = count;

			constexpr accumulator_type() noexcept = default;

			/// Accumulate unweighted sample
			constexpr void Push(Sample) noexcept
			{
				++count_;
			}

			/// Accumulate weighted sample (weight contributes to effective count)
			template<typename Weight = Sample>
			constexpr void Push(Sample, Weight weight) noexcept
			{
				weight_sum_ += static_cast<double>(weight);
				++count_;
			}

			[[nodiscard]] constexpr auto result() const noexcept -> result_type
			{
				return count_;
			}

			/// Effective count for weighted statistics
			[[nodiscard]] constexpr auto effective_count() const noexcept -> double
			{
				return weight_sum_ > 0.0 ? weight_sum_ : static_cast<double>(count_);
			}

			constexpr void reset() noexcept
			{
				count_		= 0;
				weight_sum_ = 0.0;
			}

		private:
			std::size_t count_ {};
			double weight_sum_ {};
		};
	};

	// ============================================================================
	// Feature: Sum
	// ============================================================================

	/**
	 * @brief Feature tag for sum of samples
	 */
	struct sum
	{
		using dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class accumulator_type
		{
		public:
			using sample_type  = Sample;
			using result_type  = detail::default_result_t<Sample>;
			using feature_type = sum;

			constexpr accumulator_type() noexcept = default;

			constexpr void Push(Sample value) noexcept
			{
				sum_ += static_cast<result_type>(value);
			}

			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight weight) noexcept
			{
				sum_ += static_cast<result_type>(value) * static_cast<result_type>(weight);
			}

			[[nodiscard]] constexpr auto result() const noexcept -> result_type
			{
				return sum_;
			}

			constexpr void reset() noexcept
			{
				sum_ = result_type {};
			}

		private:
			result_type sum_ {};
		};
	};

	// ============================================================================
	// Feature: Min
	// ============================================================================

	/**
	 * @brief Feature tag for minimum value
	 */
	struct min
	{
		using dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class accumulator_type
		{
		public:
			using sample_type  = Sample;
			using result_type  = Sample;
			using feature_type = min;

			constexpr accumulator_type() noexcept = default;

			constexpr void Push(Sample value) noexcept
			{
				if (!has_value_ || value < min_)
				{
					min_	   = value;
					has_value_ = true;
				}
			}

			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight) noexcept
			{
				Push(value);  // Weight doesn't affect min
			}

			[[nodiscard]] constexpr auto result() const noexcept -> std::optional<result_type>
			{
				return has_value_ ? std::optional {min_} : std::nullopt;
			}

			[[nodiscard]] constexpr auto result_or(result_type default_value) const noexcept -> result_type
			{
				return has_value_ ? min_ : default_value;
			}

			constexpr void reset() noexcept
			{
				min_	   = Sample {};
				has_value_ = false;
			}

		private:
			Sample min_ {};
			bool has_value_ {false};
		};
	};

	// ============================================================================
	// Feature: Max
	// ============================================================================

	/**
	 * @brief Feature tag for maximum value
	 */
	struct max
	{
		using dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class accumulator_type
		{
		public:
			using sample_type  = Sample;
			using result_type  = Sample;
			using feature_type = max;

			constexpr accumulator_type() noexcept = default;

			constexpr void Push(Sample value) noexcept
			{
				if (!has_value_ || value > max_)
				{
					max_	   = value;
					has_value_ = true;
				}
			}

			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight) noexcept
			{
				Push(value);  // Weight doesn't affect max
			}

			[[nodiscard]] constexpr auto result() const noexcept -> std::optional<result_type>
			{
				return has_value_ ? std::optional {max_} : std::nullopt;
			}

			[[nodiscard]] constexpr auto result_or(result_type default_value) const noexcept -> result_type
			{
				return has_value_ ? max_ : default_value;
			}

			constexpr void reset() noexcept
			{
				max_	   = Sample {};
				has_value_ = false;
			}

		private:
			Sample max_ {};
			bool has_value_ {false};
		};
	};

	// ============================================================================
	// Feature: Mean (using Welford's online algorithm)
	// ============================================================================

	/**
	 * @brief Feature tag for arithmetic mean
	 * @details Uses numerically stable incremental computation
	 */
	struct mean
	{
		using dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class accumulator_type
		{
		public:
			using sample_type  = Sample;
			using result_type  = detail::default_result_t<Sample>;
			using feature_type = mean;

			constexpr accumulator_type() noexcept = default;

			/// Welford's online mean update
			constexpr void Push(Sample value) noexcept
			{
				++count_;
				const auto delta  = static_cast<result_type>(value) - mean_;
				mean_			 += delta / static_cast<result_type>(count_);
			}

			/// Weighted incremental mean (West's algorithm)
			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight weight) noexcept
			{
				const auto w	  = static_cast<result_type>(weight);
				weight_sum_		 += w;
				const auto delta  = static_cast<result_type>(value) - mean_;
				mean_			 += (w / weight_sum_) * delta;
			}

			[[nodiscard]] constexpr auto result() const noexcept -> result_type
			{
				return mean_;
			}

			[[nodiscard]] constexpr auto count() const noexcept -> std::size_t
			{
				return count_;
			}

			[[nodiscard]] constexpr auto weight_sum() const noexcept -> result_type
			{
				return weight_sum_;
			}

			constexpr void reset() noexcept
			{
				mean_		= result_type {};
				count_		= 0;
				weight_sum_ = result_type {};
			}

		private:
			result_type mean_ {};
			std::size_t count_ {};
			result_type weight_sum_ {};
		};
	};

	// ============================================================================
	// Feature: Variance (using Welford's online algorithm)
	// ============================================================================

	/**
	 * @brief Feature tag for sample variance
	 * @details Uses Welford's numerically stable online algorithm
	 */
	struct variance
	{
		using dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class accumulator_type
		{
		public:
			using sample_type  = Sample;
			using result_type  = detail::default_result_t<Sample>;
			using feature_type = variance;

			constexpr accumulator_type() noexcept = default;

			/// Welford's online variance update
			constexpr void Push(Sample value) noexcept
			{
				++count_;
				const auto x	   = static_cast<result_type>(value);
				const auto delta   = x - mean_;
				mean_			  += delta / static_cast<result_type>(count_);
				const auto delta2  = x - mean_;
				m2_				  += delta * delta2;
			}

			/// Weighted Welford's algorithm (West's algorithm)
			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight weight) noexcept
			{
				const auto w = static_cast<result_type>(weight);
				const auto x = static_cast<result_type>(value);

				const auto old_weight_sum  = weight_sum_;
				weight_sum_				  += w;

				const auto delta  = x - mean_;
				const auto R	  = delta * w / weight_sum_;
				mean_			 += R;
				m2_				 += old_weight_sum * delta * R;
			}

			/// Sample variance (N-1 denominator)
			[[nodiscard]] constexpr auto result() const noexcept -> result_type
			{
				if (count_ < 2)
				{
					return result_type {};
				}
				return m2_ / static_cast<result_type>(count_ - 1);
			}

			/// Population variance (N denominator)
			[[nodiscard]] constexpr auto population_variance() const noexcept -> result_type
			{
				if (count_ == 0)
				{
					return result_type {};
				}
				return m2_ / static_cast<result_type>(count_);
			}

			/// Weighted variance
			[[nodiscard]] constexpr auto weighted_result() const noexcept -> result_type
			{
				if (weight_sum_ == result_type {})
				{
					return result_type {};
				}
				return m2_ / weight_sum_;
			}

			/// Standard deviation
			[[nodiscard]] constexpr auto stddev() const noexcept -> result_type
			{
				return std::sqrt(result());
			}

			/// Population standard deviation
			[[nodiscard]] constexpr auto population_stddev() const noexcept -> result_type
			{
				return std::sqrt(population_variance());
			}

			[[nodiscard]] constexpr auto mean() const noexcept -> result_type
			{
				return mean_;
			}

			[[nodiscard]] constexpr auto count() const noexcept -> std::size_t
			{
				return count_;
			}

			constexpr void reset() noexcept
			{
				mean_		= result_type {};
				m2_			= result_type {};
				count_		= 0;
				weight_sum_ = result_type {};
			}

		private:
			result_type mean_ {};
			result_type m2_ {};	 // Sum of squared deviations from mean
			std::size_t count_ {};
			result_type weight_sum_ {};
		};
	};

	// ============================================================================
	// Feature: N-th Moment
	// ============================================================================

	/**
	 * @brief Feature tag for N-th raw moment about the origin
	 * @tparam N The moment order (1 = mean, 2 = mean of squares, etc.)
	 */
	template<std::size_t N>
	struct moment
	{
		using dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class accumulator_type
		{
		public:
			using sample_type  = Sample;
			using result_type  = detail::default_result_t<Sample>;
			using feature_type = moment<N>;

			constexpr accumulator_type() noexcept = default;

			constexpr void Push(Sample value) noexcept
			{
				++count_;
				const auto x	  = static_cast<result_type>(value);
				result_type power = result_type {1};
				for (std::size_t i = 0; i < N; ++i)
				{
					power *= x;
				}
				sum_of_powers_ += power;
			}

			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight weight) noexcept
			{
				const auto w	   = static_cast<result_type>(weight);
				weight_sum_		  += w;
				const auto x	   = static_cast<result_type>(value);
				result_type power  = result_type {1};
				for (std::size_t i = 0; i < N; ++i)
				{
					power *= x;
				}
				sum_of_powers_ += w * power;
			}

			[[nodiscard]] constexpr auto result() const noexcept -> result_type
			{
				if (count_ == 0)
				{
					return result_type {};
				}
				return sum_of_powers_ / static_cast<result_type>(count_);
			}

			[[nodiscard]] constexpr auto weighted_result() const noexcept -> result_type
			{
				if (weight_sum_ == result_type {})
				{
					return result_type {};
				}
				return sum_of_powers_ / weight_sum_;
			}

			constexpr void reset() noexcept
			{
				sum_of_powers_ = result_type {};
				count_		   = 0;
				weight_sum_	   = result_type {};
			}

		private:
			result_type sum_of_powers_ {};
			std::size_t count_ {};
			result_type weight_sum_ {};
		};
	};

	// ============================================================================
	// Feature: Rolling Mean (using FixedRingBuffer)
	// ============================================================================

	/**
	 * @brief Feature tag for rolling/moving mean over a fixed window
	 * @tparam WindowSize Number of samples in the rolling window
	 */
	template<std::size_t WindowSize>
	struct rolling_mean
	{
		using dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class accumulator_type
		{
		public:
			using sample_type						 = Sample;
			using result_type						 = detail::default_result_t<Sample>;
			using feature_type						 = rolling_mean<WindowSize>;
			static constexpr std::size_t window_size = WindowSize;

			constexpr accumulator_type() noexcept = default;

			constexpr void Push(Sample value) noexcept
			{
				const auto x = static_cast<result_type>(value);

				if (buffer_.full())
				{
					// Remove oldest value from sum
					sum_ -= static_cast<result_type>(buffer_.front());
					buffer_.pop_front();
				}

				buffer_.push_back(value);
				sum_ += x;
			}

			// Note: Rolling statistics typically don't use weights
			// but we provide the interface for consistency
			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight) noexcept
			{
				Push(value);
			}

			[[nodiscard]] constexpr auto result() const noexcept -> result_type
			{
				if (buffer_.empty())
				{
					return result_type {};
				}
				return sum_ / static_cast<result_type>(buffer_.size());
			}

			[[nodiscard]] constexpr auto window_full() const noexcept -> bool
			{
				return buffer_.full();
			}

			[[nodiscard]] constexpr auto current_window_size() const noexcept -> std::size_t
			{
				return buffer_.size();
			}

			constexpr void reset() noexcept
			{
				buffer_.clear();
				sum_ = result_type {};
			}

		private:
			FixedRingBuffer<Sample, WindowSize> buffer_ {};
			result_type sum_ {};
		};
	};

	// ============================================================================
	// Feature: Rolling Variance
	// ============================================================================

	/**
	 * @brief Feature tag for rolling/moving variance over a fixed window
	 * @tparam WindowSize Number of samples in the rolling window
	 */
	template<std::size_t WindowSize>
	struct rolling_variance
	{
		using dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class accumulator_type
		{
		public:
			using sample_type						 = Sample;
			using result_type						 = detail::default_result_t<Sample>;
			using feature_type						 = rolling_variance<WindowSize>;
			static constexpr std::size_t window_size = WindowSize;

			constexpr accumulator_type() noexcept = default;

			constexpr void Push(Sample value) noexcept
			{
				const auto x = static_cast<result_type>(value);

				if (buffer_.full())
				{
					// Remove oldest value
					const auto old_value  = static_cast<result_type>(buffer_.front());
					sum_				 -= old_value;
					sum_sq_				 -= old_value * old_value;
					buffer_.pop_front();
				}

				buffer_.push_back(value);
				sum_	+= x;
				sum_sq_ += x * x;
			}

			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight) noexcept
			{
				Push(value);
			}

			[[nodiscard]] constexpr auto result() const noexcept -> result_type
			{
				const auto n = buffer_.size();
				if (n < 2)
				{
					return result_type {};
				}
				const auto n_r	= static_cast<result_type>(n);
				const auto mean = sum_ / n_r;
				// Var = E[X²] - E[X]²
				return (sum_sq_ / n_r - mean * mean) * n_r / (n_r - 1);
			}

			[[nodiscard]] constexpr auto mean() const noexcept -> result_type
			{
				if (buffer_.empty())
				{
					return result_type {};
				}
				return sum_ / static_cast<result_type>(buffer_.size());
			}

			[[nodiscard]] constexpr auto stddev() const noexcept -> result_type
			{
				return std::sqrt(result());
			}

			[[nodiscard]] constexpr auto window_full() const noexcept -> bool
			{
				return buffer_.full();
			}

			constexpr void reset() noexcept
			{
				buffer_.clear();
				sum_	= result_type {};
				sum_sq_ = result_type {};
			}

		private:
			FixedRingBuffer<Sample, WindowSize> buffer_ {};
			result_type sum_ {};
			result_type sum_sq_ {};
		};
	};

	// ============================================================================
	// Feature: Skewness
	// ============================================================================

	/**
	 * @brief Feature tag for skewness (3rd standardized moment)
	 * @details Measures asymmetry of the distribution
	 */
	struct skewness
	{
		using dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class accumulator_type
		{
		public:
			using sample_type  = Sample;
			using result_type  = detail::default_result_t<Sample>;
			using feature_type = skewness;

			constexpr accumulator_type() noexcept = default;

			/// Online algorithm for skewness (Terriberry's method)
			constexpr void Push(Sample value) noexcept
			{
				const auto n1 = count_;
				++count_;
				const auto n = count_;

				const auto x		= static_cast<result_type>(value);
				const auto delta	= x - mean_;
				const auto delta_n	= delta / static_cast<result_type>(n);
				const auto delta_n2 = delta_n * delta_n;
				const auto term1	= delta * delta_n * static_cast<result_type>(n1);

				mean_ += delta_n;
				m3_	  += term1 * delta_n * static_cast<result_type>(n - 2) - result_type {3} * delta_n * m2_;
				m2_	  += term1;
			}

			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight) noexcept
			{
				Push(value);  // Simplified: ignore weight for skewness
			}

			[[nodiscard]] constexpr auto result() const noexcept -> result_type
			{
				if (count_ < 3 || m2_ == result_type {})
				{
					return result_type {};
				}
				const auto n = static_cast<result_type>(count_);
				return (std::sqrt(n * (n - 1)) / (n - 2)) * m3_ / std::pow(m2_, result_type {1.5});
			}

			[[nodiscard]] constexpr auto mean() const noexcept -> result_type
			{
				return mean_;
			}

			constexpr void reset() noexcept
			{
				mean_  = result_type {};
				m2_	   = result_type {};
				m3_	   = result_type {};
				count_ = 0;
			}

		private:
			result_type mean_ {};
			result_type m2_ {};
			result_type m3_ {};
			std::size_t count_ {};
		};
	};

	// ============================================================================
	// Feature: Kurtosis
	// ============================================================================

	/**
	 * @brief Feature tag for excess kurtosis (4th standardized moment - 3)
	 * @details Measures "tailedness" of distribution. Normal distribution has kurtosis = 0.
	 */
	struct kurtosis
	{
		using dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class accumulator_type
		{
		public:
			using sample_type  = Sample;
			using result_type  = detail::default_result_t<Sample>;
			using feature_type = kurtosis;

			constexpr accumulator_type() noexcept = default;

			/// Online algorithm for kurtosis
			constexpr void Push(Sample value) noexcept
			{
				const auto n1 = count_;
				++count_;
				const auto n = count_;

				const auto x		= static_cast<result_type>(value);
				const auto delta	= x - mean_;
				const auto delta_n	= delta / static_cast<result_type>(n);
				const auto delta_n2 = delta_n * delta_n;
				const auto term1	= delta * delta_n * static_cast<result_type>(n1);

				mean_ += delta_n;
				m4_	  += term1 * delta_n2 *
						   (static_cast<result_type>(n * n) - result_type {3} * static_cast<result_type>(n) +
							result_type {3}) +
					   result_type {6} * delta_n2 * m2_ - result_type {4} * delta_n * m3_;
				m3_ += term1 * delta_n * static_cast<result_type>(n - 2) - result_type {3} * delta_n * m2_;
				m2_ += term1;
			}

			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight) noexcept
			{
				Push(value);
			}

			/// Excess kurtosis (normal distribution = 0)
			[[nodiscard]] constexpr auto result() const noexcept -> result_type
			{
				if (count_ < 4 || m2_ == result_type {})
				{
					return result_type {};
				}
				const auto n = static_cast<result_type>(count_);
				return ((n + 1) * n * m4_) / ((n - 1) * (n - 2) * (n - 3) * m2_ * m2_) -
					   result_type {3} * (n - 1) * (n - 1) / ((n - 2) * (n - 3));
			}

			[[nodiscard]] constexpr auto mean() const noexcept -> result_type
			{
				return mean_;
			}

			constexpr void reset() noexcept
			{
				mean_  = result_type {};
				m2_	   = result_type {};
				m3_	   = result_type {};
				m4_	   = result_type {};
				count_ = 0;
			}

		private:
			result_type mean_ {};
			result_type m2_ {};
			result_type m3_ {};
			result_type m4_ {};
			std::size_t count_ {};
		};
	};
}
