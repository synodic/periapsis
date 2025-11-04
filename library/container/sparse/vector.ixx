export module synodic.library:container.sparse.vector;

import :container.sparse.bit_map;
import :exception.not_implemented;

import std;

export namespace synodic::sparse
{

	template<class T, std::uint8_t BlockSize>
	class Vector 
	{
		using BlockType = BitMap<size_t, BlockSize>;

		template<bool Const = false>
		class SparseVectorIterator
		{
			using OuterIteratorType = typename std::vector<BlockType>::iterator;
			using InnerIteratorType = typename BlockType::iterator;

		public:
			using iterator_category = std::bidirectional_iterator_tag;
			using value_type		= T;
			using difference_type	= std::ptrdiff_t;
			using pointer			= T*;
			using reference			= T&;

			SparseVectorIterator(OuterIteratorType = nullptr, InnerIteratorType = nullptr);

			T& operator*();

		private:
			OuterIteratorType outerIterator_;
			InnerIteratorType innerIterator_;
		};

	public:
		using size_type		 = size_t;
		using value_type	 = T;
		using pointer		 = T*;
		using reference		 = T&;
		using iterator		 = SparseVectorIterator<false>;
		using const_iterator = SparseVectorIterator<true>;

		Vector();
		~Vector() = default;

		[[nodiscard]] size_type Size() const noexcept;
		void Reserve(size_type count);

		void Clear() noexcept;
		std::pair<iterator, bool> Insert(const size_type position, const T& value);
		template<typename... Args>
		std::pair<iterator, bool> Emplace(const size_type position, Args&&... args);
		iterator Erase(const_iterator pos);

		iterator Find(size_type position);

		[[nodiscard]] size_type BucketCount() const noexcept;

		iterator begin() noexcept;
		iterator end() noexcept;

	private:
		std::vector<BlockType> blocks_;
		size_type size_;
	};

	template<class T, std::uint8_t BlockSize>
	template<bool Const>
	Vector<T, BlockSize>::SparseVectorIterator<Const>::SparseVectorIterator(
		OuterIteratorType outerIterator,
		InnerIteratorType innerIterator) :
		outerIterator_(outerIterator), innerIterator_(innerIterator)
	{
	}

	template<class T, std::uint8_t BlockSize>
	template<bool Const>
	T& Vector<T, BlockSize>::SparseVectorIterator<Const>::operator*()
	{
		return *innerIterator_;
	}

	template<class T, std::uint8_t BlockSize>
	Vector<T, BlockSize>::Vector() : blocks_(4), size_(0)
	{
	}

	/*
	 * Gets the size
	 *
	 * @returns	The number of elements stored in the table.
	 */

	template<class T, std::uint8_t BlockSize>
	typename Vector<T, BlockSize>::size_type Vector<T, BlockSize>::Size() const noexcept
	{
		return size_;
	}

	template<class T, std::uint8_t BlockSize>
	void Vector<T, BlockSize>::Reserve(size_type count)
	{
		blocks_.reserve((count - 1) / BlockSize + 1);
	}

	template<class T, std::uint8_t BlockSize>
	void Vector<T, BlockSize>::Clear() noexcept
	{
		blocks_.clear();
		size_ = 0;
	}

	template<class T, std::uint8_t BlockSize>
	std::pair<typename Vector<T, BlockSize>::iterator, bool> Vector<T, BlockSize>::Insert(
		const size_type position,
		const T& value)
	{
		auto index	 = position / BlockSize;
		auto& bitMap = blocks_.at(index);

		auto [foundValue, inserted] = bitMap.Insert(position % BlockSize, value);

		if(inserted)
		{
			++size_;
		}

		return {{blocks_.begin() + index, foundValue}, inserted};
	}

	template<class T, std::uint8_t BlockSize>
	template<typename... Args>
	std::pair<typename Vector<T, BlockSize>::iterator, bool> Vector<T, BlockSize>::Emplace(
		const size_type position,
		Args&&... args)
	{
		throw NotImplementedException ();
	}

	template<class T, std::uint8_t BlockSize>
	typename Vector<T, BlockSize>::iterator Vector<T, BlockSize>::Erase(const_iterator pos)
	{
		throw NotImplementedException ();
	}

	template<class T, std::uint8_t BlockSize>
	typename Vector<T, BlockSize>::iterator Vector<T, BlockSize>::Find(const size_type position)
	{
		auto index	 = position / BlockSize;
		auto& bitMap = blocks_.at(index);

		if(auto innerIndex = position % BlockSize; bitMap.Test(innerIndex))
		{
			return {blocks_.begin() + index, bitMap.Find(innerIndex)};
		}

		return {blocks_.end(), bitMap.end()};
	}

	template<class T, std::uint8_t BlockSize>
	typename Vector<T, BlockSize>::size_type Vector<T, BlockSize>::BucketCount() const noexcept
	{
		return blocks_.size();
	}

	template<class T, std::uint8_t BlockSize>
	typename Vector<T, BlockSize>::iterator Vector<T, BlockSize>::begin() noexcept
	{
		return {blocks_.begin(), blocks_.front().begin()};

	}

	template<class T, std::uint8_t BlockSize>
	typename Vector<T, BlockSize>::iterator Vector<T, BlockSize>::end() noexcept
	{
		return {blocks_.end(), blocks_.back().end()};
	}

}}
