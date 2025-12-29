export module synodic.periapsis:container.sparse.bit_vector;

import std;
import :structure.tree.fenwick;
import :container.dynamic.bitset;
import :container.fixed.bitset;

export namespace peri::sparse
{
	// ========================================================================
	// SetBitIterator - Iterator over set bits in a BitVector
	// ========================================================================

	/**
	 * @brief Forward iterator over set bit positions in a BitVectorBase.
	 *
	 * @details Efficiently iterates only over set bits, skipping zeros.
	 *          Uses find_first()/find_next() for O(1) amortized iteration per set bit.
	 *
	 * @tparam BitVectorType The BitVectorBase type to iterate over
	 */
	template<typename BitVectorType>
	class SetBitIterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type		= std::size_t;
		using difference_type	= std::ptrdiff_t;
		using pointer			= const value_type*;
		using reference			= value_type;

		static constexpr value_type NOT_VALID = static_cast<value_type>(-1);

		/**
		 * @brief Constructs an iterator pointing to a specific position.
		 */
		constexpr SetBitIterator(const BitVectorType* bv, value_type pos) noexcept :
			bv_(bv),
			pos_(pos)
		{
		}

		/**
		 * @brief Default constructor creates an end iterator.
		 */
		constexpr SetBitIterator() noexcept :
			bv_(nullptr),
			pos_(NOT_VALID)
		{
		}

		[[nodiscard]] constexpr reference operator*() const noexcept
		{
			return pos_;
		}

		constexpr SetBitIterator& operator++() noexcept
		{
			if (bv_ != nullptr && pos_ != NOT_VALID)
			{
				pos_ = bv_->find_next(pos_);
			}
			return *this;
		}

		constexpr SetBitIterator operator++(int) noexcept
		{
			SetBitIterator tmp = *this;
			++*this;
			return tmp;
		}

		[[nodiscard]] constexpr bool operator==(const SetBitIterator& other) const noexcept
		{
			return pos_ == other.pos_;
		}

		[[nodiscard]] constexpr bool operator!=(const SetBitIterator& other) const noexcept
		{
			return pos_ != other.pos_;
		}

