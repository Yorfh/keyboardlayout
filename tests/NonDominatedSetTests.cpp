#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "NonDominatedSet.hpp"
#include <array>
using namespace testing;

template<typename T>
void testEqual(const T& expected, const NonDominatedSet<1>::SolutionsVector& actual)
{
	ASSERT_EQ(expected.size(), actual.size());
	auto ii = std::begin(expected);
	for (auto&& i : actual)
	{
		auto jj = std::begin(ii->second);
		for (auto&& j : i.second)
		{
			EXPECT_EQ(*jj, j);
			++jj;
		}
		++ii;
	}
}

TEST(NonDominatedSetTests, SimpleOneDimensionalOneValue)
{
	std::array<std::pair<Keyboard<1>, std::array<float, 1>>, 1> input{ { {Keyboard<1>({1}), {1.0}} } };
	NonDominatedSet<1> s(std::begin(input), std::end(input));
	ASSERT_EQ(1, s.size()); 
	testEqual(input, s.getResult());
}

TEST(NonDominatedSetTests, OneDimensionalTwoValues)
{
	std::array<std::pair<Keyboard<1>, std::array<float, 1>>, 2> input{ { {Keyboard<1>({1}), {1.0}}, {Keyboard<1>({2}), {2.0}} } };
	NonDominatedSet<1> s(std::begin(input), std::end(input));
	ASSERT_EQ(1, s.size()); 
	std::array<std::pair<Keyboard<1>, std::array<float, 1>>, 1> res{ { {Keyboard<1>({2}), {2.0} } } };
	testEqual(res, s.getResult());
}

TEST(NonDominatedSetTests, OneDimensionalTwoDifferentOrder)
{
	std::array<std::pair<Keyboard<1>, std::array<float, 1>>, 2> input{ { {Keyboard<1>({1}), {2.0} }, {Keyboard<1>({2}), {1.0} } } };
	NonDominatedSet<1> s(std::begin(input), std::end(input));
	ASSERT_EQ(1, s.size()); 
	std::array<std::pair<Keyboard<1>, std::array<float, 1>>, 1> res{ { {Keyboard<1>({2}), {2.0} } } };
	testEqual(res, s.getResult());
}

TEST(NonDominatedSetTests, OneDimensionalThreeValues1)
{
	using A = std::pair<Keyboard<1>, std::array<float, 1>>;
	A a = { Keyboard<1>({1}), { 1.0f } };
	A b = { Keyboard<1>({2}), { 2.0f } };
	A c = { Keyboard<1>({3}), { 3.0f } };
	std::array<A, 3> input{ a, b, c };
	NonDominatedSet<1> s(std::begin(input), std::end(input));
	ASSERT_EQ(1, s.size()); 
	std::array<A, 1> res{ c };
	testEqual(res, s.getResult());
}

TEST(NonDominatedSetTests, OneDimensionalThreeValues2)
{
	using A = std::pair<Keyboard<1>, std::array<float, 1>>;
	A a = { Keyboard<1>({1}), { 1.0f } };
	A b = { Keyboard<1>({2}), { 2.0f } };
	A c = { Keyboard<1>({3}), { 3.0f } };
	std::array<A, 3> input{  a, c, b };
	NonDominatedSet<1> s(std::begin(input), std::end(input));
	ASSERT_EQ(1, s.size()); 
	std::array<A, 1> res{ c };
	testEqual(res, s.getResult());
}

TEST(NonDominatedSetTests, OneDimensionalThreeValues3)
{
	using A = std::pair<Keyboard<1>, std::array<float, 1>>;
	A a = { Keyboard<1>({1}), { 1.0f } };
	A b = { Keyboard<1>({2}), { 2.0f } };
	A c = { Keyboard<1>({3}), { 3.0f } };
	std::array<A, 3> input{ b, a, c };
	NonDominatedSet<1> s(std::begin(input), std::end(input));
	ASSERT_EQ(1, s.size()); 
	std::array<A, 1> res{ c };
	testEqual(res, s.getResult());
}

