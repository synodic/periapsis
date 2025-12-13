export module synodic.periapsis:string.inplace_string;

import std;
import :type.traits;

export namespace peri
{

	/**
	 * @brief A string with an internal buffer. Mimics the api of std::basic_string while supporting modifications.
	 *	Implements the interface of std::fixed_string (p3094) while allowing for modifications and a smaller storage
	 *	footprint.
	 */
	template<typename CharT, std::size_t N, typename Traits = std::char_traits<CharT>>
	class BasicInplaceString
	{
	public:
		using traits_type			 = Traits;
		using value_type			 = CharT;
		using pointer				 = value_type*;
		using const_pointer			 = const value_type*;
		using reference				 = value_type&;
		using const_reference		 = const value_type&;
		using const_iterator		 = const value_type*;
		using iterator				 = const_iterator;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using reverse_iterator		 = const_reverse_iterator;
		using size_type				 = std::size_t;
		using difference_type		 = std::ptrdiff_t;

		constexpr BasicInplaceString() noexcept :
			data_ {},
			size_(0) {};

		template<std::convertible_to<CharT>... Chars>
			requires(sizeof...(Chars) <= N) && (... && !std::is_pointer_v<Chars>)
		constexpr explicit BasicInplaceString(Chars... chars) noexcept :
			data_ {chars..., CharT {}},	 // Ends the string with a null terminator
			size_(sizeof...(Chars))
		{
		}

		template<std::size_t Size>
			requires(Size - 1 <= N)
		consteval explicit(false) BasicInplaceString(const CharT (&data)[Size]) noexcept :
			data_(),
			size_(Size - 1)
		{
			std::copy_n(data, Size - 1, data_.begin());
			std::fill(data_.begin() + Size, data_.end(), CharT());	// Will always fill at least the built-in terminator
		}

		constexpr explicit BasicInplaceString(const std::basic_string_view<CharT, Traits> data) noexcept :
			data_(),
			size_(data.size())
		{
			std::copy_n(data.data(), size_, data_.begin());
			std::fill(data_.begin() + size_, data_.end(), CharT(0));	// Will always fill at least the built-in terminator
		}

		constexpr BasicInplaceString(const BasicInplaceString&) noexcept			= default;
		constexpr BasicInplaceString& operator=(const BasicInplaceString&) noexcept = default;

		[[nodiscard]] constexpr const_iterator begin() const noexcept
		{
			return data();
		}

		[[nodiscard]] constexpr const_iterator end() const noexcept
		{
			return data() + size_;
		}

		[[nodiscard]] constexpr const_iterator cbegin() const noexcept
		{
			return begin();
		}

		[[nodiscard]] constexpr const_iterator cend() const noexcept
		{
			return end();
		}

		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept
		{
			return const_reverse_iterator(end());
		}

		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept
		{
			return const_reverse_iterator(begin());
		}

		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
		{
			return rbegin();
		}

		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
		{
			return rend();
		}

		constexpr size_type size()
		{
			return size_;
		}

		constexpr size_type length()
		{
			return size_;
		}

		[[nodiscard]] constexpr const_reference operator[](size_type pos) const
		{
			return data_[pos];
		}

		[[nodiscard]] constexpr const_reference front() const
		{
			return data_.front();
		}

		[[nodiscard]] constexpr const_reference back() const
		{
			return data_.back();
		}

		[[nodiscard]] constexpr const_pointer c_str() const noexcept
		{
			return data();
		}

		[[nodiscard]] constexpr const_pointer data() const noexcept
		{
			return static_cast<const_pointer>(data_.data());
		}

		[[nodiscard]] constexpr std::basic_string_view<CharT, Traits> view() const noexcept
		{
			return std::basic_string_view<CharT, Traits>(cbegin(), cend());
		}

		[[nodiscard]] constexpr explicit(false) operator std::basic_string_view<CharT, Traits>() const noexcept
		{
			return view();
		}

		template<size_t N2>
		[[nodiscard]] friend constexpr bool
			operator==(const BasicInplaceString& lhs, const BasicInplaceString<CharT, N2, Traits>& rhs)
		{
			return lhs.view() == rhs.view();
		}

		template<size_t N2>
		[[nodiscard]] friend consteval bool operator==(const BasicInplaceString& lhs, const CharT (&rhs)[N2])
		{
			return lhs.view() == std::basic_string_view<CharT, Traits>(rhs, rhs + N2 - 1);
		}

		template<size_t N2>
		[[nodiscard]] friend constexpr auto
			operator<=>(const BasicInplaceString& lhs, const BasicInplaceString<CharT, N2, Traits>& rhs)
		{
			return lhs.view() <=> rhs.view();
		}

		template<size_t N2>
		[[nodiscard]] friend constexpr auto operator<=>(const BasicInplaceString& lhs, const CharT (&rhs)[N2])
		{
			return lhs.view() <=> std::basic_string_view<CharT, Traits>(rhs, rhs + N2 - 1);
		}

		friend std::basic_ostream<CharT, Traits>&
			operator<<(std::basic_ostream<CharT, Traits>& os, const BasicInplaceString& str)
		{
			return os << str.c_str();
		}

	private:
		std::array<CharT, N + 1> data_;
		MinimalIntegerType<N> size_;
	};

	template<typename T, typename... Ts>
	concept one_of = (false || ... || std::same_as<T, Ts>);

	// Deduction Guides

	template<one_of<char, char8_t, char16_t, char32_t, wchar_t> CharT, typename... Rest>
	BasicInplaceString(CharT, Rest...) -> BasicInplaceString<CharT, 1 + sizeof...(Rest)>;

	template<typename CharT, std::size_t N>
	BasicInplaceString(const CharT (&str)[N]) -> BasicInplaceString<CharT, N - 1>;

	template<std::size_t N>
	using InplaceString = BasicInplaceString<char, N>;
	template<std::size_t N>
	using InplaceU8String = BasicInplaceString<char8_t, N>;
	template<std::size_t N>
	using InplaceU16String = BasicInplaceString<char16_t, N>;
	template<std::size_t N>
	using InplaceU32String = BasicInplaceString<char32_t, N>;
	template<std::size_t N>
	using InplaceWString = BasicInplaceString<wchar_t, N>;

}

export template<std::size_t N>
struct std::hash<peri::InplaceString<N>> : std::hash<std::string_view>
{
};

export template<std::size_t N>
struct std::hash<peri::InplaceU8String<N>> : std::hash<std::u8string_view>
{
};

export template<std::size_t N>
struct std::hash<peri::InplaceU16String<N>> : std::hash<std::u16string_view>
{
};

export template<std::size_t N>
struct std::hash<peri::InplaceU32String<N>> : std::hash<std::u32string_view>
{
};

export template<std::size_t N>
struct std::hash<peri::InplaceWString<N>> : std::hash<std::wstring_view>
{
};

export template<typename CharT, std::size_t N>
struct std::formatter<peri::BasicInplaceString<CharT, N>> :
	std::formatter<std::basic_string_view<CharT>>
{
	template<typename FormatContext>
	auto format(const peri::BasicInplaceString<CharT, N>& str, FormatContext& ctx) const
		-> decltype(ctx.out())
	{
		return std::formatter<std::basic_string_view<CharT>>::format(std::basic_string_view<CharT>(str), ctx);
	}
};
