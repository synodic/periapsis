/**
 *	@brief Hashing utilities for associative containers. Not suitable for cryptographic purposes.
 */

export module synodic.periapsis:utility.hash;

import std;

export namespace peri
{
	/**
	 * @brief FNV-1a hash algorithm for contiguous byte ranges
	 * @tparam Integral The result type (uint32_t or uint64_t)
	 */
	template<std::unsigned_integral Integral>
	struct FNV1a;

	template<>
	struct FNV1a<std::uint32_t>
	{
		static constexpr std::uint32_t OFFSET_BASIS = 2'166'136'261u;
		static constexpr std::uint32_t PRIME        = 16'777'619u;

		static constexpr std::uint32_t Hash(const std::string_view view) noexcept
		{
			std::uint32_t result = OFFSET_BASIS;
			for (const char c: view)
			{
				result ^= static_cast<std::uint8_t>(c);
				result *= PRIME;
			}
			return result;
		}
	};

	template<>
	struct FNV1a<std::uint64_t>
	{
		static constexpr std::uint64_t OFFSET_BASIS = 14'695'981'039'346'656'037ull;
		static constexpr std::uint64_t PRIME        = 1'099'511'628'211ull;

		static constexpr std::uint64_t Hash(const std::string_view view) noexcept
		{
			std::uint64_t result = OFFSET_BASIS;
			for (const char c: view)
			{
				result ^= static_cast<std::uint8_t>(c);
				result *= PRIME;
			}
			return result;
		}
	};

	// Smaller types use truncated 32-bit Hash
	template<>
	struct FNV1a<std::uint8_t>
	{
		static constexpr std::uint8_t Hash(const std::string_view view) noexcept
		{
			return static_cast<std::uint8_t>(FNV1a<std::uint32_t>::Hash(view));
		}
	};

	template<>
	struct FNV1a<std::uint16_t>
	{
		static constexpr std::uint16_t Hash(const std::string_view view) noexcept
		{
			return static_cast<std::uint16_t>(FNV1a<std::uint32_t>::Hash(view));
		}
	};

	/**
	 * @brief Concept to check if an invocable type is a hashing invocable
	 */
	template<typename Fn, typename T>
	concept hashable = std::regular_invocable<Fn, T> && std::unsigned_integral<std::invoke_result_t<Fn, T>>;

	/**
	 * @brief Wrapper around a hashed value for type safety
	 * @tparam Integral The unsigned integral type for the hash
	 */
	template<std::unsigned_integral Integral>
	class BasicHash
	{
	public:
		constexpr BasicHash() noexcept = default;

		explicit constexpr BasicHash(std::string_view value) noexcept :
			hash_(FNV1a<Integral>::hash(value))
		{
		}

		constexpr bool operator==(const BasicHash&) const  = default;
		constexpr auto operator<=>(const BasicHash&) const = default;

		constexpr explicit(false) operator Integral() const noexcept
		{
			return hash_;
		}

	private:
		Integral hash_{};
	};

	using Hash = BasicHash<std::uint32_t>;

	using Hash8  = BasicHash<std::uint8_t>;
	using Hash16 = BasicHash<std::uint16_t>;
	using Hash32 = BasicHash<std::uint32_t>;
	using Hash64 = BasicHash<std::uint64_t>;
}

/**
 * @brief std::hash specialization for BasicHash that allows it to be used in associative containers
 */
export template<std::unsigned_integral Integral>
struct std::hash<peri::BasicHash<Integral>>
{
	constexpr std::size_t operator()(const peri::BasicHash<Integral>& hash) const noexcept
	{
		return static_cast<std::size_t>(hash);
	}
};
