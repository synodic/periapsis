export module synodic.periapsis:container.dynamic.bitset;

import std;

export namespace peri
{
	/**
	 * @brief A dynamically-sized bitset with configurable block storage.
	 *
	 * @tparam Block The unsigned integral type used for internal storage
	 * @tparam Allocator The allocator type for block storage
	 */
	template<std::unsigned_integral Block = std::size_t, typename Allocator = std::allocator<Block>>
		requires(!std::same_as<Block, bool>)
	class DynamicBitset
	{
	public:
		// ========================================================================
		// Type Definitions
		// ========================================================================

		using block_type	  = Block;
		using allocator_type  = Allocator;
		using size_type		  = std::size_t;
		using difference_type = std::ptrdiff_t;

		static constexpr size_type BITS_PER_BLOCK = std::numeric_limits<Block>::digits;
		static constexpr size_type NOT_VALID	  = static_cast<size_type>(-1);

		// ========================================================================
		// Reference Proxy
		// ========================================================================

		/**
		 * @brief Proxy class for non-const bit access via operator[]
		 */
		class Proxy
		{
			friend class DynamicBitset;

		public:
			constexpr Proxy(const Proxy&) noexcept = default;

			constexpr Proxy& operator=(const Proxy& other) noexcept
			{
				return *this = static_cast<bool>(other);
			}

			constexpr ~Proxy() noexcept = default;

			constexpr Proxy& operator=(const bool value) noexcept
			{
				if (value)
				{
					block_ |= mask_;
				}
				else
				{
					block_ &= ~mask_;
				}
				return *this;
			}

			constexpr Proxy& operator|=(const bool value) noexcept
			{
				if (value)
				{
					block_ |= mask_;
				}
				return *this;
			}

			constexpr Proxy& operator&=(const bool value) noexcept
			{
				if (!value)
				{
					block_ &= ~mask_;
				}
				return *this;
			}

			constexpr Proxy& operator^=(const bool value) noexcept
			{
				if (value)
				{
					block_ ^= mask_;
				}
				return *this;
			}

			constexpr Proxy& operator-=(const bool value) noexcept
			{
				if (value)
				{
					block_ &= ~mask_;
				}
				return *this;
			}

			[[nodiscard]] explicit(false) constexpr operator bool() const noexcept
			{
				return (block_ & mask_) != 0;
			}

			[[nodiscard]] constexpr bool operator~() const noexcept
			{
				return (block_ & mask_) == 0;
			}

			constexpr Proxy& flip() noexcept
			{
				block_ ^= mask_;
				return *this;
			}

		private:
			constexpr Proxy(Block& block, Block mask) noexcept :
				block_(block),
				mask_(mask)
			{
			}

			Block& block_;
			Block mask_;
		};

		// ========================================================================
		// Iterator
		// ========================================================================

		/**
		 * @brief Random access proxy iterator over bits
		 */
		template<bool IsConst>
		class BitIterator
		{
			friend class DynamicBitset;
			friend class BitIterator<!IsConst>;
			using bitset_type = std::conditional_t<IsConst, const DynamicBitset, DynamicBitset>;

		public:
			using iterator_category = std::random_access_iterator_tag;
			using value_type		= bool;
			using difference_type	= std::ptrdiff_t;
			using pointer			= void;
			using reference			= std::conditional_t<IsConst, bool, Proxy>;

			constexpr BitIterator() noexcept							  = default;
			constexpr BitIterator(const BitIterator&) noexcept			  = default;
			constexpr BitIterator& operator=(const BitIterator&) noexcept = default;

			// Converting constructor: non-const to const iterator
			explicit(false) constexpr BitIterator(const BitIterator<false>& other) noexcept
				requires IsConst
				:
				bitset_(other.bitset_),
				pos_(other.pos_)
			{
			}

			[[nodiscard]] constexpr reference operator*() const noexcept
			{
				if constexpr (IsConst)
				{
					return bitset_->UncheckedTest(pos_);
				}
				else
				{
					return (*bitset_)[pos_];
				}
			}

			[[nodiscard]] constexpr reference operator[](const difference_type n) const noexcept
			{
				if constexpr (IsConst)
				{
					return bitset_->UncheckedTest(pos_ + static_cast<size_type>(n));
				}
				else
				{
					return (*bitset_)[pos_ + static_cast<size_type>(n)];
				}
			}

			constexpr BitIterator& operator++() noexcept
			{
				++pos_;
				return *this;
			}

