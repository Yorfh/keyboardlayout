#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "NonDominatingSet.hpp"
#include <array>
using namespace testing;

template<typename T>
void testEqual(T& res, NonDominatingSet& s)
{
	auto ii = std::begin(res);
	for (auto&& i : s)
	{
		auto jj = std::begin(*ii);
		for (auto&& j : i)
		{
			EXPECT_EQ(*jj, j);
			++jj;
		}
		++ii;
	}
}

TEST(NonDominatingSetTests, SimpleOneDimensionalOneValue)
{
	std::array<std::array<float, 1>, 1> input{ { {1.0} } };
	NonDominatingSet s(std::begin(input), std::end(input));
	ASSERT_EQ(1, s.size()); 
	testEqual(input, s);
}

TEST(NonDominatingSet, OneDimensionalTwoValues)
{
	std::array<std::array<float, 1>, 2> input{ { {1.0}, {2.0} } };
	NonDominatingSet s(std::begin(input), std::end(input));
	ASSERT_EQ(1, s.size()); 
	std::array<std::array<float, 1>, 1> res{ { {2.0} } };
	testEqual(res, s);
}

TEST(NonDominatingSet, OneDimensionalTwoDifferentOrder)
{
	std::array<std::array<float, 1>, 2> input{ { {2.0}, {1.0} } };
	NonDominatingSet s(std::begin(input), std::end(input));
	ASSERT_EQ(1, s.size()); 
	std::array<std::array<float, 1>, 1> res{ { {2.0} } };
	testEqual(res, s);
}

TEST(NonDominatingSet, TwoDimensional)
{
	using A = std::array<float, 2>;
	A a = { 4.0f, 1.0f };
	A b = { 3.0f, 1.0f };
	A c = { 2.0f, 1.0f };
	A d = { 1.0f, 1.0f };
	A e = { 3.0f, 2.0f };
	A f = { 2.0f, 2.0f };
	A g = { 1.0f, 2.0f };
	A h = { 2.0f, 3.0f };
	std::array<std::array<float, 2>, 8> input{ a, b, c, d, e, f, g, h };
	NonDominatingSet s(std::begin(input), std::end(input));
	ASSERT_EQ(3, s.size()); 
	std::array<std::array<float, 2>, 3> res{ a, e, h };
	testEqual(res, s);
}

TEST(NonDominatingSet, TwoDimensional2)
{
	using A = std::array<float, 2>;
	A a = { 4.0f, 1.0f };
	A b = { 3.0f, 2.0f };
	A c = { 2.0f, 1.0f };
	A d = { 1.0f, 1.0f };
	A e = { 3.0f, 3.0f };
	A f = { 2.0f, 2.0f };
	A g = { 1.0f, 2.0f };
	A h = { 2.0f, 3.0f };
	//This doesn't seem right
	std::array<std::array<float, 2>, 8> input{ a, b, c, d, e, f, g, h };
	NonDominatingSet s(std::begin(input), std::end(input));
	ASSERT_EQ(3, s.size()); 
	std::array<std::array<float, 2>, 3> res{ a, b, h };
	testEqual(res, s);
}