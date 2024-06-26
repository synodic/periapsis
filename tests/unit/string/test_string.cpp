import std;
import synodic.honesty.test;
import synodic.honesty.utility;

using namespace synodic::honesty::test;
using namespace synodic::honesty::test::literals;

namespace
{
	Suite stringSuite(
		"string",
		[]() -> Generator
		{
			co_yield "construction"_test = [](const Requirements& requirements)
			{
				{
					// View
					constexpr std::string_view name = "test";
					constexpr synodic::honesty::utility::FixedString<name.size()> str(name);

					requirements.Expect(not str.View().empty());
					requirements.Expect(str.View() == "test");
				}
				{
					// Direct pointer with terminating 0
					constexpr const char* name = "test";
					constexpr synodic::honesty::utility::FixedString<4> str(name);

					requirements.Expect(not str.View().empty());
					requirements.Expect(str.View() == "test");
				}
				{
					// Pointer with terminating 0
					constexpr const char (&name)[5] = "test";
					constexpr synodic::honesty::utility::FixedString str(name);

					requirements.Expect(not str.View().empty());
					requirements.Expect(str.View() == "test");
				}
				{
					// Span with terminating 0
					constexpr const char (&base)[5] = "test";
					constexpr std::span name(base);
					constexpr synodic::honesty::utility::FixedString str(name);

					requirements.Expect(not str.View().empty());
					requirements.Expect(str.View() == "test");
				}
				{
					// Span without terminating 0
					constexpr const char (&base)[5] = "test";
					constexpr std::span second(base);
					constexpr std::span name = second.subspan(0, 4);
					constexpr synodic::honesty::utility::FixedString str(name);

					requirements.Expect(not str.View().empty());
					requirements.Expect(str.View() == "test");
				}
				{
					// Initializer list
					constexpr synodic::honesty::utility::FixedString str({"t", "e", "s", "t"});

					requirements.Expect(not str.View().empty());
					requirements.Expect(str.View() == "test");
				}
			};

			co_yield "equality"_test = [](const Requirements& requirements)
			{
				constexpr synodic::honesty::utility::FixedString str("test");
				constexpr synodic::honesty::utility::FixedStringU8 u8Str(u8"test");
				constexpr synodic::honesty::utility::FixedStringU16 u16Str(u"test");
				constexpr synodic::honesty::utility::FixedStringU32 u32Str(U"test");

				constexpr synodic::honesty::utility::FixedString strCopy("test");
				constexpr synodic::honesty::utility::FixedStringU8 u8StrCopy(u8"test");
				constexpr synodic::honesty::utility::FixedStringU16 u16StrCopy(u"test");
				constexpr synodic::honesty::utility::FixedStringU32 u32StrCopy(U"test");

				requirements.Expect(str == str);
				requirements.Expect(u8Str == u8Str);
				requirements.Expect(u16Str == u16Str);
				requirements.Expect(u32Str == u32Str);

				requirements.Expect(str == strCopy);
				requirements.Expect(u8Str == u8StrCopy);
				requirements.Expect(u16Str == u16StrCopy);
				requirements.Expect(u32Str == u32StrCopy);

				requirements.Expect(str.View() == std::string_view("test"));
				requirements.Expect(u8Str.View() == std::u8string_view(u8"test"));
				requirements.Expect(u16Str.View() == std::u16string_view(u"test"));
				requirements.Expect(u32Str.View() == std::u32string_view(U"test"));

				requirements.Expect(str == "test");
				requirements.Expect(u8Str == u8"test");
				requirements.Expect(u16Str == u"test");
				requirements.Expect(u32Str == U"test");

				requirements.Expect("test" == str);
				requirements.Expect(u8"test" == u8Str);
				requirements.Expect(u"test" == u16Str);
				requirements.Expect(U"test" == u32Str);
			};

			co_yield "comparison"_test = [](const Requirements& requirements)
			{
				constexpr synodic::honesty::utility::FixedString strA("a");
				constexpr synodic::honesty::utility::FixedStringU8 u8StrA(u8"a");
				constexpr synodic::honesty::utility::FixedStringU16 u16StrA(u"a");
				constexpr synodic::honesty::utility::FixedStringU32 u32StrA(U"a");

				constexpr synodic::honesty::utility::FixedString strB("b");
				constexpr synodic::honesty::utility::FixedStringU8 u8StrB(u8"b");
				constexpr synodic::honesty::utility::FixedStringU16 u16StrB(u"b");
				constexpr synodic::honesty::utility::FixedStringU32 u32StrB(U"b");

				requirements.Expect(strA < strB);
				requirements.Expect(u8StrA < u8StrB);
				requirements.Expect(u16StrA < u16StrB);
				requirements.Expect(u32StrA < u32StrB);

				requirements.Expect(strA < "b");
				requirements.Expect(u8StrA < u8"b");
				requirements.Expect(u16StrA < u"b");
				requirements.Expect(u32StrA < U"b");
			};
		});

	SuiteRegistrar _(stringSuite);
}