			constexpr BitIterator operator++(int) noexcept
			{
				BitIterator tmp = *this;
				++pos_;
				return tmp;
			}

			constexpr BitIterator& operator--() noexcept
			{
				--pos_;
				return *this;
			}

			constexpr BitIterator operator--(int) noexcept
			{
				BitIterator tmp = *this;
				--pos_;
				return tmp;
			}

			constexpr BitIterator& operator+=(const difference_type n) noexcept
			{
				pos_ += static_cast<size_type>(n);
				return *this;
			}

			constexpr BitIterator& operator-=(const difference_type n) noexcept
			{
				pos_ -= static_cast<size_type>(n);
				return *this;
			}

			[[nodiscard]] constexpr BitIterator operator+(const difference_type n) const noexcept
			{
				return bit_iterator(bitset_, pos_ + static_cast<size_type>(n));
			}

			[[nodiscard]] constexpr BitIterator operator-(const difference_type n) const noexcept
			{
				return bit_iterator(bitset_, pos_ - static_cast<size_type>(n));
			}

			[[nodiscard]] constexpr difference_type operator-(const BitIterator& other) const noexcept
			{
				return static_cast<difference_type>(pos_) - static_cast<difference_type>(other.pos_);
			}

			[[nodiscard]] friend constexpr BitIterator operator+(difference_type n, const BitIterator& it) noexcept
			{
				return it + n;
			}

			[[nodiscard]] constexpr bool operator==(const BitIterator& other) const noexcept
			{
				return pos_ == other.pos_;
			}

			[[nodiscard]] constexpr auto operator<=>(const BitIterator& other) const noexcept
			{
				return pos_ <=> other.pos_;
			}

		private:
			constexpr BitIterator(bitset_type* bs, const size_type pos) noexcept :
				bitset_(bs),
				pos_(pos)
			{
			}

			bitset_type* bitset_ = nullptr;
			size_type pos_		 = 0;
		};

		using iterator				 = BitIterator<false>;
		using const_iterator		 = BitIterator<true>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		// ========================================================================
		// Construction
		// ========================================================================

		constexpr DynamicBitset() = default;

		constexpr explicit DynamicBitset(const Allocator& allocator) :
			blocks_(allocator)
		{
		}

		constexpr explicit DynamicBitset(const size_type numBits, const Allocator& allocator = Allocator()) :
			blocks_(NumBlocks(numBits), Block {0}, allocator),
			size_(numBits)
		{
		}

		constexpr DynamicBitset(
			const size_type numBits,
			unsigned long long value,
			const Allocator& allocator = Allocator()) :
			blocks_(NumBlocks(numBits), Block {0}, allocator),
			size_(numBits)
		{
			if (numBits > 0)
			{
				constexpr size_type ullBits = std::numeric_limits<unsigned long long>::digits;
				const size_type bitsToCopy	= std::min(numBits, ullBits);

				if constexpr (BITS_PER_BLOCK >= ullBits)
				{
					blocks_[0] = static_cast<Block>(value);
				}
				else
				{
					size_type bitIndex	 = 0;
					size_type blockIndex = 0;
					while (bitIndex < bitsToCopy && blockIndex < blocks_.size())
					{
						blocks_[blockIndex] = static_cast<Block>(value >> bitIndex);
						bitIndex += BITS_PER_BLOCK;
						++blockIndex;
					}
				}
				ZeroUnusedBits();
			}
		}

		template<typename CharT, typename Traits, typename CharAlloc>
		constexpr explicit DynamicBitset(
			const std::basic_string<CharT, Traits, CharAlloc>& str,
			std::basic_string<CharT, Traits, CharAlloc>::size_type pos = 0,
			std::basic_string<CharT, Traits, CharAlloc>::size_type n =
				std::basic_string<CharT, Traits, CharAlloc>::npos,
			CharT zero				   = CharT('0'),
			CharT one				   = CharT('1'),
			const Allocator& allocator = Allocator()) :
			blocks_(allocator)
		{
			InitFromString(str.data(), str.size(), pos, n, zero, one);
		}

		template<typename CharT>
		constexpr explicit DynamicBitset(
			const CharT* str,
			std::basic_string<CharT>::size_type n = std::basic_string<CharT>::npos,
			CharT zero							  = CharT('0'),
			CharT one							  = CharT('1'),
			const Allocator& allocator			  = Allocator()) :
			blocks_(allocator)
		{
			const size_type len = n == std::basic_string<CharT>::npos ? std::char_traits<CharT>::length(str) : n;
			InitFromString(str, len, 0, len, zero, one);
		}