TEST(NonDominatedSetTests, OneDimensionalThreeValues4)
{
	using A = std::pair<Keyboard<1>, std::array<float, 1>>;
	A a = { Keyboard<1>({1}), { 1.0f } };
	A b = { Keyboard<1>({2}), { 2.0f } };
	A c = { Keyboard<1>({3}), { 3.0f } };
	std::array<A, 3> input{ b, c, a };
	NonDominatedSet<1> s(std::begin(input), std::end(input));
	ASSERT_EQ(1, s.size()); 
	std::array<A, 1> res{ c };
	testEqual(res, s.getResult());
}

TEST(NonDominatedSetTests, OneDimensionalThreeValues5)
{
	using A = std::pair<Keyboard<1>, std::array<float, 1>>;
	A a = { Keyboard<1>({1}), { 1.0f } };
	A b = { Keyboard<1>({2}), { 2.0f } };
	A c = { Keyboard<1>({3}), { 3.0f } };
	std::array<A, 3> input{ c, a, b };
	NonDominatedSet<1> s(std::begin(input), std::end(input));
	ASSERT_EQ(1, s.size()); 
	std::array<A, 1> res{ c };
	testEqual(res, s.getResult());
}

TEST(NonDominatedSetTests, OneDimensionalThreeValues6)
{
	using A = std::pair<Keyboard<1>, std::array<float, 1>>;
	A a = { Keyboard<1>({1}), { 1.0f } };
	A b = { Keyboard<1>({2}), { 2.0f } };
	A c = { Keyboard<1>({3}), { 3.0f } };
	std::array<A, 3> input{ c, b, a };
	NonDominatedSet<1> s(std::begin(input), std::end(input));
	ASSERT_EQ(1, s.size()); 
	std::array<A, 1> res{ c };
	testEqual(res, s.getResult());
}

TEST(NonDominatedSetTests, TwoDimensional)
{
	using A = std::pair<Keyboard<1>, std::array<float, 2>>;
	A a = { Keyboard<1>({1}), { 4.0f, 1.0f } };
	A b = { Keyboard<1>({2}), { 3.0f, 1.0f } };
	A c = { Keyboard<1>({3}), { 2.0f, 1.0f } };
	A d = { Keyboard<1>({4}), { 1.0f, 1.0f } };
	A e = { Keyboard<1>({5}), { 3.0f, 2.0f } };
	A f = { Keyboard<1>({6}), { 2.0f, 2.0f } };
	A g = { Keyboard<1>({7}), { 1.0f, 2.0f } };
	A h = { Keyboard<1>({8}), { 2.0f, 3.0f } };
	std::array<A, 8> input{ a, b, c, d, e, f, g, h };
	// 4
	// 3   h 
	// 2 g f e
	// 1 d c b a
	//   1 2 3 4
	NonDominatedSet<1> s(std::begin(input), std::end(input));
	ASSERT_EQ(3, s.size()); 
	std::array<A, 3> res{ a, e, h };
	testEqual(res, s.getResult());
}

TEST(NonDominatedSetTests, TwoDimensional2)
{
	using A = std::pair<Keyboard<1>, std::array<float, 2>>;
	A a = { Keyboard<1>({1}), { 4.0f, 1.0f } };
	A b = { Keyboard<1>({2}), { 3.0f, 2.0f } };
	A c = { Keyboard<1>({3}), { 2.0f, 1.0f } };
	A d = { Keyboard<1>({4}), { 1.0f, 1.0f } };
	A e = { Keyboard<1>({5}), { 3.0f, 3.0f } };
	A f = { Keyboard<1>({6}), { 2.0f, 2.0f } };
	A g = { Keyboard<1>({7}), { 1.0f, 2.0f } };
	A h = { Keyboard<1>({8}), { 2.0f, 3.0f } };
	// 4
	// 3   h e
	// 2 g f b
	// 1 d c   a
	//   1 2 3 4
	std::array<A, 8> input{ a, b, c, d, e, f, g, h };
	NonDominatedSet<1> s(std::begin(input), std::end(input));
	ASSERT_EQ(2, s.size()); 
	std::array<A, 2> res{ a, e };
	testEqual(res, s.getResult());
}