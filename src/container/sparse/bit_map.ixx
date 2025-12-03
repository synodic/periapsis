export module synodic.library:container.sparse.bit_map;

import std;

export namespace synodic::sparse
{
	/**
	 * @brief Faster creation than unordered_set, but suffers from worse intersection
	 * @tparam T  The type to store
	 * @tparam N  The capacity
	 */
	template<class T, std::uint8_t N>
	class BitMap final
	{
		static_assert(N % 4 == 0, "N must be byte aligned.");

	public:
		using size_type = std::uint8_t;
		using iterator	= typename std::vector<T>::iterator;

		BitMap()  = default;
		~BitMap() = default;

		T& operator[](size_type);
		[[nodiscard]] bool Test(size_type) const;

		template<typename... Args>
		std::pair<iterator, bool> Emplace(size_type, Args&&...);
		std::pair<iterator, bool> Insert(size_type position, const T& value);

		void Clear();

		iterator Find(const size_type position);

		iterator begin() noexcept;
		iterator end() noexcept;

	private:
		[[nodiscard]] std::uint8_t CountPosition(const size_type position) const;

		std::bitset<N> mapping_;
		std::vector<T> data_;
	};

	template<class T, std::uint8_t N>
	T& BitMap<T, N>::operator[](size_type position)
	{
		assert(mapping_.test(position));

		uint8_t pos = CountPosition(position) - 1;

		assert(pos < data_.size());

		return data_[pos];
	}

	template<class T, std::uint8_t N>
	bool BitMap<T, N>::Test(size_type position) const
	{
		return mapping_.test(position);
	}

	template<class T, std::uint8_t N>
	template<typename... Args>
	std::pair<typename BitMap<T, N>::iterator, bool> BitMap<T, N>::Emplace(size_type position, Args&&... args)
	{
		if (Test(position))
		{
			return {operator[](position), false};
		}

		std::uint8_t pos = CountPosition(position);
		mapping_.set(position);

		auto iter = data_.emplace(data_.begin() + pos, std::forward<Args>(args)...);

		return {iter, true};
	}

	template<class T, std::uint8_t N>
	std::pair<typename BitMap<T, N>::iterator, bool> BitMap<T, N>::Insert(size_type position, const T& value)
	{
		if (Test(position))
		{
			return {data_.begin() + position, false};
		}

		std::uint8_t pos = CountPosition(position);
		mapping_.set(position);

		auto iter = data_.insert(data_.begin() + pos, value);

		return {iter, true};
	}

	template<class T, std::uint8_t N>
	void BitMap<T, N>::Clear()
	{
		mapping_.reset();
		data_.resize(0);
	}

	template<class T, std::uint8_t N>
	typename BitMap<T, N>::iterator BitMap<T, N>::Find(const size_type position)
	{
		if (!Test(position))
		{
			return data_.end();
		}

		std::uint8_t pos = CountPosition(position);

		return data_.begin() + pos;
	}

	template<class T, std::uint8_t N>
	typename BitMap<T, N>::iterator BitMap<T, N>::begin() noexcept
	{
		return data_.begin();
	}

	template<class T, std::uint8_t N>
	typename BitMap<T, N>::iterator BitMap<T, N>::end() noexcept
	{
		return data_.end();
	}

	template<class T, std::uint8_t N>
	std::uint8_t BitMap<T, N>::CountPosition(const size_type position) const
	{
		std::bitset<N> mask;
		mask = ~(~mask << position) & mapping_;

		// TODO: Replace with `popcount`
		return static_cast<uint8_t>(mask.count());
	}

}