		constexpr DynamicBitset(const DynamicBitset&)	  = default;
		constexpr DynamicBitset(DynamicBitset&&) noexcept = default;

		constexpr DynamicBitset& operator=(const DynamicBitset&)	 = default;
		constexpr DynamicBitset& operator=(DynamicBitset&&) noexcept = default;

		constexpr ~DynamicBitset() = default;

		// ========================================================================
		// Iterators
		// ========================================================================

		[[nodiscard]] constexpr iterator begin() noexcept
		{
			return iterator(this, 0);
		}

		[[nodiscard]] constexpr const_iterator begin() const noexcept
		{
			return const_iterator(this, 0);
		}

		[[nodiscard]] constexpr const_iterator cbegin() const noexcept
		{
			return const_iterator(this, 0);
		}

		[[nodiscard]] constexpr iterator end() noexcept
		{
			return iterator(this, size_);
		}

		[[nodiscard]] constexpr const_iterator end() const noexcept
		{
			return const_iterator(this, size_);
		}

		[[nodiscard]] constexpr const_iterator cend() const noexcept
		{
			return const_iterator(this, size_);
		}

		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept
		{
			return reverse_iterator(end());
		}

		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept
		{
			return const_reverse_iterator(end());
		}

		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
		{
			return const_reverse_iterator(cend());
		}

		[[nodiscard]] constexpr reverse_iterator rend() noexcept
		{
			return reverse_iterator(begin());
		}

		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept
		{
			return const_reverse_iterator(begin());
		}

		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
		{
			return const_reverse_iterator(cbegin());
		}

		// ========================================================================
		// Capacity
		// ========================================================================

		[[nodiscard]] constexpr bool empty() const noexcept
		{
			return size_ == 0;
		}

		[[nodiscard]] constexpr size_type size() const noexcept
		{
			return size_;
		}

		[[nodiscard]] constexpr size_type num_blocks() const noexcept
		{
			return blocks_.size();
		}

		[[nodiscard]] constexpr size_type max_size() const noexcept
		{
			const size_type max_blocks = blocks_.max_size();
			return max_blocks * BITS_PER_BLOCK;
		}

		[[nodiscard]] constexpr size_type capacity() const noexcept
		{
			return blocks_.capacity() * BITS_PER_BLOCK;
		}

		constexpr void reserve(const size_type numBits)
		{
			blocks_.reserve(NumBlocks(numBits));
		}

		constexpr void shrink_to_fit()
		{
			blocks_.shrink_to_fit();
		}

		// ========================================================================
		// Element Access
		// ========================================================================

		[[nodiscard]] constexpr Proxy operator[](const size_type pos) noexcept
		{
			return Proxy(blocks_[BlockIndex(pos)], BitMask(pos));
		}

		[[nodiscard]] constexpr bool operator[](const size_type pos) const noexcept
		{
			return test(pos);
		}

		[[nodiscard]] constexpr bool test(const size_type pos) const
		{
			if (pos >= size_)
			{
				throw std::out_of_range("DynamicBitset::test: position out of range");
			}
			return UncheckedTest(pos);
		}

		[[nodiscard]] constexpr bool test_set(const size_type pos, const bool value = true)
		{
			if (pos >= size_)
			{
				throw std::out_of_range("DynamicBitset::test_set: position out of range");
			}
			const bool oldValue = UncheckedTest(pos);
			UncheckedSet(pos, value);
			return oldValue;
		}

		// ========================================================================
		// Modifiers - Bit Operations
		// ========================================================================

		constexpr DynamicBitset& set() noexcept
		{
			std::ranges::fill(blocks_, ~Block {0});
			ZeroUnusedBits();
			return *this;
		}

		constexpr DynamicBitset& set(const size_type pos, const bool value = true)
		{
			if (pos >= size_)
			{
				throw std::out_of_range("DynamicBitset::set: position out of range");
			}
			UncheckedSet(pos, value);
			return *this;
		}

		constexpr DynamicBitset& reset() noexcept
		{
			std::ranges::fill(blocks_, Block {0});
			return *this;
		}

		constexpr DynamicBitset& reset(const size_type pos)
		{
			if (pos >= size_)
			{
				throw std::out_of_range("DynamicBitset::reset: position out of range");
			}
			blocks_[BlockIndex(pos)] &= ~BitMask(pos);
			return *this;
		}

