export module synodic.honesty.utility:string;

import std;
import inplace_vector;

namespace synodic::honesty::utility
{

	export template<typename CharT, std::size_t MaxSize, typename Traits = std::char_traits<CharT>>
	class BasicFixedString
	{
	public:
		using StringViewType = std::basic_string_view<CharT, Traits>;

		explicit(false) constexpr BasicFixedString(const CharT (&str)[MaxSize]) :
			data_(MaxSize - 1)
		{
			std::copy(std::begin(str), std::end(str), std::begin(data_));
		}

		explicit(false) constexpr BasicFixedString(std::span<const CharT, MaxSize> str) :
			data_(MaxSize - 1)
		{
			std::copy(std::begin(str), std::end(str), std::begin(data_));
		}

		explicit(false) constexpr BasicFixedString(std::basic_string_view<CharT> str) :
			data_(str.size())
		{
			std::copy(std::begin(str), std::end(str), std::begin(data_));
		}

		BasicFixedString(const BasicFixedString& other)				   = default;
		BasicFixedString(BasicFixedString&& other) noexcept			   = default;
		BasicFixedString& operator=(const BasicFixedString& other)	   = default;
		BasicFixedString& operator=(BasicFixedString&& other) noexcept = default;

		/**
		 * @brief Explicit conversion to std::basic_string_view
		 */
		constexpr std::basic_string_view<CharT> View() const noexcept
		{
			return {data_.data(), data_.size()};
		}

		[[nodiscard]] explicit constexpr operator std::basic_string_view<CharT>() const noexcept
		{
			return {data_.data(), data_.size()};
		}

	private:
		std::inplace_vector<CharT, MaxSize> data_;
	};

	export template<typename CharT, typename Traits, std::size_t SizeLeft, std::size_t SizeRight>
	[[nodiscard]] constexpr bool operator==(
		const BasicFixedString<CharT, SizeLeft, Traits>& lhs,
		const BasicFixedString<CharT, SizeRight, Traits>& rhs)
	{
		if constexpr (SizeLeft != SizeRight)
		{
			return false;
		}

		using ViewType = typename BasicFixedString<CharT, SizeLeft, Traits>::StringViewType;
		return static_cast<ViewType>(rhs) == lhs;
	}

	export template<typename CharT, typename Traits, std::size_t SizeLeft>
	[[nodiscard]] constexpr bool operator==(
		const BasicFixedString<CharT, SizeLeft, Traits>& lhs,
		std::type_identity_t<std::basic_string_view<CharT, Traits>> rhs)
	{
		using ViewType = typename BasicFixedString<CharT, SizeLeft, Traits>::StringViewType;
		return static_cast<ViewType>(lhs) == rhs;
	}

	export template<typename CharT, typename Traits, std::size_t SizeLeft, std::size_t SizeRight>
	[[nodiscard]] constexpr auto operator<=>(
		const BasicFixedString<CharT, SizeLeft, Traits>& lhs,
		const BasicFixedString<CharT, SizeRight, Traits>& rhs)
	{
		using ViewType = typename BasicFixedString<CharT, SizeLeft, Traits>::StringViewType;
		return static_cast<ViewType>(lhs) <=> rhs;
	}

	export template<typename CharT, typename Traits, std::size_t SizeLeft>
	[[nodiscard]] constexpr auto operator<=>(
		const BasicFixedString<CharT, SizeLeft, Traits>& lhs,
		std::type_identity_t<std::basic_string_view<CharT, Traits>> rhs)
	{
		using ViewType = typename BasicFixedString<CharT, SizeLeft, Traits>::StringViewType;
		return static_cast<ViewType>(lhs) <=> rhs;
	}

	export template<std::size_t Size>
	using FixedString = BasicFixedString<char, Size>;

	export template<std::size_t Size>
	using FixedStringU8 = BasicFixedString<char8_t, Size>;

	export template<std::size_t Size>
	using FixedStringU16 = BasicFixedString<char16_t, Size>;

	export template<std::size_t Size>
	using FixedStringU32 = BasicFixedString<char32_t, Size>;

}

export template<typename CharT, std::size_t Size, typename Traits>
struct std::formatter<synodic::honesty::utility::BasicFixedString<CharT, Size, Traits>, CharT> :
	std::formatter<std::basic_string_view<CharT, Traits>, CharT>
{
	template<typename Context>
	auto format(const synodic::honesty::utility::BasicFixedString<CharT, Size, Traits>& string, Context& context) const
		-> typename Context::iterator
	{
		using ViewType = typename synodic::honesty::utility::BasicFixedString<CharT, Size, Traits>::StringViewType;
		return std::formatter<std::basic_string_view<CharT, Traits>, CharT>::format(
			static_cast<ViewType>(string),
			context);
	}
};
