import std;
import synodic.honesty.test;

import synodic.library;

using namespace honesty::test;
using namespace honesty::test::literals;

namespace
{
	Suite SUITE(
		"utility",
		[]() -> Generator
		{
			co_yield "nth_element"_test = [](const Requirements& requirements)
			{
				static_assert(1 == synodic::nth_element<0>(1));
				static_assert(1 == synodic::nth_element<0>(1, 2));
				static_assert(2 == synodic::nth_element<1>(1, 2));
				static_assert(1 == synodic::nth_element<0>(1, 2, 3));
				static_assert(2 == synodic::nth_element<1>(1, 2, 3));
			};
		}
		);
	SuiteRegistrar _(SUITE);
}
