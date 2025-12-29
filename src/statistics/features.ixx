export module synodic.periapsis:statistics.features;

import :statistics.concepts;
import :container.fixed.ring_buffer;

import std;

export namespace peri::statistics
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
	struct Count
	{
		using Dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class AccumulatorType
		{
		public:
			using SampleType  = Sample;
			using ResultType  = std::size_t;
			using FeatureType = Count;

			constexpr AccumulatorType() noexcept = default;

			/**
			 * @brief Accumulate unweighted sample
			 */
			constexpr void Push(Sample) noexcept
			{
				++count_;
			}

			/**
			 * @brief Accumulate weighted sample (weight contributes to effective count)
			 */
			template<typename Weight = Sample>
			constexpr void Push(Sample, Weight weight) noexcept
			{
				weightSum_ += static_cast<double>(weight);
				++count_;
			}

			[[nodiscard]] constexpr auto Result() const noexcept -> ResultType
			{
				return count_;
			}

			/**
			 * @brief Effective count for weighted statistics
			 */
			[[nodiscard]] constexpr auto EffectiveCount() const noexcept -> double
			{
				return weightSum_ > 0.0 ? weightSum_ : static_cast<double>(count_);
			}

			constexpr void Reset() noexcept
			{
				count_	   = 0;
				weightSum_ = 0.0;
			}

		private:
			std::size_t count_ {};
			double weightSum_ {};
		};
	};

	// ============================================================================
	// Feature: Sum
	// ============================================================================

	/**
	 * @brief Feature tag for sum of samples
	 */
	struct Sum
	{
		using Dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class AccumulatorType
		{
		public:
			using SampleType  = Sample;
			using ResultType  = detail::default_result_t<Sample>;
			using FeatureType = Sum;

			constexpr AccumulatorType() noexcept = default;

			constexpr void Push(Sample value) noexcept
			{
				sum_ += static_cast<ResultType>(value);
			}

			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight weight) noexcept
			{
				sum_ += static_cast<ResultType>(value) * static_cast<ResultType>(weight);
			}

			[[nodiscard]] constexpr auto Result() const noexcept -> ResultType
			{
				return sum_;
			}

			constexpr void Reset() noexcept
			{
				sum_ = ResultType {};
			}

		private:
			ResultType sum_ {};
		};
	};

	// ============================================================================
	// Feature: Min
	// ============================================================================

	/**
	 * @brief Feature tag for minimum value
	 */
	struct Min
	{
		using Dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class AccumulatorType
		{
		public:
			using SampleType  = Sample;
			using ResultType  = Sample;
			using FeatureType = Min;

			constexpr AccumulatorType() noexcept = default;

			constexpr void Push(Sample value) noexcept
			{
				if (!hasValue_ || value < min_)
				{
					min_	  = value;
					hasValue_ = true;
				}
			}

			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight) noexcept
			{
				Push(value);  // Weight doesn't affect min
			}

			[[nodiscard]] constexpr auto Result() const noexcept -> std::optional<ResultType>
			{
				return hasValue_ ? std::optional {min_} : std::nullopt;
			}

			[[nodiscard]] constexpr auto ResultOr(ResultType defaultValue) const noexcept -> ResultType
			{
				return hasValue_ ? min_ : defaultValue;
			}

			constexpr void Reset() noexcept
			{
				min_	  = Sample {};
				hasValue_ = false;
			}

		private:
			Sample min_ {};
			bool hasValue_ {false};
		};
	};

	// ============================================================================
	// Feature: Max
	// ============================================================================

	/**
	 * @brief Feature tag for maximum value
	 */
	struct Max
	{
		using Dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class AccumulatorType
		{
		public:
			using SampleType  = Sample;
			using ResultType  = Sample;
			using FeatureType = Max;

			constexpr AccumulatorType() noexcept = default;

			constexpr void Push(Sample value) noexcept
			{
				if (!hasValue_ || value > max_)
				{
					max_	  = value;
					hasValue_ = true;
				}
			}

			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight) noexcept
			{
				Push(value);  // Weight doesn't affect max
			}

			[[nodiscard]] constexpr auto Result() const noexcept -> std::optional<ResultType>
			{
				return hasValue_ ? std::optional {max_} : std::nullopt;
			}

			[[nodiscard]] constexpr auto ResultOr(ResultType defaultValue) const noexcept -> ResultType
			{
				return hasValue_ ? max_ : defaultValue;
			}

			constexpr void Reset() noexcept
			{
				max_	  = Sample {};
				hasValue_ = false;
			}

		private:
			Sample max_ {};
			bool hasValue_ {false};
		};
	};

	// ============================================================================
	// Feature: Mean (using Welford's online algorithm)
	// ============================================================================

	/**
	 * @brief Feature tag for arithmetic mean
	 * @details Uses numerically stable incremental computation
	 */
	struct Mean
	{
		using Dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class AccumulatorType
		{
		public:
			using SampleType  = Sample;
			using ResultType  = detail::default_result_t<Sample>;
			using FeatureType = Mean;

			constexpr AccumulatorType() noexcept = default;

			/**
			 * @brief Welford's online mean update
			 */
			constexpr void Push(Sample value) noexcept
			{
				++count_;
				const auto delta = static_cast<ResultType>(value) - mean_;
				mean_ += delta / static_cast<ResultType>(count_);
			}

			/**
			 * @brief Weighted incremental mean (West's algorithm)
			 */
			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight weight) noexcept
			{
				const auto w = static_cast<ResultType>(weight);
				weightSum_ += w;
				const auto delta = static_cast<ResultType>(value) - mean_;
				mean_ += w / weightSum_ * delta;
			}

			[[nodiscard]] constexpr auto Result() const noexcept -> ResultType
			{
				return mean_;
			}

			[[nodiscard]] constexpr auto Count() const noexcept -> std::size_t
			{
				return count_;
			}

			[[nodiscard]] constexpr auto WeightSum() const noexcept -> ResultType
			{
				return weightSum_;
			}

			constexpr void Reset() noexcept
			{
				mean_	   = ResultType {};
				count_	   = 0;
				weightSum_ = ResultType {};
			}

		private:
			ResultType mean_ {};
			std::size_t count_ {};
			ResultType weightSum_ {};
		};
	};

	// ============================================================================
	// Feature: Variance (using Welford's online algorithm)
	// ============================================================================

	/**
	 * @brief Feature tag for sample variance
	 * @details Uses Welford's numerically stable online algorithm
	 */
	struct Variance
	{
		using Dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class AccumulatorType
		{
		public:
			using SampleType  = Sample;
			using ResultType  = detail::default_result_t<Sample>;
			using FeatureType = Variance;

			constexpr AccumulatorType() noexcept = default;

			/**
			 * @brief Welford's online variance update
			 */
			constexpr void Push(Sample value) noexcept
			{
				++count_;
				const auto x	 = static_cast<ResultType>(value);
				const auto delta = x - mean_;
				mean_ += delta / static_cast<ResultType>(count_);
				const auto delta2 = x - mean_;
				m2_ += delta * delta2;
			}

			/**
			 * @brief Weighted Welford's algorithm (West's algorithm)
			 */
			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight weight) noexcept
			{
				const auto w = static_cast<ResultType>(weight);
				const auto x = static_cast<ResultType>(value);

				const auto oldWeightSum = weightSum_;
				weightSum_ += w;

				const auto delta = x - mean_;
				const auto R	 = delta * w / weightSum_;
				mean_ += R;
				m2_ += oldWeightSum * delta * R;
			}

			/**
			 * @brief Sample variance (N-1 denominator)
			 */
			[[nodiscard]] constexpr auto Result() const noexcept -> ResultType
			{
				if (count_ < 2)
				{
					return ResultType {};
				}
				return m2_ / static_cast<ResultType>(count_ - 1);
			}

			/**
			 * @brief Population variance (N denominator)
			 */
			[[nodiscard]] constexpr auto PopulationVariance() const noexcept -> ResultType
			{
				if (count_ == 0)
				{
					return ResultType {};
				}
				return m2_ / static_cast<ResultType>(count_);
			}

			/**
			 * @brief Weighted variance
			 */
			[[nodiscard]] constexpr auto WeightedResult() const noexcept -> ResultType
			{
				if (weightSum_ == ResultType {})
				{
					return ResultType {};
				}
				return m2_ / weightSum_;
			}

			/**
			 * @brief Standard deviation
			 */
			[[nodiscard]] constexpr auto StdDev() const noexcept -> ResultType
			{
				return std::sqrt(Result());
			}

			/**
			 * @brief Population standard deviation
			 */
			[[nodiscard]] constexpr auto PopulationStdDev() const noexcept -> ResultType
			{
				return std::sqrt(PopulationVariance());
			}

			[[nodiscard]] constexpr auto Mean() const noexcept -> ResultType
			{
				return mean_;
			}

			[[nodiscard]] constexpr auto Count() const noexcept -> std::size_t
			{
				return count_;
			}

			constexpr void Reset() noexcept
			{
				mean_	   = ResultType {};
				m2_		   = ResultType {};
				count_	   = 0;
				weightSum_ = ResultType {};
			}

		private:
			ResultType mean_ {};
			ResultType m2_ {};	// Sum of squared deviations from mean
			std::size_t count_ {};
			ResultType weightSum_ {};
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
	struct Moment
	{
		using Dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class AccumulatorType
		{
		public:
			using SampleType  = Sample;
			using ResultType  = detail::default_result_t<Sample>;
			using FeatureType = Moment<N>;

			constexpr AccumulatorType() noexcept = default;

			constexpr void Push(Sample value) noexcept
			{
				++count_;
				const auto x	 = static_cast<ResultType>(value);
				ResultType power = ResultType {1};
				for (std::size_t i = 0; i < N; ++i)
				{
					power *= x;
				}
				sumOfPowers_ += power;
			}

			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight weight) noexcept
			{
				const auto w = static_cast<ResultType>(weight);
				weightSum_ += w;
				const auto x	 = static_cast<ResultType>(value);
				ResultType power = ResultType {1};
				for (std::size_t i = 0; i < N; ++i)
				{
					power *= x;
				}
				sumOfPowers_ += w * power;
			}

			[[nodiscard]] constexpr auto Result() const noexcept -> ResultType
			{
				if (count_ == 0)
				{
					return ResultType {};
				}
				return sumOfPowers_ / static_cast<ResultType>(count_);
			}

			[[nodiscard]] constexpr auto WeightedResult() const noexcept -> ResultType
			{
				if (weightSum_ == ResultType {})
				{
					return ResultType {};
				}
				return sumOfPowers_ / weightSum_;
			}

			constexpr void Reset() noexcept
			{
				sumOfPowers_ = ResultType {};
				count_		 = 0;
				weightSum_	 = ResultType {};
			}

		private:
			ResultType sumOfPowers_ {};
			std::size_t count_ {};
			ResultType weightSum_ {};
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
	struct RollingMean
	{
		using Dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class AccumulatorType
		{
		public:
			using SampleType						 = Sample;
			using ResultType						 = detail::default_result_t<Sample>;
			using FeatureType						 = RollingMean<WindowSize>;
			static constexpr std::size_t WINDOW_SIZE = WindowSize;

			constexpr AccumulatorType() noexcept = default;

			constexpr void Push(Sample value) noexcept
			{
				const auto x = static_cast<ResultType>(value);

				if (buffer_.full())
				{
					// Remove oldest value from sum
					sum_ -= static_cast<ResultType>(buffer_.front());
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

			[[nodiscard]] constexpr auto Result() const noexcept -> ResultType
			{
				if (buffer_.empty())
				{
					return ResultType {};
				}
				return sum_ / static_cast<ResultType>(buffer_.size());
			}

			[[nodiscard]] constexpr auto WindowFull() const noexcept -> bool
			{
				return buffer_.full();
			}

			[[nodiscard]] constexpr auto CurrentWindowSize() const noexcept -> std::size_t
			{
				return buffer_.size();
			}

			constexpr void Reset() noexcept
			{
				buffer_.clear();
				sum_ = ResultType {};
			}

		private:
			FixedRingBuffer<Sample, WindowSize> buffer_ {};
			ResultType sum_ {};
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
	struct RollingVariance
	{
		using Dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class AccumulatorType
		{
		public:
			using SampleType						 = Sample;
			using ResultType						 = detail::default_result_t<Sample>;
			using FeatureType						 = RollingVariance<WindowSize>;
			static constexpr std::size_t WINDOW_SIZE = WindowSize;

			constexpr AccumulatorType() noexcept = default;

			constexpr void Push(Sample value) noexcept
			{
				const auto x = static_cast<ResultType>(value);

				if (buffer_.full())
				{
					const auto removed = static_cast<ResultType>(buffer_.front());
					sumX_ -= removed;
					sumX2_ -= removed * removed;
					buffer_.pop_front();
				}

				buffer_.push_back(value);
				sumX_ += x;
				sumX2_ += x * x;
			}

			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight) noexcept
			{
				Push(value);
			}

			[[nodiscard]] constexpr auto Result() const noexcept -> ResultType
			{
				const auto n = buffer_.size();
				if (n < 2)
				{
					return ResultType {};
				}

				const auto mean		= sumX_ / static_cast<ResultType>(n);
				const auto variance = sumX2_ / static_cast<ResultType>(n) - mean * mean;
				return variance * static_cast<ResultType>(n) / static_cast<ResultType>(n - 1);
			}

			[[nodiscard]] constexpr auto WindowFull() const noexcept -> bool
			{
				return buffer_.full();
			}

			[[nodiscard]] constexpr auto CurrentWindowSize() const noexcept -> std::size_t
			{
				return buffer_.size();
			}

			constexpr void Reset() noexcept
			{
				buffer_.clear();
				sumX_  = ResultType {};
				sumX2_ = ResultType {};
			}

		private:
			FixedRingBuffer<Sample, WindowSize> buffer_ {};
			ResultType sumX_ {};
			ResultType sumX2_ {};
		};
	};

	// ============================================================================
	// Feature: Skewness
	// ============================================================================

	/**
	 * @brief Feature tag for Fisher-Pearson skewness coefficient
	 * @details Requires Mean and Variance features for computation
	 */
	struct Skewness
	{
		using Dependencies = std::tuple<Mean, Variance>;

		template<statistical_sample Sample>
		class AccumulatorType
		{
		public:
			using SampleType  = Sample;
			using ResultType  = detail::default_result_t<Sample>;
			using FeatureType = Skewness;

			constexpr AccumulatorType() noexcept = default;

			constexpr void Push(Sample value) noexcept
			{
				++count_;
				const auto x = static_cast<ResultType>(value);
				m1_ += x;
				m2_ += x * x;
				m3_ += x * x * x;
			}

			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight weight) noexcept
			{
				const auto w = static_cast<ResultType>(weight);
				const auto x = static_cast<ResultType>(value);
				weightSum_ += w;
				m1_ += w * x;
				m2_ += w * x * x;
				m3_ += w * x * x * x;
			}

			[[nodiscard]] constexpr auto Result() const noexcept -> ResultType
			{
				if (count_ < 3)
				{
					return ResultType {};
				}

				const auto n	= static_cast<ResultType>(count_);
				const auto mean = m1_ / n;
				const auto m2	= m2_ / n - mean * mean;
				const auto m3	= m3_ / n - 3.0 * mean * m2 - mean * mean * mean;

				if (m2 == ResultType {})
				{
					return ResultType {};
				}

				return m3 / std::pow(m2, ResultType {1.5});
			}

			[[nodiscard]] constexpr auto WeightedResult() const noexcept -> ResultType
			{
				if (weightSum_ == ResultType {})
				{
					return ResultType {};
				}

				const auto mean = m1_ / weightSum_;
				const auto m2	= m2_ / weightSum_ - mean * mean;
				const auto m3	= m3_ / weightSum_ - 3.0 * mean * m2 - mean * mean * mean;

				if (m2 == ResultType {})
				{
					return ResultType {};
				}

				return m3 / std::pow(m2, ResultType {1.5});
			}

			constexpr void Reset() noexcept
			{
				count_	   = 0;
				m1_		   = ResultType {};
				m2_		   = ResultType {};
				m3_		   = ResultType {};
				weightSum_ = ResultType {};
			}

		private:
			std::size_t count_ {};
			ResultType m1_ {};
			ResultType m2_ {};
			ResultType m3_ {};
			ResultType weightSum_ {};
		};
	};

	// ============================================================================
	// Feature: Kurtosis
	// ============================================================================

	/**
	 * @brief Feature tag for excess kurtosis (Fisher definition)
	 * @details Measures tailedness of distribution
	 */
	struct Kurtosis
	{
		using Dependencies = std::tuple<Mean, Variance>;

		template<statistical_sample Sample>
		class AccumulatorType
		{
		public:
			using SampleType  = Sample;
			using ResultType  = detail::default_result_t<Sample>;
			using FeatureType = Kurtosis;

			constexpr AccumulatorType() noexcept = default;

			constexpr void Push(Sample value) noexcept
			{
				++count_;
				const auto x = static_cast<ResultType>(value);
				m1_ += x;
				m2_ += x * x;
				m4_ += x * x * x * x;
			}

			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight weight) noexcept
			{
				const auto w = static_cast<ResultType>(weight);
				const auto x = static_cast<ResultType>(value);
				weightSum_ += w;
				m1_ += w * x;
				m2_ += w * x * x;
				m4_ += w * x * x * x * x;
			}

			[[nodiscard]] constexpr auto Result() const noexcept -> ResultType
			{
				if (count_ < 4)
				{
					return ResultType {};
				}

				const auto n	= static_cast<ResultType>(count_);
				const auto mean = m1_ / n;
				const auto m2	= m2_ / n - mean * mean;
				const auto m4 =
					m4_ / n - 4.0 * mean * (m2_ / n) + 6.0 * (mean * mean) * m2 - 3.0 * (mean * mean * mean * mean);

				if (m2 == ResultType {})
				{
					return ResultType {};
				}

				return m4 / (m2 * m2) - 3.0;
			}

			[[nodiscard]] constexpr auto WeightedResult() const noexcept -> ResultType
			{
				if (weightSum_ == ResultType {})
				{
					return ResultType {};
				}

				const auto mean = m1_ / weightSum_;
				const auto m2	= m2_ / weightSum_ - mean * mean;
				const auto m4	= m4_ / weightSum_ - 4.0 * mean * (m2_ / weightSum_) + 6.0 * (mean * mean) * m2 -
								3.0 * (mean * mean * mean * mean);

				if (m2 == ResultType {})
				{
					return ResultType {};
				}

				return m4 / (m2 * m2) - 3.0;
			}

			constexpr void Reset() noexcept
			{
				count_	   = 0;
				m1_		   = ResultType {};
				m2_		   = ResultType {};
				m4_		   = ResultType {};
				weightSum_ = ResultType {};
			}

		private:
			std::size_t count_ {};
			ResultType m1_ {};
			ResultType m2_ {};
			ResultType m4_ {};
			ResultType weightSum_ {};
		};
	};

	// ============================================================================
	// Feature: Median (Piecewise Parabolic Distribution - P^2 Algorithm)
	// ============================================================================

	/**
	 * @brief Feature tag for median estimation using P^2 algorithm
	 * @details Space-efficient median estimation without storing all samples
	 */
	struct Median
	{
		using Dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class AccumulatorType
		{
		public:
			using SampleType  = Sample;
			using ResultType  = detail::default_result_t<Sample>;
			using FeatureType = Median;

			constexpr AccumulatorType() noexcept = default;

			constexpr void Push(Sample value) noexcept;

			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight) noexcept
			{
				Push(value);
			}

			[[nodiscard]] constexpr auto Result() const noexcept -> ResultType;
			constexpr void Reset() noexcept;

		private:
			std::array<ResultType, 5> q_ {};   // Marker heights (quantile estimates)
			std::array<std::size_t, 5> n_ {};  // Marker positions
			std::array<double, 5> np_ {};	   // Desired marker positions
			static constexpr std::array<double, 5> dnp_ = {0.0, 0.25, 0.5, 0.75, 1.0};	// Desired increments
			std::size_t count_ {};
		};
	};

	// P^2 algorithm implementation for Median
	template<statistical_sample Sample>
	constexpr void Median::AccumulatorType<Sample>::Push(Sample value) noexcept
	{
		const auto x = static_cast<ResultType>(value);
		++count_;

		if (count_ <= 5)
		{
			// Initialization phase: collect first 5 samples
			q_[count_ - 1] = x;

			if (count_ == 5)
			{
				// Sort the initial markers
				std::sort(q_.begin(), q_.end());
				// Initialize marker positions
				for (std::size_t i = 0; i < 5; ++i)
				{
					n_[i]  = i + 1;
					np_[i] = static_cast<double>(i + 1);
				}
			}
		}
		else
		{
			// Main P^2 algorithm
			// Find cell k such that q[k-1] <= x < q[k]
			int k;
			if (x < q_[0])
			{
				q_[0] = x;
				k	  = 1;
			}
			else if (x >= q_[4])
			{
				q_[4] = x;
				k	  = 4;
			}
			else
			{
				k = 1;
				while (k < 4 && x >= q_[k])
				{
					++k;
				}
			}

			// Increment positions of markers k+1 through 5
			for (int i = k; i < 5; ++i)
			{
				++n_[i];
			}

			// Update desired positions
			for (std::size_t i = 0; i < 5; ++i)
			{
				np_[i] += dnp_[i];
			}

			// Adjust marker heights using P^2 formula
			for (std::size_t i = 1; i < 4; ++i)
			{
				const double d = np_[i] - static_cast<double>(n_[i]);

				if ((d >= 1.0 && static_cast<int>(n_[i + 1]) - static_cast<int>(n_[i]) > 1) ||
					(d <= -1.0 && static_cast<int>(n_[i - 1]) - static_cast<int>(n_[i]) < -1))
				{
					const int sign = d >= 0.0 ? 1 : -1;

					// Try parabolic formula
					const double ni	  = static_cast<double>(n_[i]);
					const double nim1 = static_cast<double>(n_[i - 1]);
					const double nip1 = static_cast<double>(n_[i + 1]);

					const double qp = q_[i] + sign / (nip1 - nim1) *
												  ((ni - nim1 + sign) * (q_[i + 1] - q_[i]) / (nip1 - ni) +
												   (nip1 - ni - sign) * (q_[i] - q_[i - 1]) / (ni - nim1));

					// Check if parabolic estimate is valid
					if (q_[i - 1] < qp && qp < q_[i + 1])
					{
						q_[i] = static_cast<ResultType>(qp);
					}
					else
					{
						// Use linear formula
						q_[i] = q_[i] + sign * (q_[i + sign] - q_[i]) / static_cast<ResultType>(n_[i + sign] - n_[i]);
					}

					n_[i] += sign;
				}
			}
		}
	}

	template<statistical_sample Sample>
	constexpr auto Median::AccumulatorType<Sample>::Result() const noexcept -> ResultType
	{
		if (count_ == 0)
		{
			return ResultType {};
		}

		if (count_ < 5)
		{
			// Not enough samples for P^2, return simple median
			std::array<ResultType, 5> sorted = q_;
			std::sort(sorted.begin(), sorted.begin() + count_);
			return sorted[count_ / 2];
		}

		return q_[2];  // Middle marker is the median estimate
	}

	template<statistical_sample Sample>
	constexpr void Median::AccumulatorType<Sample>::Reset() noexcept
	{
		q_.fill(ResultType {});
		n_.fill(0);
		np_.fill(0.0);
		count_ = 0;
	}

	// ============================================================================
	// Feature: Percentile (using P^2 algorithm)
	// ============================================================================

	/**
	 * @brief Feature tag for percentile estimation using P^2 algorithm
	 * @tparam P The percentile value (0-100)
	 * @tparam Markers Number of internal markers (must be >= 5 and odd)
	 */
	template<std::size_t P, std::size_t Markers = 5>
	struct Percentile
	{
		static_assert(P >= 0 && P <= 100, "Percentile must be between 0 and 100");
		static_assert(Markers >= 5, "Need at least 5 markers for P^2 algorithm");
		static_assert(Markers % 2 == 1, "Markers must be odd for symmetric quantile estimation");

		using Dependencies = std::tuple<>;

		template<statistical_sample Sample>
		class AccumulatorType
		{
		public:
			using SampleType  = Sample;
			using ResultType  = detail::default_result_t<Sample>;
			using FeatureType = Percentile<P, Markers>;

			static constexpr double TARGET_P = static_cast<double>(P) / 100.0;

			constexpr AccumulatorType() noexcept
			{
				InitializeDesiredIncrements();
			}

			constexpr void Push(Sample value) noexcept;

			template<typename Weight = Sample>
			constexpr void Push(Sample value, Weight) noexcept
			{
				Push(value);
			}

			[[nodiscard]] constexpr auto Result() const noexcept -> ResultType;
			constexpr void Reset() noexcept;

		private:
			constexpr void InitializeDesiredIncrements() noexcept
			{
				// Set up desired increments so markers converge to estimate the target percentile
				// For 5 markers estimating percentile p:
				// Marker 0: always tracks minimum (increment 0)
				// Marker 1: tracks p/2
				// Marker 2: tracks p (the target)
				// Marker 3: tracks (1+p)/2
				// Marker 4: always tracks maximum (increment 1)
				constexpr double p = TARGET_P;
				dnp_[0]			   = 0.0;
				dnp_[1]			   = p / 2.0;
				dnp_[2]			   = p;
				dnp_[3]			   = (1.0 + p) / 2.0;
				dnp_[4]			   = 1.0;
			}

			std::array<ResultType, Markers> q_ {};
			std::array<std::size_t, Markers> n_ {};
			std::array<double, Markers> np_ {};
			std::array<double, Markers> dnp_ {};
			std::size_t count_ {};
		};
	};

	template<std::size_t P, std::size_t Markers>
	template<statistical_sample Sample>
	constexpr void Percentile<P, Markers>::AccumulatorType<Sample>::Push(Sample value) noexcept
	{
		const auto x = static_cast<ResultType>(value);
		++count_;

		if (count_ <= Markers)
		{
			// Initialization phase
			q_[count_ - 1] = x;

			if (count_ == Markers)
			{
				std::sort(q_.begin(), q_.end());
				for (std::size_t i = 0; i < Markers; ++i)
				{
					n_[i] = i + 1;
					// Initialize desired positions based on the target percentile
					np_[i] = 1.0 + dnp_[i] * static_cast<double>(Markers - 1);
				}
			}
		}
		else
		{
			// Main P^2 algorithm
			int k;
			if (x < q_[0])
			{
				q_[0] = x;
				k	  = 1;
			}
			else if (x >= q_[Markers - 1])
			{
				q_[Markers - 1] = x;
				k				= static_cast<int>(Markers) - 1;
			}
			else
			{
				k = 1;
				while (k < static_cast<int>(Markers) - 1 && x >= q_[k])
				{
					++k;
				}
			}

			for (int i = k; i < static_cast<int>(Markers); ++i)
			{
				++n_[i];
			}

			for (std::size_t i = 0; i < Markers; ++i)
			{
				np_[i] += dnp_[i];
			}

			for (std::size_t i = 1; i < Markers - 1; ++i)
			{
				const double d = np_[i] - static_cast<double>(n_[i]);

				if ((d >= 1.0 && static_cast<int>(n_[i + 1]) - static_cast<int>(n_[i]) > 1) ||
					(d <= -1.0 && static_cast<int>(n_[i - 1]) - static_cast<int>(n_[i]) < -1))
				{
					const int sign = d >= 0.0 ? 1 : -1;

					const double ni	  = static_cast<double>(n_[i]);
					const double nim1 = static_cast<double>(n_[i - 1]);
					const double nip1 = static_cast<double>(n_[i + 1]);

					const double qp = q_[i] + sign / (nip1 - nim1) *
												  ((ni - nim1 + sign) * (q_[i + 1] - q_[i]) / (nip1 - ni) +
												   (nip1 - ni - sign) * (q_[i] - q_[i - 1]) / (ni - nim1));

					if (q_[i - 1] < qp && qp < q_[i + 1])
					{
						q_[i] = static_cast<ResultType>(qp);
					}
					else
					{
						q_[i] = q_[i] + sign * (q_[i + sign] - q_[i]) / static_cast<ResultType>(n_[i + sign] - n_[i]);
					}

					n_[i] += sign;
				}
			}
		}
	}

	template<std::size_t P, std::size_t Markers>
	template<statistical_sample Sample>
	constexpr auto Percentile<P, Markers>::AccumulatorType<Sample>::Result() const noexcept -> ResultType
	{
		if (count_ == 0)
		{
			return ResultType {};
		}

		if (count_ < Markers)
		{
			// Not enough samples, use linear interpolation on sorted data
			std::array<ResultType, Markers> sorted = q_;
			std::sort(sorted.begin(), sorted.begin() + count_);

			// Linear interpolation for percentile
			const double pos		= TARGET_P * static_cast<double>(count_ - 1);
			const std::size_t lower = static_cast<std::size_t>(pos);
			const std::size_t upper = lower + 1 < count_ ? lower + 1 : lower;
			const double frac		= pos - static_cast<double>(lower);

			return sorted[lower] + static_cast<ResultType>(frac) * (sorted[upper] - sorted[lower]);
		}

		// Handle edge cases: P=0 returns minimum, P=100 returns maximum
		if constexpr (P == 0)
		{
			return q_[0];
		}
		else if constexpr (P == 100)
		{
			return q_[Markers - 1];
		}
		else
		{
			// Return the middle marker which tracks the target percentile
			return q_[2];
		}
	}

	template<std::size_t P, std::size_t Markers>
	template<statistical_sample Sample>
	constexpr void Percentile<P, Markers>::AccumulatorType<Sample>::Reset() noexcept
	{
		q_.fill(ResultType {});
		n_.fill(0);
		np_.fill(0.0);
		InitializeDesiredIncrements();
		count_ = 0;
	}
}
