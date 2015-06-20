#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "NonDominatedSet.hpp"
#include <array>
#include "boost/iterator/zip_iterator.hpp"
#include "boost/tuple/tuple.hpp"
using namespace testing;

template<typename T>
void testEqual(T& res, NonDominatedSet<1>& s)
{
	ASSERT_EQ(res.size(), s.size());
	auto ii = std::begin(res);
	auto b = boost::make_zip_iterator(boost::make_tuple(std::begin(res), std::begin(s)));
	auto e = boost::make_zip_iterator(boost::make_tuple(std::end(res), std::end(s)));
	auto r = boost::make_iterator_range(b, e);
	for (auto&& e : r)
	{
		auto b2 = boost::make_zip_iterator(boost::make_tuple(std::begin(e.get<0>()), std::begin(e.get<1>().solution())));
		auto e2 = boost::make_zip_iterator(boost::make_tuple(std::end(e.get<0>()), std::end(e.get<1>().solution())));
		auto solutions = boost::make_iterator_range(b2, e2);
		for (auto&& s : solutions)
		{
			EXPECT_EQ(s.get<0>(), s.get<1>());
		}
	}
}

TEST(NonDominatingSetTests, SimpleOneDimensionalOneValue)
{
	std::array<Keyboard<1>, 1> keyboards;
	std::array<std::array<float, 1>, 1> solutions{ 1.0 };
	NonDominatedSet<1> s(makeSolutions(keyboards, solutions));
	ASSERT_EQ(1, s.size()); 
	testEqual(solutions, s);
}

TEST(NonDominatingSetTests, OneDimensionalTwoValues)
{
	std::array<Keyboard<1>, 2> keyboards;
	std::array<std::array<float, 1>, 2> solutions{ {1.0, 2.0} };
	NonDominatedSet<1> s(makeSolutions(keyboards, solutions));
	ASSERT_EQ(1, s.size()); 
	std::array<std::array<float, 1>, 1> res{ 2.0 };
	testEqual(res, s);
}
/*
TEST(NonDominatingSetTests, OneDimensionalTwoDifferentOrder)
{
	std::array<Keyboard<1>, 2> keyboards;
	std::array<std::array<float, 1>, 2> solutions{ {2.0, 1.0} };
	NonDominatedSet<1> s(makeSolutions(keyboards, solutions));
	ASSERT_EQ(1, s.size()); 
	std::array<boost::tuple<Keyboard<1>, std::array<float, 1>>, 1> res{ { {Keyboard<1>(), {2.0} } } };
	testEqual(res, s);
}

TEST(NonDominatingSetTests, TwoDimensional)
{
	using A = boost::tuple<Keyboard<1>, std::array<float, 2>>;
	A a = { Keyboard<1>(), { 4.0f, 1.0f } };
	A b = { Keyboard<1>(), { 3.0f, 1.0f } };
	A c = { Keyboard<1>(), { 2.0f, 1.0f } };
	A d = { Keyboard<1>(), { 1.0f, 1.0f } };
	A e = { Keyboard<1>(), { 3.0f, 2.0f } };
	A f = { Keyboard<1>(), { 2.0f, 2.0f } };
	A g = { Keyboard<1>(), { 1.0f, 2.0f } };
	A h = { Keyboard<1>(), { 2.0f, 3.0f } };
	std::array<A, 8> input{ a, b, c, d, e, f, g, h };
	// 4
	// 3   h 
	// 2 g f e
	// 1 d c b a
	//   1 2 3 4
	NonDominatedSet<1> s(input);
	ASSERT_EQ(3, s.size()); 
	std::array<A, 3> res{ a, e, h };
	testEqual(res, s);
}

TEST(NonDominatingSetTests, TwoDimensional2)
{
	using A = boost::tuple<Keyboard<1>, std::array<float, 2>>;
	A a = { Keyboard<1>(), { 4.0f, 1.0f } };
	A b = { Keyboard<1>(), { 3.0f, 2.0f } };
	A c = { Keyboard<1>(), { 2.0f, 1.0f } };
	A d = { Keyboard<1>(), { 1.0f, 1.0f } };
	A e = { Keyboard<1>(), { 3.0f, 3.0f } };
	A f = { Keyboard<1>(), { 2.0f, 2.0f } };
	A g = { Keyboard<1>(), { 1.0f, 2.0f } };
	A h = { Keyboard<1>(), { 2.0f, 3.0f } };
	// 4
	// 3   h e
	// 2 g f b
	// 1 d c   a
	//   1 2 3 4
	std::array<A, 8> input{ a, b, c, d, e, f, g, h };
	NonDominatedSet<1> s(input);
	ASSERT_EQ(2, s.size()); 
	std::array<A, 2> res{ a, e };
	testEqual(res, s);
}
*/