		constexpr DynamicBitset& flip() noexcept
		{
			for (auto& block: blocks_)
			{
				block = ~block;
			}
			ZeroUnusedBits();
			return *this;
		}

		constexpr DynamicBitset& flip(const size_type pos)
		{
			if (pos >= size_)
			{
				throw std::out_of_range("DynamicBitset::flip: position out of range");
			}
			blocks_[BlockIndex(pos)] ^= BitMask(pos);
			return *this;
		}

		// ========================================================================
		// Modifiers - Resizing
		// ========================================================================

		constexpr void clear() noexcept
		{
			blocks_.clear();
			size_ = 0;
		}

		constexpr void resize(const size_type numBits, bool value = false)
		{
			const size_type oldNumBlocks = blocks_.size();

			if (const size_type newNumBlocks = NumBlocks(numBits); newNumBlocks != oldNumBlocks)
			{
				blocks_.resize(newNumBlocks, value ? ~Block {0} : Block {0});
			}

			if (numBits > size_ && value)
			{
				// Set new bits in the partially filled old last block
				const size_type extraBits = ExtraBits(size_);
				if (extraBits > 0 && oldNumBlocks > 0)
				{
					blocks_[oldNumBlocks - 1] |= ~Block {0} << extraBits;
				}
			}

			size_ = numBits;
			ZeroUnusedBits();
		}

		constexpr void push_back(const bool value)
		{
			if (const size_type newSize = size_ + 1; NumBlocks(newSize) > blocks_.size())
			{
				blocks_.push_back(Block {0});
			}
			++size_;
			if (value)
			{
				blocks_[BlockIndex(size_ - 1)] |= BitMask(size_ - 1);
			}
		}

		constexpr void pop_back()
		{
			if (size_ > 0)
			{
				--size_;
				// Clear the popped bit
				if (const size_type extra = ExtraBits(size_); extra == 0 && !blocks_.empty())
				{
					blocks_.pop_back();
				}
				else
				{
					ZeroUnusedBits();
				}
			}
		}

		constexpr void append(Block block)
		{
			const size_type extra = ExtraBits(size_);
			if (extra == 0)
			{
				blocks_.push_back(block);
			}
			else
			{
				blocks_.back() |= block << extra;
				blocks_.push_back(block >> (BITS_PER_BLOCK - extra));
			}
			size_ += BITS_PER_BLOCK;
		}

		constexpr void swap(DynamicBitset& other) noexcept
		{
			blocks_.swap(other.blocks_);
			std::swap(size_, other.size_);
		}

		// ========================================================================
		// Queries
		// ========================================================================

		[[nodiscard]] constexpr size_type count() const noexcept
		{
			return std::transform_reduce(
				blocks_.begin(),
				blocks_.end(),
				size_type {0},
				std::plus<> {},
				[](Block b)
				{
					return static_cast<size_type>(std::popcount(b));
				});
		}

		[[nodiscard]] constexpr bool all() const noexcept
		{
			if (empty())
			{
				return true;
			}

			// Check all complete blocks
			for (size_type i = 0; i + 1 < blocks_.size(); ++i)
			{
				if (blocks_[i] != ~Block {0})
				{
					return false;
				}
			}

			// Check last block
			const size_type extra = ExtraBits(size_);
			const Block mask	  = extra == 0 ? ~Block {0} : (Block {1} << extra) - 1;
			return blocks_.back() == mask;
		}

		[[nodiscard]] constexpr bool any() const noexcept
		{
			return std::ranges::any_of(
				blocks_,
				[](Block b)
				{
					return b != Block {0};
				});
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
			for (size_type i = 0; i < blocks_.size(); ++i)
			{
				if (blocks_[i] != Block {0})
				{
					const size_type bitPos = i * BITS_PER_BLOCK + static_cast<size_type>(std::countr_zero(blocks_[i]));
					return bitPos < size_ ? bitPos : NOT_VALID;
				}
			}
			return NOT_VALID;
		}