	private:
		const BitVectorType* bv_;
		value_type pos_;
	};

	// ========================================================================
	// BitVector - Sparse bit vector with O(log n) rank/select
	// ========================================================================

	/**
	 * @brief A sparse bit vector combining a bitset with a Fenwick tree for fast prefix sum queries.
	 *
	 * @details Provides O(log n) rank and select operations by maintaining a Fenwick tree over block popcounts.
	 *
	 * @tparam Bitset The underlying bitset type
	 * @tparam FenwickContainer The container type for the Fenwick tree
	 */
	template<typename Bitset, typename FenwickContainer>
	class BitVectorBase
	{
	public:
		// ========================================================================
		// Type Definitions
		// ========================================================================

		using bitset_type	   = Bitset;
		using size_type		   = std::size_t;
		using difference_type  = std::ptrdiff_t;
		using block_type	   = Bitset::block_type;
		using set_bit_iterator = SetBitIterator<BitVectorBase>;

		static constexpr size_type BITS_PER_BLOCK = Bitset::BITS_PER_BLOCK;
		static constexpr size_type NOT_VALID	  = static_cast<size_type>(-1);

		// ========================================================================
		// Construction
		// ========================================================================

		/**
		 * @brief Constructs a BitVectorBase with given number of bits.
		 *
		 * @param numBits Number of bits to allocate
		 */
		explicit constexpr BitVectorBase(const size_type numBits)
			requires requires {
				Bitset(numBits);
				FenwickContainer(std::declval<size_type>(), size_type {});
			}
			:
			bits_(numBits),
			fenwick_(NumBlocks(numBits))
		{
		}

		/**
		 * @brief Constructs a BitVectorBase from an existing bitset.
		 *
		 * @param bitset The bitset to use
		 */
		explicit constexpr BitVectorBase(Bitset bitset)
			requires requires { FenwickContainer(std::declval<size_type>(), size_type {}); }
			:
			bits_(std::move(bitset)),
			fenwick_(bits_.num_blocks())
		{
			RebuildFenwick();
		}

		/**
		 * @brief Default constructs a BitVectorBase for fixed-size bitsets with array-backed Fenwick.
		 */
		constexpr BitVectorBase()
			requires std::default_initializable<Bitset> && std::default_initializable<FenwickContainer>
			:
			bits_(),
			fenwick_()
		{
		}

		// ========================================================================
		// Capacity
		// ========================================================================

		[[nodiscard]] constexpr size_type size() const noexcept
		{
			return bits_.size();
		}

		[[nodiscard]] constexpr bool empty() const noexcept
		{
			return size() == 0;
		}

		[[nodiscard]] constexpr size_type num_blocks() const noexcept
		{
			return bits_.num_blocks();
		}

		// ========================================================================
		// Element Access
		// ========================================================================

		/**
		 * @brief Tests the bit at the given position.
		 *
		 * @param pos Position to test
		 * @return true if the bit is set, false otherwise
		 */
		[[nodiscard]] constexpr bool test(const size_type pos) const
		{
			return bits_.test(pos);
		}

		[[nodiscard]] constexpr bool operator[](const size_type pos) const noexcept
		{
			return bits_[pos];
		}

		// ========================================================================
		// Modifiers
		// ========================================================================

		/**
		 * @brief Sets the bit at the given position.
		 *
		 * @param pos Position to set
		 * @param value Value to set, defaults to true
		 * @return Reference to this BitVectorBase
		 */
		constexpr BitVectorBase& set(const size_type pos, const bool value = true)
		{
			const bool oldValue = bits_.test(pos);
			if (oldValue != value)
			{
				bits_.set(pos, value);
				const size_type blockIndex = pos / BITS_PER_BLOCK;
				if (value)
				{
					fenwick_.update(blockIndex, size_type {1});
				}
				else
				{
					fenwick_.update(blockIndex, static_cast<size_type>(-1));
				}
			}
			return *this;
		}

		/**
		 * @brief Resets the bit at the given position.
		 *
		 * @param pos Position to reset
		 * @return Reference to this BitVectorBase
		 */
		constexpr BitVectorBase& reset(const size_type pos)
		{
			return set(pos, false);
		}

		/**
		 * @brief Flips the bit at the given position.
		 *
		 * @param pos Position to flip
		 * @return Reference to this BitVectorBase
		 */
		constexpr BitVectorBase& flip(const size_type pos)
		{
			return set(pos, !bits_.test(pos));
		}

		/**
		 * @brief Clears all bits.
		 */
		constexpr void clear()
		{
			bits_.reset();
			fenwick_.clear();
		}

		// ========================================================================
		// Rank/Select Operations
		// ========================================================================

		/**
		 * @brief Counts the number of set bits in [0, pos], inclusive.
		 *
		 * @param pos Right boundary, inclusive
		 * @return Number of set bits from 0 to pos
		 */
		[[nodiscard]] constexpr size_type rank(const size_type pos) const
		{
			if (pos >= size())
			{
				return count();
			}

			const size_type blockIndex = pos / BITS_PER_BLOCK;
			const size_type bitIndex   = pos % BITS_PER_BLOCK;

			// Sum of complete blocks before this one
			size_type result = blockIndex > 0 ? fenwick_.prefix(blockIndex - 1) : 0;

			// Count bits in the partial block [0, bitIndex]
			const block_type block = GetBlock(blockIndex);
			// Handle the case where bitIndex + 1 == BITS_PER_BLOCK (would cause shift overflow)
			const block_type mask =
				bitIndex + 1 == BITS_PER_BLOCK ? ~block_type {0} : (block_type {1} << (bitIndex + 1)) - 1;
			result += static_cast<size_type>(std::popcount(block & mask));

			return result;
		}

		/**
		 * @brief Counts the number of set bits in [left, right], inclusive.
		 *
		 * @param left Left boundary, inclusive
		 * @param right Right boundary, inclusive
		 * @return Number of set bits from left to right
		 */
		[[nodiscard]] constexpr size_type rank(const size_type left, const size_type right) const
		{
			if (left > right || left >= size())
			{
				return 0;
			}
			if (left == 0)
			{
				return rank(right);
			}
			return rank(right) - rank(left - 1);
		}

		/**
		 * @brief Finds the position of the n-th set bit, 0-indexed.
		 *
		 * @param n The index of the set bit to find, 0-indexed
		 * @return Position of the n-th set bit, or NOT_VALID if not found
		 */
		[[nodiscard]] constexpr size_type select(size_type n) const
		{
			if (n >= count())
			{
				return NOT_VALID;
			}

			// Binary search for the block containing the n-th set bit
			size_type left	= 0;
			size_type right = num_blocks();

			while (left < right)
			{
				const size_type mid		 = left + (right - left) / 2;
				const size_type midCount = mid > 0 ? fenwick_.prefix(mid - 1) : 0;

				if (midCount <= n)
				{
					left = mid + 1;
				}
				else
				{
					right = mid;
				}
			}

			// left - 1 is the block containing the n-th set bit
			const size_type blockIndex = left > 0 ? left - 1 : 0;
			const size_type prevCount  = blockIndex > 0 ? fenwick_.prefix(blockIndex - 1) : 0;
			size_type remaining		   = n - prevCount;

			// Find the (remaining+1)-th set bit within the block using countr_zero
			// This is O(remaining) but remaining is bounded by BITS_PER_BLOCK
			block_type block          = GetBlock(blockIndex);
			const size_type basePos   = blockIndex * BITS_PER_BLOCK;
			const size_type maxBits   = std::min(BITS_PER_BLOCK, size() - basePos);
			const block_type sizeMask = maxBits == BITS_PER_BLOCK ? ~block_type {0} : (block_type {1} << maxBits) - 1;
			block                     &= sizeMask;

			// Iterate: find lowest set bit, decrement remaining, clear that bit
			while (remaining > 0 && block != 0)
			{
				block &= block - 1;	 // Clear lowest set bit
				--remaining;
			}

			if (block == 0)
			{
				return NOT_VALID;
			}

			return basePos + static_cast<size_type>(std::countr_zero(block));
		}

		// ========================================================================
		// Queries
		// ========================================================================

		/**
		 * @brief Returns the total number of set bits.
		 *
		 * @return Total count of set bits
		 */
		[[nodiscard]] constexpr size_type count() const noexcept
		{
			return bits_.count();
		}

		/**
		 * @brief Checks if all bits are set.
		 *
		 * @return true if all bits are set
		 */
		[[nodiscard]] constexpr bool all() const noexcept
		{
			return bits_.all();
		}

		/**
		 * @brief Checks if any bit is set.
		 *
		 * @return true if at least one bit is set
		 */
		[[nodiscard]] constexpr bool any() const noexcept
		{
			return bits_.any();
		}

		/**
		 * @brief Checks if no bits are set.
		 *
		 * @return true if no bits are set
		 */
		[[nodiscard]] constexpr bool none() const noexcept
		{
			return bits_.none();
		}

		// ========================================================================
		// Search Operations
		// ========================================================================

		/**
		 * @brief Finds the position of the first set bit.
		 *
		 * @return Position of the first set bit, or NOT_VALID if none
		 */
		[[nodiscard]] constexpr size_type find_first() const noexcept
		{
			return bits_.find_first();
		}

		/**
		 * @brief Finds the position of the next set bit after pos.
		 *
		 * @param pos Position to search from
		 * @return Position of the next set bit, or NOT_VALID if none
		 */
		[[nodiscard]] constexpr size_type find_next(const size_type pos) const noexcept
		{
			return bits_.find_next(pos);
		}

		// ========================================================================
		// Set Bit Iteration
		// ========================================================================

		/**
		 * @brief Returns an iterator to the first set bit.
		 *
		 * @return Iterator to the first set bit position, or set_bits_end if none
		 */
		[[nodiscard]] constexpr set_bit_iterator set_bits_begin() const noexcept
		{
			return set_bit_iterator(this, find_first());
		}

		/**
		 * @brief Returns a sentinel iterator marking the end of set bits.
		 *
		 * @return End iterator with position equal to NOT_VALID
		 */
		[[nodiscard]] constexpr set_bit_iterator set_bits_end() const noexcept
		{
			return set_bit_iterator(this, NOT_VALID);
		}

		/**
		 * @brief Returns a range view over all set bit positions.
		 *
		 * @details Enables range-for syntax: for auto pos : bv.set_bits
		 *
		 * @return A subrange over set bit positions
		 */
		[[nodiscard]] constexpr auto set_bits() const noexcept
		{
			return std::ranges::subrange(set_bits_begin(), set_bits_end());
		}

		// ========================================================================
		// Underlying Access
		// ========================================================================

		/**
		 * @brief Returns the underlying bitset.
		 *
		 * @return Const reference to the bitset
		 */
		[[nodiscard]] constexpr const Bitset& bits() const noexcept
		{
			return bits_;
		}

	private:
		// ========================================================================
		// Helpers
		// ========================================================================

		[[nodiscard]] static constexpr size_type NumBlocks(const size_type numBits) noexcept
		{
			return (numBits + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;
		}

		[[nodiscard]] constexpr block_type GetBlock(const size_type blockIndex) const noexcept
		{
			if constexpr (requires { bits_.block(blockIndex); })
			{
				// FixedBitset has block() method
				return bits_.block(blockIndex);
			}
			else if constexpr (requires { bits_.data()[blockIndex]; })
			{
				// DynamicBitset has data() method returning pointer to blocks
				return bits_.data()[blockIndex];
			}
			else
			{
				// Fallback for containers with blocks() array access
				return bits_.blocks()[blockIndex];
			}
		}

		constexpr void RebuildFenwick()
		{
			fenwick_.clear();
			for (size_type i = 0; i < num_blocks(); ++i)
			{
				const size_type popcount = static_cast<size_type>(std::popcount(GetBlock(i)));
				fenwick_.update(i, popcount);
			}
		}

		Bitset bits_;
		structure::Fenwick<size_type, FenwickContainer> fenwick_;
	};

	// ========================================================================
	// Type Aliases
	// ========================================================================

	/**
	 * @brief Dynamic BitVector using DynamicBitset and vector-backed Fenwick tree.
	 *
	 * @tparam Block The block type for the underlying bitset
	 */
	template<std::unsigned_integral Block = std::size_t>
	using BitVector = BitVectorBase<DynamicBitset<Block>, std::vector<std::size_t>>;

	/**
	 * @brief Fixed-capacity BitVector using fixed::Bitset and array-backed Fenwick tree.
	 *
	 * @tparam N The number of bits
	 * @tparam Block The block type for the underlying bitset
	 */
	template<std::size_t N, std::unsigned_integral Block = std::size_t>
	using InplaceBitVector =
		BitVectorBase<fixed::Bitset<N, Block>, std::array<std::size_t, fixed::Bitset<N, Block>::NUM_BLOCKS>>;
}
