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