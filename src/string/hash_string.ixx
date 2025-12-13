export module synodic.periapsis:string.hash_string;

import std;

export namespace peri
{
	class HashString final
	{
	public:
		using HashType = std::uint64_t;

		explicit constexpr HashString(std::string_view);

		constexpr explicit(false) operator std::string_view() const;
		constexpr explicit(false) operator HashType() const;

		constexpr bool operator==(const HashString& other) const;
		constexpr bool operator!=(const HashString& other) const;

	private:
		constexpr static HashType Hash(HashType, std::string_view);

		constexpr static HashType OFFSET = 0xcbf29ce484222325;
		constexpr static HashType PRIME = 0x100000001b3;

		const HashType hash_;
		std::string_view string_;
	};

	constexpr HashString::HashString(const std::string_view string) : hash_(Hash(OFFSET, string)), string_(string)
	{
	}

	// cast of the data as a readable string
	constexpr HashString::operator std::string_view() const
	{
		return string_;
	}

	// cast of the data as a numerical value
	constexpr HashString::operator HashType() const
	{
		return hash_;
	}

	constexpr bool HashString::operator==(const HashString& other) const
	{
		return hash_ == other.hash_;
	}

	constexpr bool HashString::operator!=(const HashString& other) const
	{
		return hash_ != other.hash_;
	}

	constexpr HashString::HashType HashString::Hash(const HashType offset, const std::string_view string)
	{
		return !string.empty() ? offset : Hash((offset ^ string[0]) * PRIME, string.substr(1));
	}

	constexpr HashString operator"" _hashed(const char* str, std::size_t)
	{
		return HashString{str};
	}
}
