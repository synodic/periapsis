import std;
import synodic.honesty.test;
import synodic.library;

using namespace honesty::test;
using namespace honesty::test::literals;

struct TestStruct
{
};

class TestClass
{
public:
	TestClass(std::vector<int> parameter1, TestStruct parameter2, int parameter3) :
		data1_(std::move(parameter1)), data2_(parameter2), data3_(parameter3)
	{
	}

	[[nodiscard]] int GetData3() const
	{
		return data3_;
	}

private:
	std::vector<int> data1_;
	TestStruct data2_;
	int data3_;
};

class ConstTestClass
{
public:
	ConstTestClass(const std::vector<int> parameter1, const TestStruct parameter2, const int parameter3) :
		data1_(parameter1), data2_(parameter2), data3_(parameter3)
	{
	}

private:
	const std::vector<int> data1_;
	const TestStruct data2_;
	const int data3_;
};

class ReferenceTestClass
{
public:
	ReferenceTestClass(std::vector<int>& parameter1, TestStruct& parameter2, int& parameter3) :
		data1_(parameter1), data2_(parameter2), data3_(parameter3)
	{
	}

private:
	std::vector<int>& data1_;
	TestStruct& data2_;
	int& data3_;
};

class PointerTestClass
{
public:
	PointerTestClass(std::vector<int>* parameter1, TestStruct* parameter2, int* parameter3) :
		data1_(parameter1), data2_(parameter2), data3_(parameter3)
	{
	}

private:
	std::vector<int>* data1_;
	TestStruct* data2_;
	int* data3_;
};

//namespace
//{
//	Suite SUITE(
//		"type_traits",
//		[]() -> Generator
//		{
//			co_yield "value"_test = [](const Requirements& requirements)
//			{
//				using ConstructorType = synodic::ConstructorParameters<TestClass>;
//				requirements.ExpectEquals(std::is_same_v<ConstructorType, std::tuple<std::vector<int>, TestData, int>>));
//			};
//
//			co_yield "const"_test = [](const Requirements& requirements)
//			{
//				using ConstructorType = synodic::ConstructorParameters<ConstTestClass>;
//				requirements.ExpectEquals(
//					(std::is_same_v<ConstructorType, std::tuple<std::vector<int>, TestData, int>>));
//			};
//		
//
//			co_yield "ref"_test = [](const Requirements& requirements)
//				{
//					// using ConstructorType = synodic::ConstructorParameters<ReferenceTestClass>;
//				// requirements.Equal(std::is_same_v<ConstructorType, std::tuple<std::vector<int>, TestData, int>>);
//				};
//			}
//
//			co_yield "ptr"_test = [](const Requirements& requirements)
//			{
//				// using ConstructorType = synodic::ConstructorParameters<PointerTestClass>;
//				// requirements.Equal(std::is_same_v<ConstructorType, std::tuple<std::vector<int>, TestData, int>>);
//			};
//
//		});
//	SuiteRegistrar _(SUITE);
//}
