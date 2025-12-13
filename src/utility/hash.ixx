/**
 *	@brief Hashing utilities for associative containers. Not suitable for cryptographic purposes.
 */

export module synodic.periapsis:utility.hash;

import std;

export namespace peri
{

	/**
	 * @brief fnv-1a_32 hash function of an input string
	 * @param view The input string
	 * @return Hashed value
	 */
	template<typename T>
	constexpr std::uint32_t FNV1a32(const T& input) noexcept
	{
		return 0;
	}

	/**
	 * @brief fnv-1a_32 hash function of an input string
	 * @param view The input string
	 * @return Hashed value
	 */
	template<>
	constexpr std::uint32_t FNV1a32(const std::string_view& view) noexcept
	{
		return view.size() ? (FNV1a32(std::string_view(view.data(), view.size() - 1)) ^ view.data()[view.size() - 1]) *
								 16'777'619u :
							 2'166'136'261u;
	}

	/**
	 * @brief Concept to check if an invocable type is a hashing invocable
	 */
	template<typename Fn, typename T>
	concept hashable = std::regular_invocable<Fn, T> && std::is_integral_v<std::invoke_result_t<Fn, T>>;

	/**
	 * @brief Wrapper around a hashed value for type safety
	 * @tparam Integral
	 */
	template<typename Integral>
		requires std::is_integral_v<Integral>
	class BasicHash
	{
	public:
		constexpr BasicHash() noexcept :
			hash_(0)
		{
		}

		template<typename T>
		explicit constexpr BasicHash(const T& value) noexcept :
			hash_(FNV1a32(value))
		{
		}

		constexpr BasicHash(const BasicHash& other)				   = default;
		constexpr BasicHash(BasicHash&& other) noexcept			   = default;
		constexpr BasicHash& operator=(const BasicHash& other)	   = default;
		constexpr BasicHash& operator=(BasicHash&& other) noexcept = default;

		constexpr bool operator==(const BasicHash& other) const = default;
		auto operator<=>(const BasicHash&) const				= default;

		explicit(false) operator Integral() const noexcept
		{
			return hash_;
		}

	private:
		Integral hash_;
	};

	using Hash = BasicHash<std::uint32_t>;

	using Hash8	 = BasicHash<std::uint8_t>;
	using Hash16 = BasicHash<std::uint16_t>;
	using Hash32 = BasicHash<std::uint32_t>;
	using Hash64 = BasicHash<std::uint64_t>;

}

/**
 * @brief std::hash specialization for BasicHash that allows it to be used in associative containers
 */
export template<typename Integral>
struct std::hash<peri::BasicHash<Integral>>
{
	std::size_t operator()(const peri::BasicHash<Integral>& hash) const noexcept
	{
		return static_cast<std::size_t>(hash);
	}
};