		[[nodiscard]] constexpr size_type find_next(size_type pos) const noexcept
		{
			if (pos >= size_ || empty())
			{
				return NOT_VALID;
			}

			++pos;	// Start searching from the next position
			if (pos >= size_)
			{
				return NOT_VALID;
			}

			size_type blockIndex = BlockIndex(pos);
			size_type bitIndex	 = BitIndex(pos);

			// Check remaining bits in current block
			Block masked = blocks_[blockIndex] >> bitIndex;
			if (masked != Block {0})
			{
				const size_type result = pos + static_cast<size_type>(std::countr_zero(masked));
				return result < size_ ? result : NOT_VALID;
			}

			// Search subsequent blocks
			for (++blockIndex; blockIndex < blocks_.size(); ++blockIndex)
			{
				if (blocks_[blockIndex] != Block {0})
				{
					const size_type result =
						blockIndex * BITS_PER_BLOCK + static_cast<size_type>(std::countr_zero(blocks_[blockIndex]));
					return result < size_ ? result : NOT_VALID;
				}
			}

			return NOT_VALID;
		}

		[[nodiscard]] constexpr size_type find_first_off() const noexcept
		{
			if (empty())
			{
				return NOT_VALID;
			}

			for (size_type i = 0; i + 1 < blocks_.size(); ++i)
			{
				if (blocks_[i] != ~Block {0})
				{
					return i * BITS_PER_BLOCK + static_cast<size_type>(std::countr_one(blocks_[i]));
				}
			}

			// Check last block
			if (!blocks_.empty())
			{
				const size_type extra = ExtraBits(size_);
				const Block mask	  = extra == 0 ? ~Block {0} : (Block {1} << extra) - 1;
				const Block last	  = blocks_.back();
				const Block inverted  = ~last & mask;
				if (inverted != Block {0})
				{
					const size_type bitPos =
						(blocks_.size() - 1) * BITS_PER_BLOCK + static_cast<size_type>(std::countr_zero(inverted));
					return bitPos < size_ ? bitPos : NOT_VALID;
				}
			}

			return NOT_VALID;
		}

		[[nodiscard]] constexpr size_type find_next_off(size_type pos) const noexcept
		{
			if (pos >= size_ || empty())
			{
				return NOT_VALID;
			}

			++pos;
			if (pos >= size_)
			{
				return NOT_VALID;
			}

			size_type blockIndex = BlockIndex(pos);
			size_type bitIndex	 = BitIndex(pos);

			// Create mask for remaining bits in this block
			const Block remainingMask = ~Block {0} << bitIndex;
			Block inverted			  = ~blocks_[blockIndex] & remainingMask;

			if (inverted != Block {0})
			{
				const size_type result =
					blockIndex * BITS_PER_BLOCK + static_cast<size_type>(std::countr_zero(inverted));
				return result < size_ ? result : NOT_VALID;
			}

			// Search subsequent blocks
			for (++blockIndex; blockIndex + 1 < blocks_.size(); ++blockIndex)
			{
				if (blocks_[blockIndex] != ~Block {0})
				{
					return blockIndex * BITS_PER_BLOCK + static_cast<size_type>(std::countr_one(blocks_[blockIndex]));
				}
			}

			// Check last block
			if (blockIndex < blocks_.size())
			{
				const size_type extra = ExtraBits(size_);
				const Block mask	  = extra == 0 ? ~Block {0} : (Block {1} << extra) - 1;
				inverted			  = ~blocks_[blockIndex] & mask;
				if (inverted != Block {0})
				{
					const size_type result =
						blockIndex * BITS_PER_BLOCK + static_cast<size_type>(std::countr_zero(inverted));
					return result < size_ ? result : NOT_VALID;
				}
			}

			return NOT_VALID;
		}

		// ========================================================================
		// Set Operations
		// ========================================================================

		[[nodiscard]] constexpr bool is_subset_of(const DynamicBitset& other) const noexcept
		{
			if (size_ > other.size_)
			{
				// Check if bits beyond other's size are all zero
				for (size_type i = other.blocks_.size(); i < blocks_.size(); ++i)
				{
					if (blocks_[i] != Block {0})
					{
						return false;
					}
				}
			}

			const size_type commonBlocks = std::min(blocks_.size(), other.blocks_.size());
			for (size_type i = 0; i < commonBlocks; ++i)
			{
				if ((blocks_[i] & ~other.blocks_[i]) != Block {0})
				{
					return false;
				}
			}
			return true;
		}

		[[nodiscard]] constexpr bool is_proper_subset_of(const DynamicBitset& other) const noexcept
		{
			return is_subset_of(other) && *this != other;
		}

