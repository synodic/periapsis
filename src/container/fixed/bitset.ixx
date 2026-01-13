export module synodic.periapsis:container.fixed.bitset;

import std;

export namespace peri::fixed
{
	/**
	 * @brief A fixed-size bitset with configurable block type and direct block access.
	 *
	 * @tparam N The number of bits in the bitset
	 * @tparam Block The unsigned integral type used for internal block representation
	 */
	template<std::size_t N, std::unsigned_integral Block = std::size_t>
	class Bitset
	{
	public:
		// ========================================================================
		// Type Definitions
		// ========================================================================

		using block_type      = Block;
		using size_type       = std::size_t;
		using difference_type = std::ptrdiff_t;

		static constexpr size_type BITS_PER_BLOCK = std::numeric_limits<Block>::digits;
		static constexpr size_type NUM_BLOCKS     = (N + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;
		static constexpr size_type NOT_VALID      = static_cast<size_type>(-1);

		// ========================================================================
		// Construction
		// ========================================================================

		constexpr Bitset() noexcept = default;

		constexpr explicit Bitset(unsigned long long value) noexcept
		{
			if constexpr (N > 0)
			{
				constexpr size_type ullBits    = std::numeric_limits<unsigned long long>::digits;
				constexpr size_type bitsToCopy = std::min(N, ullBits);

				if constexpr (BITS_PER_BLOCK >= ullBits)
				{
					blocks_[0] = static_cast<Block>(value);
				}
				else
				{
					size_type bitIndex   = 0;
					size_type blockIndex = 0;
					while (bitIndex < bitsToCopy && blockIndex < NUM_BLOCKS)
					{
						blocks_[blockIndex] = static_cast<Block>(value >> bitIndex);
						bitIndex            += BITS_PER_BLOCK;
						++blockIndex;
					}
				}
				ZeroUnusedBits();
			}
		}

		// ========================================================================
		// Capacity
		// ========================================================================

		[[nodiscard]] static constexpr size_type size() noexcept
		{
			return N;
		}

		[[nodiscard]] static constexpr size_type num_blocks() noexcept
		{
			return NUM_BLOCKS;
		}

		// ========================================================================
		// Element Access
		// ========================================================================

		[[nodiscard]] constexpr bool test(const size_type pos) const
		{
			if (pos >= N)
			{
				throw std::out_of_range("Bitset::test: position out of range");
			}
			return UncheckedTest(pos);
		}

		[[nodiscard]] constexpr bool operator[](const size_type pos) const noexcept
		{
			return UncheckedTest(pos);
		}

		// ========================================================================
		// Block Access
		// ========================================================================

		[[nodiscard]] constexpr Block block(const size_type blockIndex) const noexcept
		{
			return blocks_[blockIndex];
		}

		[[nodiscard]] constexpr Block& block(const size_type blockIndex) noexcept
		{
			return blocks_[blockIndex];
		}

		[[nodiscard]] constexpr const std::array<Block, NUM_BLOCKS>& blocks() const noexcept
		{
			return blocks_;
		}

		[[nodiscard]] constexpr std::array<Block, NUM_BLOCKS>& blocks() noexcept
		{
			return blocks_;
		}

		// ========================================================================
		// Modifiers
		// ========================================================================

		constexpr Bitset& set() noexcept
		{
			blocks_.fill(~Block{0});
			ZeroUnusedBits();
			return *this;
		}

		constexpr Bitset& set(const size_type pos, const bool value = true)
		{
			if (pos >= N)
			{
				throw std::out_of_range("Bitset::set: position out of range");
			}
			UncheckedSet(pos, value);
			return *this;
		}

		constexpr Bitset& reset() noexcept
		{
			blocks_.fill(Block{0});
			return *this;
		}

		constexpr Bitset& reset(const size_type pos)
		{
			if (pos >= N)
			{
				throw std::out_of_range("Bitset::reset: position out of range");
			}
			blocks_[BlockIndex(pos)] &= ~BitMask(pos);
			return *this;
		}

		constexpr Bitset& flip() noexcept
		{
			for (auto& block: blocks_)
			{
				block = ~block;
			}
			ZeroUnusedBits();
			return *this;
		}

		constexpr Bitset& flip(const size_type pos)
		{
			if (pos >= N)
			{
				throw std::out_of_range("Bitset::flip: position out of range");
			}
			blocks_[BlockIndex(pos)] ^= BitMask(pos);
			return *this;
		}

		// ========================================================================
		// Queries
		// ========================================================================

		[[nodiscard]] constexpr size_type count() const noexcept
		{
			size_type result = 0;
			for (const auto& block: blocks_)
			{
				result += static_cast<size_type>(std::popcount(block));
			}
			return result;
		}

		[[nodiscard]] constexpr bool all() const noexcept
		{
			if constexpr (N == 0)
			{
				return true;
			}
			else
			{
				for (size_type i = 0; i + 1 < NUM_BLOCKS; ++i)
				{
					if (blocks_[i] != ~Block{0})
					{
						return false;
					}
				}
				constexpr size_type extra = N % BITS_PER_BLOCK;
				constexpr Block mask      = extra == 0 ? ~Block{0} : (Block{1} << extra) - 1;
				return blocks_[NUM_BLOCKS - 1] == mask;
			}
		}

		[[nodiscard]] constexpr bool any() const noexcept
		{
			for (const auto& block: blocks_)
			{
				if (block != Block{0})
				{
					return true;
				}
			}
			return false;
		}

		[[nodiscard]] constexpr bool none() const noexcept
		{
			return !any();
		}

		// ========================================================================
		// Search Operations
		// ========================================================================

		[[nodiscard]] constexpr size_type find_first() const noexcept
		{
			for (size_type i = 0; i < NUM_BLOCKS; ++i)
			{
				if (blocks_[i] != Block{0})
				{
					const size_type bitPos = i * BITS_PER_BLOCK + static_cast<size_type>(std::countr_zero(blocks_[i]));
					return bitPos < N ? bitPos : NOT_VALID;
				}
			}
			return NOT_VALID;
		}

		[[nodiscard]] constexpr size_type find_next(size_type pos) const noexcept
		{
			if (pos >= N)
			{
				return NOT_VALID;
			}

			++pos;
			if (pos >= N)
			{
				return NOT_VALID;
			}

			size_type blockIndex = BlockIndex(pos);
			size_type bitIndex   = BitIndex(pos);

			Block masked = blocks_[blockIndex] >> bitIndex;
			if (masked != Block{0})
			{
				const size_type result = pos + static_cast<size_type>(std::countr_zero(masked));
				return result < N ? result : NOT_VALID;
			}

			for (++blockIndex; blockIndex < NUM_BLOCKS; ++blockIndex)
			{
				if (blocks_[blockIndex] != Block{0})
				{
					const size_type result =
						blockIndex * BITS_PER_BLOCK + static_cast<size_type>(std::countr_zero(blocks_[blockIndex]));
					return result < N ? result : NOT_VALID;
				}
			}

			return NOT_VALID;
		}

		// ========================================================================
		// Comparison
		// ========================================================================

		[[nodiscard]] constexpr bool operator==(const Bitset& other) const noexcept = default;

	private:
		// ========================================================================
		// Helpers
		// ========================================================================

		[[nodiscard]] static constexpr size_type BlockIndex(const size_type pos) noexcept
		{
			return pos / BITS_PER_BLOCK;
		}

		[[nodiscard]] static constexpr size_type BitIndex(const size_type pos) noexcept
		{
			return pos % BITS_PER_BLOCK;
		}

		[[nodiscard]] static constexpr Block BitMask(const size_type pos) noexcept
		{
			return Block{1} << BitIndex(pos);
		}

		[[nodiscard]] constexpr bool UncheckedTest(const size_type pos) const noexcept
		{
			return (blocks_[BlockIndex(pos)] & BitMask(pos)) != 0;
		}

		constexpr void UncheckedSet(const size_type pos, const bool value) noexcept
		{
			if (value)
			{
				blocks_[BlockIndex(pos)] |= BitMask(pos);
			}
			else
			{
				blocks_[BlockIndex(pos)] &= ~BitMask(pos);
			}
		}

		constexpr void ZeroUnusedBits() noexcept
		{
			if constexpr (N > 0)
			{
				constexpr size_type extra = N % BITS_PER_BLOCK;
				if constexpr (extra != 0)
				{
					blocks_[NUM_BLOCKS - 1] &= (Block{1} << extra) - 1;
				}
			}
		}

		std::array<Block, NUM_BLOCKS> blocks_{};
	};
}