		[[nodiscard]] constexpr bool intersects(const DynamicBitset& other) const noexcept
		{
			const size_type commonBlocks = std::min(blocks_.size(), other.blocks_.size());
			for (size_type i = 0; i < commonBlocks; ++i)
			{
				if ((blocks_[i] & other.blocks_[i]) != Block {0})
				{
					return true;
				}
			}
			return false;
		}

		// ========================================================================
		// Bitwise Operators (In-Place)
		// ========================================================================

		constexpr DynamicBitset& operator&=(const DynamicBitset& other) noexcept
		{
			const size_type commonBlocks = std::min(blocks_.size(), other.blocks_.size());
			for (size_type i = 0; i < commonBlocks; ++i)
			{
				blocks_[i] &= other.blocks_[i];
			}
			// Blocks beyond other's size become zero
			for (size_type i = commonBlocks; i < blocks_.size(); ++i)
			{
				blocks_[i] = Block {0};
			}
			return *this;
		}

		constexpr DynamicBitset& operator|=(const DynamicBitset& other) noexcept
		{
			if (other.size_ > size_)
			{
				resize(other.size_);
			}
			const size_type commonBlocks = std::min(blocks_.size(), other.blocks_.size());
			for (size_type i = 0; i < commonBlocks; ++i)
			{
				blocks_[i] |= other.blocks_[i];
			}
			return *this;
		}

		constexpr DynamicBitset& operator^=(const DynamicBitset& other) noexcept
		{
			if (other.size_ > size_)
			{
				resize(other.size_);
			}
			const size_type commonBlocks = std::min(blocks_.size(), other.blocks_.size());
			for (size_type i = 0; i < commonBlocks; ++i)
			{
				blocks_[i] ^= other.blocks_[i];
			}
			return *this;
		}

		constexpr DynamicBitset& operator-=(const DynamicBitset& other) noexcept
		{
			const size_type commonBlocks = std::min(blocks_.size(), other.blocks_.size());
			for (size_type i = 0; i < commonBlocks; ++i)
			{
				blocks_[i] &= ~other.blocks_[i];
			}
			return *this;
		}

		constexpr DynamicBitset& operator<<=(const size_type shift) noexcept
		{
			if (shift == 0 || empty())
			{
				return *this;
			}

			if (shift >= size_)
			{
				reset();
				return *this;
			}

			const size_type blockShift = shift / BITS_PER_BLOCK;
			const size_type bitShift   = shift % BITS_PER_BLOCK;

			if (bitShift == 0)
			{
				// Shift by whole blocks
				for (size_type i = blocks_.size(); i-- > blockShift;)
				{
					blocks_[i] = blocks_[i - blockShift];
				}
			}
			else
			{
				const size_type reverse_shift = BITS_PER_BLOCK - bitShift;
				for (size_type i = blocks_.size(); i-- > blockShift + 1;)
				{
					blocks_[i] = blocks_[i - blockShift] << bitShift | blocks_[i - blockShift - 1] >> reverse_shift;
				}
				blocks_[blockShift] = blocks_[0] << bitShift;
			}

			// Zero out low blocks
			for (size_type i = 0; i < blockShift; ++i)
			{
				blocks_[i] = Block {0};
			}

			ZeroUnusedBits();
			return *this;
		}

		constexpr DynamicBitset& operator>>=(const size_type shift) noexcept
		{
			if (shift == 0 || empty())
			{
				return *this;
			}

			if (shift >= size_)
			{
				reset();
				return *this;
			}

			const size_type blockShift = shift / BITS_PER_BLOCK;
			const size_type bitShift   = shift % BITS_PER_BLOCK;
			const size_type limit	   = blocks_.size() - blockShift - 1;

			if (bitShift == 0)
			{
				for (size_type i = 0; i <= limit; ++i)
				{
					blocks_[i] = blocks_[i + blockShift];
				}
			}
			else
			{
				const size_type reverseShift = BITS_PER_BLOCK - bitShift;
				for (size_type i = 0; i < limit; ++i)
				{
					blocks_[i] = blocks_[i + blockShift] >> bitShift | blocks_[i + blockShift + 1] << reverseShift;
				}
				blocks_[limit] = blocks_[blocks_.size() - 1] >> bitShift;
			}

			// Zero out high blocks
			for (size_type i = limit + 1; i < blocks_.size(); ++i)
			{
				blocks_[i] = Block {0};
			}

			return *this;
		}

		[[nodiscard]] constexpr DynamicBitset operator~() const
		{
			DynamicBitset result(*this);
			result.flip();
			return result;
		}

		[[nodiscard]] constexpr DynamicBitset operator<<(size_type shift) const
		{
			DynamicBitset result(*this);
			result <<= shift;
			return result;
		}

		[[nodiscard]] constexpr DynamicBitset operator>>(size_type shift) const
		{
			DynamicBitset result(*this);
			result >>= shift;
			return result;
		}

		// ========================================================================
		// Comparison Operators
		// ========================================================================

		[[nodiscard]] constexpr bool operator==(const DynamicBitset& other) const noexcept
		{
			if (size_ != other.size_)
			{
				return false;
			}
			return blocks_ == other.blocks_;
		}

		[[nodiscard]] constexpr auto operator<=>(const DynamicBitset& other) const noexcept
		{
			// Lexicographic comparison starting from MSB
			const size_type maxBlocks = std::max(blocks_.size(), other.blocks_.size());
			for (size_type i = maxBlocks; i-- > 0;)
			{
				const Block lhs = i < blocks_.size() ? blocks_[i] : Block {0};
				const Block rhs = i < other.blocks_.size() ? other.blocks_[i] : Block {0};
				if (auto cmp = lhs <=> rhs; cmp != 0)
				{
					return cmp;
				}
			}
			return std::strong_ordering::equal;
		}

		// ========================================================================
		// Conversion
		// ========================================================================

		[[nodiscard]] constexpr unsigned long to_ulong() const
		{
			if (size_ == 0)
			{
				return 0;
			}

			constexpr size_type ulongBits = std::numeric_limits<unsigned long>::digits;
			if (size_ > ulongBits)
			{
				// Check if bits beyond ulong range are set
				const size_type ulongBlocks = (ulongBits + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;
				for (size_type i = ulongBlocks; i < blocks_.size(); ++i)
				{
					if (blocks_[i] != Block {0})
					{
						throw std::overflow_error("DynamicBitset::to_ulong: value too large");
					}
				}
				// Check upper bits of the last relevant block
				if constexpr (ulongBits % BITS_PER_BLOCK != 0)
				{
					constexpr size_type extra = ulongBits % BITS_PER_BLOCK;
					if (ulongBlocks > 0 && ulongBlocks <= blocks_.size())
					{
						if (blocks_[ulongBlocks - 1] >> extra != Block {0})
						{
							throw std::overflow_error("DynamicBitset::to_ulong: value too large");
						}
					}
				}
			}

			unsigned long result = 0;
			size_type shift		 = 0;
			for (size_type i = 0; i < blocks_.size() && shift < ulongBits; ++i)
			{
				result |= static_cast<unsigned long>(blocks_[i]) << shift;
				shift += BITS_PER_BLOCK;
			}
			return result;
		}

		[[nodiscard]] constexpr unsigned long long to_ullong() const
		{
			if (size_ == 0)
			{
				return 0;
			}

			constexpr size_type ullongBits = std::numeric_limits<unsigned long long>::digits;
			if (size_ > ullongBits)
			{
				const size_type ullongBlocks = (ullongBits + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;
				for (size_type i = ullongBlocks; i < blocks_.size(); ++i)
				{
					if (blocks_[i] != Block {0})
					{
						throw std::overflow_error("DynamicBitset::to_ullong: value too large");
					}
				}
				if constexpr (ullongBits % BITS_PER_BLOCK != 0)
				{
					constexpr size_type extra = ullongBits % BITS_PER_BLOCK;
					if (ullongBlocks > 0 && ullongBlocks <= blocks_.size())
					{
						if (blocks_[ullongBlocks - 1] >> extra != Block {0})
						{
							throw std::overflow_error("DynamicBitset::to_ullong: value too large");
						}
					}
				}
			}

			unsigned long long result = 0;
			size_type shift			  = 0;
			for (size_type i = 0; i < blocks_.size() && shift < ullongBits; ++i)
			{
				result |= static_cast<unsigned long long>(blocks_[i]) << shift;
				shift += BITS_PER_BLOCK;
			}
			return result;
		}

		template<
			typename CharT	  = char,
			typename Traits	  = std::char_traits<CharT>,
			typename StrAlloc = std::allocator<CharT>>
		[[nodiscard]] constexpr std::basic_string<CharT, Traits, StrAlloc>
			to_string(CharT zero = CharT('0'), CharT one = CharT('1')) const
		{
			std::basic_string<CharT, Traits, StrAlloc> result(size_, zero);
			for (size_type i = 0; i < size_; ++i)
			{
				if (UncheckedTest(i))
				{
					result[size_ - 1 - i] = one;
				}
			}
			return result;
		}

		// ========================================================================
		// Allocator Access
		// ========================================================================

		[[nodiscard]] constexpr allocator_type get_allocator() const noexcept
		{
			return blocks_.get_allocator();
		}

		// ========================================================================
		// Block Access (for advanced/SIMD usage)
		// ========================================================================

		[[nodiscard]] constexpr Block* data() noexcept
		{
			return blocks_.data();
		}

		[[nodiscard]] constexpr const Block* data() const noexcept
		{
			return blocks_.data();
		}

		// ========================================================================
		// Friend Operators
		// ========================================================================

		[[nodiscard]] friend constexpr DynamicBitset operator&(DynamicBitset lhs, const DynamicBitset& rhs)
		{
			lhs &= rhs;
			return lhs;
		}

		[[nodiscard]] friend constexpr DynamicBitset operator|(DynamicBitset lhs, const DynamicBitset& rhs)
		{
			lhs |= rhs;
			return lhs;
		}

		[[nodiscard]] friend constexpr DynamicBitset operator^(DynamicBitset lhs, const DynamicBitset& rhs)
		{
			lhs ^= rhs;
			return lhs;
		}

		[[nodiscard]] friend constexpr DynamicBitset operator-(DynamicBitset lhs, const DynamicBitset& rhs)
		{
			lhs -= rhs;
			return lhs;
		}

		friend constexpr void swap(DynamicBitset& lhs, DynamicBitset& rhs) noexcept
		{
			lhs.swap(rhs);
		}

		template<typename CharT, typename Traits>
		friend std::basic_ostream<CharT, Traits>&
			operator<<(std::basic_ostream<CharT, Traits>& os, const DynamicBitset& bs)
		{
			return os << bs.to_string<CharT, Traits>();
		}

		template<typename CharT, typename Traits>
		friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, DynamicBitset& bs)
		{
			std::basic_string<CharT, Traits> str;
			is >> str;
			bs = DynamicBitset(str);
			return is;
		}

	private:
		// ========================================================================
		// Private Helpers
		// ========================================================================

		[[nodiscard]] static constexpr size_type NumBlocks(const size_type numBits) noexcept
		{
			return numBits == 0 ? 0 : (numBits - 1) / BITS_PER_BLOCK + 1;
		}

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
			return Block {1} << BitIndex(pos);
		}

		[[nodiscard]] static constexpr size_type ExtraBits(const size_type numBits) noexcept
		{
			return numBits % BITS_PER_BLOCK;
		}

		constexpr void ZeroUnusedBits() noexcept
		{
			const size_type extra = ExtraBits(size_);
			if (extra != 0 && !blocks_.empty())
			{
				blocks_.back() &= (Block {1} << extra) - 1;
			}
		}

		[[nodiscard]] constexpr bool UncheckedTest(const size_type pos) const noexcept
		{
			return (blocks_[BlockIndex(pos)] & BitMask(pos)) != Block {0};
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

		template<typename CharT>
		constexpr void InitFromString(
			const CharT* str,
			const size_type strLen,
			const size_type pos,
			const size_type n,
			CharT zero,
			CharT one)
		{
			if (pos > strLen)
			{
				throw std::out_of_range("DynamicBitset: position out of range");
			}

			const size_type rlen = std::min(n, strLen - pos);
			size_				 = rlen;
			blocks_.resize(NumBlocks(size_), Block {0});

			// String is read from right to left (LSB is rightmost character)
			for (size_type i = 0; i < rlen; ++i)
			{
				const CharT c = str[pos + rlen - 1 - i];
				if (c == one)
				{
					blocks_[BlockIndex(i)] |= BitMask(i);
				}
				else if (c != zero)
				{
					throw std::invalid_argument("DynamicBitset: invalid character in string");
				}
			}
		}

		std::vector<Block, Allocator> blocks_;
		size_type size_ = 0;
	};

	// ========================================================================
	// Deduction Guides
	// ========================================================================

	template<typename CharT, typename Traits, typename Alloc>
	DynamicBitset(const std::basic_string<CharT, Traits, Alloc>&) -> DynamicBitset<>;

	template<typename CharT>
	DynamicBitset(const CharT*) -> DynamicBitset<>;
}
