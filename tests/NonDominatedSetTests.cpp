#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "NonDominatedSet.hpp"
#include <array>
#include "TestUtilities.hpp"
#include "MakeArray.hpp"
using namespace testing;

template<typename KeyboardArray, typename SolutionArray, typename SolutionsVector>
void testEqual(const KeyboardArray& expectedKeyboards, const SolutionArray& expectedSolutions, 
	const SolutionsVector& actual)
{
	ASSERT_EQ(expectedKeyboards.size(), actual.size());
	ASSERT_EQ(expectedSolutions.size(), actual.size());
	auto sortedSolutions = actual;
	std::sort(sortedSolutions.begin(), sortedSolutions.end(), [](const auto& lhs, const auto& rhs)
	{
		return lhs.m_solution < rhs.m_solution;
	});
	auto ek = std::begin(expectedKeyboards);
	auto es = std::begin(expectedSolutions);
	for (auto&& i : sortedSolutions)
	{
		EXPECT_EQ(*ek, i.m_keyboard);
		
		auto si = std::begin(*es);
		for (auto&& j : i.m_solution)
		{
			EXPECT_EQ(*si, j);
			++si;
		}
		++ek;
		++es;
	}
}
template<size_t Size>
using KeyboardArray = std::array<Keyboard<1>, Size>;
template<size_t Size, size_t NumDimensions = 1>
using SolutionArray = std::array<std::array<float, NumDimensions>, Size>;

template<typename SizeType>
class NonDominatedSetTests : public testing::Test
{
};

TYPED_TEST_CASE_P(NonDominatedSetTests);

TYPED_TEST_P(NonDominatedSetTests, SimpleOneDimensionalOneValue)
{
	KeyboardArray<1> keyboards{ Keyboard<1>({1}) };
	SolutionArray<1> solutions{ {1.0} };
	NonDominatedSet<1, 1, TypeParam::value> s(keyboards, solutions);
	ASSERT_EQ(1, s.size()); 
	testEqual(keyboards, solutions, s.getResult());
	EXPECT_THAT(s.getIdealPoint(), ElementsAreClose(1.0f));
}

TYPED_TEST_P(NonDominatedSetTests, OneDimensionalTwoValues)
{
	KeyboardArray<2> keyboards{ Keyboard<1>({1}), Keyboard<1>({2}) };
	SolutionArray<2> solutions{ 1.0, 2.0 };
	NonDominatedSet<1, 1, TypeParam::value> s(keyboards, solutions);
	ASSERT_EQ(1, s.size()); 
	testEqual(KeyboardArray<1>{ Keyboard<1>({2}) }, SolutionArray<1>{ {2.0} }, s.getResult());
	EXPECT_THAT(s.getIdealPoint(), ElementsAreClose(2.0f));
}

TYPED_TEST_P(NonDominatedSetTests, OneDimensionalTwoDifferentOrder)
{
	KeyboardArray<2> keyboards{ Keyboard<1>({2}), Keyboard<1>({1}) };
	SolutionArray<2> solutions{ 2.0, 1.0 };
	NonDominatedSet<1, 1, TypeParam::value> s(keyboards, solutions);
	ASSERT_EQ(1, s.size()); 
	testEqual(KeyboardArray<1>{ Keyboard<1>({2}) }, SolutionArray<1>{ {2.0} }, s.getResult());
	EXPECT_THAT(s.getIdealPoint(), ElementsAreClose(2.0f));
}

TYPED_TEST_P(NonDominatedSetTests, OneDimensionalThreeValues1)
{
	using A = std::pair<Keyboard<1>, std::array<float, 1>>;
	A a = { Keyboard<1>({1}), { 1.0f } };
	A b = { Keyboard<1>({2}), { 2.0f } };
	A c = { Keyboard<1>({3}), { 3.0f } };
	KeyboardArray<3> keyboards{ a.first, b.first, c.first };
	SolutionArray<3> solutions{ a.second, b.second, c.second };
	NonDominatedSet<1, 1, TypeParam::value> s(keyboards, solutions);
	ASSERT_EQ(1, s.size()); 
	testEqual(KeyboardArray<1>{ c.first }, SolutionArray<1>{ c.second }, s.getResult());
	EXPECT_THAT(s.getIdealPoint(), ElementsAreClose(3.0f));
}

TYPED_TEST_P(NonDominatedSetTests, OneDimensionalThreeValues2)
{
	using A = std::pair<Keyboard<1>, std::array<float, 1>>;
	A a = { Keyboard<1>({1}), { 1.0f } };
	A b = { Keyboard<1>({2}), { 2.0f } };
	A c = { Keyboard<1>({3}), { 3.0f } };
	KeyboardArray<3> keyboards{ a.first, b.first, c.first };
	SolutionArray<3> solutions{ a.second, b.second, c.second };
	NonDominatedSet<1, 1, TypeParam::value> s(keyboards, solutions);
	ASSERT_EQ(1, s.size()); 
	testEqual(KeyboardArray<1>{ c.first }, SolutionArray<1>{ c.second }, s.getResult());
	EXPECT_THAT(s.getIdealPoint(), ElementsAreClose(3.0f));
}

TYPED_TEST_P(NonDominatedSetTests, OneDimensionalThreeValues3)
{
	using A = std::pair<Keyboard<1>, std::array<float, 1>>;
	A a = { Keyboard<1>({1}), { 1.0f } };
	A b = { Keyboard<1>({2}), { 2.0f } };
	A c = { Keyboard<1>({3}), { 3.0f } };
	KeyboardArray<3> keyboards{ a.first, b.first, c.first };
	SolutionArray<3> solutions{ a.second, b.second, c.second };
	NonDominatedSet<1, 1, TypeParam::value> s(keyboards, solutions);
	ASSERT_EQ(1, s.size()); 
	testEqual(KeyboardArray<1>{ c.first }, SolutionArray<1>{ c.second }, s.getResult());
	EXPECT_THAT(s.getIdealPoint(), ElementsAreClose(3.0f));
}

TYPED_TEST_P(NonDominatedSetTests, OneDimensionalThreeValues4)
{
	using A = std::pair<Keyboard<1>, std::array<float, 1>>;
	A a = { Keyboard<1>({1}), { 1.0f } };
	A b = { Keyboard<1>({2}), { 2.0f } };
	A c = { Keyboard<1>({3}), { 3.0f } };
	KeyboardArray<3> keyboards{ a.first, b.first, c.first };
	SolutionArray<3> solutions{ a.second, b.second, c.second };
	NonDominatedSet<1, 1, TypeParam::value> s(keyboards, solutions);
	ASSERT_EQ(1, s.size()); 
	testEqual(KeyboardArray<1>{ c.first }, SolutionArray<1>{ c.second }, s.getResult());
	EXPECT_THAT(s.getIdealPoint(), ElementsAreClose(3.0f));
}

TYPED_TEST_P(NonDominatedSetTests, OneDimensionalThreeValues5)
{
	using A = std::pair<Keyboard<1>, std::array<float, 1>>;
	A a = { Keyboard<1>({1}), { 1.0f } };
	A b = { Keyboard<1>({2}), { 2.0f } };
	A c = { Keyboard<1>({3}), { 3.0f } };
	KeyboardArray<3> keyboards{ a.first, b.first, c.first };
	SolutionArray<3> solutions{ a.second, b.second, c.second };
	NonDominatedSet<1, 1, TypeParam::value> s(keyboards, solutions);
	ASSERT_EQ(1, s.size()); 
	std::array<A, 1> res{ c };
	testEqual(KeyboardArray<1>{ c.first }, SolutionArray<1>{ c.second }, s.getResult());
	EXPECT_THAT(s.getIdealPoint(), ElementsAreClose(3.0f));
}

TYPED_TEST_P(NonDominatedSetTests, OneDimensionalThreeValues6)
{
	using A = std::pair<Keyboard<1>, std::array<float, 1>>;
	A a = { Keyboard<1>({1}), { 1.0f } };
	A b = { Keyboard<1>({2}), { 2.0f } };
	A c = { Keyboard<1>({3}), { 3.0f } };
	NonDominatedSet<1, 1, TypeParam::value> s;
	s.insert(a.first, a.second);
	s.insert(b.first, b.second);
	s.insert(c.first, c.second);
	ASSERT_EQ(1, s.size()); 
	std::array<A, 1> res{ c };
	testEqual(KeyboardArray<1>{ c.first }, SolutionArray<1>{ c.second }, s.getResult());
	EXPECT_THAT(s.getIdealPoint(), ElementsAreClose(3.0f));
}

TYPED_TEST_P(NonDominatedSetTests, TwoDimensional)
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
	KeyboardArray<8> keyboards{ a.first, b.first, c.first, d.first, e.first, f.first, g.first, h.first };
	SolutionArray<8, 2> solutions{ a.second, b.second, c.second, d.second, e.second, f.second, g.second, h.second };
	// 4
	// 3   h 
	// 2 g f e
	// 1 d c b a
	//   1 2 3 4
	NonDominatedSet<1, 2, TypeParam::value> s(keyboards, solutions);
	ASSERT_EQ(3, s.size()); 
	KeyboardArray<3> keyboardRes{ h.first, e.first, a.first };
	SolutionArray<3, 2> solutionRes{ h.second, e.second, a.second };
	testEqual(keyboardRes, solutionRes, s.getResult());
	EXPECT_THAT(s.getIdealPoint(), ElementsAreClose(4.0f, 3.0f));
}

TYPED_TEST_P(NonDominatedSetTests, TwoDimensional2)
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
	KeyboardArray<8> keyboards{ a.first, b.first, c.first, d.first, e.first, f.first, g.first, h.first };
	SolutionArray<8, 2> solutions{ a.second, b.second, c.second, d.second, e.second, f.second, g.second, h.second };
	// 4
	// 3   h e
	// 2 g f b
	// 1 d c   a
	//   1 2 3 4
	NonDominatedSet<1, 2, TypeParam::value> s(keyboards, solutions);
	ASSERT_EQ(2, s.size()); 
	std::array<A, 2> res{ a, e };
	KeyboardArray<2> keyboardRes{ e.first, a.first };
	SolutionArray<2, 2> solutionRes{ e.second, a.second };
	testEqual(keyboardRes, solutionRes, s.getResult());
	EXPECT_THAT(s.getIdealPoint(), ElementsAreClose(4.0f, 3.0f));
}

TYPED_TEST_P(NonDominatedSetTests, TwoDimensional3)
{
	using A = std::pair<Keyboard<1>, std::array<float, 2>>;
	A a = { Keyboard<1>({1}), {-2.0f, -8.0f} };
	A b = { Keyboard<1>({2}), {-2.0f, -5.0f} };
	A c = { Keyboard<1>({3}), {-3.0f, -9.0f} };
	A d = { Keyboard<1>({4}), {-4.0f, -6.0f} };
	A e = { Keyboard<1>({5}), {-4.0f, -4.0f} };
	A f = { Keyboard<1>({6}), {-5.0f, -7.0f} };
	A g = { Keyboard<1>({7}), {-6.0f, -4.0f} };
	A h = { Keyboard<1>({8}), {-7.0f, -6.0f} };
	A i = { Keyboard<1>({9}), {-7.0f, -2.0f} };
	A j = { Keyboard<1>({10}), {-8.0f, -8.0f} };
	A k = { Keyboard<1>({11}), {-8.0f, -3.0f} };
	A l = { Keyboard<1>({12}), {-9.0f, -1.0f} };
	KeyboardArray<12> keyboards{ a.first, b.first, c.first, d.first, e.first, f.first, g.first, h.first, i.first, j.first, k.first, l.first };
	SolutionArray<12, 2> solutions{ a.second, b.second, c.second, d.second, e.second, f.second, g.second, h.second, i.second, j.second, k.second, l.second };
	NonDominatedSet<1, 2, TypeParam::value> s(keyboards, solutions);
	ASSERT_EQ(4, s.size()); 
	KeyboardArray<4> keyboardRes{ l.first, i.first, e.first, b.first };
	SolutionArray<4, 2> solutionRes{ l.second, i.second, e.second, b.second};
	testEqual(keyboardRes, solutionRes, s.getResult());
	EXPECT_THAT(s.getIdealPoint(), ElementsAreClose(-2.0f, -1.0f));
}

TYPED_TEST_P(NonDominatedSetTests, TwoDimensional4)
{
	using A = std::pair<Keyboard<1>, std::array<float, 2>>;
	A a = { Keyboard<1>({1}), {2.0f, 8.0f} };
	A b = { Keyboard<1>({2}), {2.0f, 5.0f} };
	A c = { Keyboard<1>({3}), {3.0f, 9.0f} };
	A d = { Keyboard<1>({4}), {4.0f, 6.0f} };
	A e = { Keyboard<1>({5}), {4.0f, 4.0f} };
	A f = { Keyboard<1>({6}), {5.0f, 7.0f} };
	A g = { Keyboard<1>({7}), {6.0f, 4.0f} };
	A h = { Keyboard<1>({8}), {7.0f, 6.0f} };
	A i = { Keyboard<1>({9}), {7.0f, 2.0f} };
	A j = { Keyboard<1>({10}), {8.0f, 8.0f} };
	A k = { Keyboard<1>({11}), {8.0f, 3.0f} };
	A l = { Keyboard<1>({12}), {9.0f, 1.0f} };
	KeyboardArray<12> keyboards{ a.first, b.first, c.first, d.first, e.first, f.first, g.first, h.first, i.first, j.first, k.first, l.first };
	SolutionArray<12, 2> solutions{ a.second, b.second, c.second, d.second, e.second, f.second, g.second, h.second, i.second, j.second, k.second, l.second };
	NonDominatedSet<1, 2, TypeParam::value> s(keyboards, solutions);
	ASSERT_EQ(3, s.size()); 
	KeyboardArray<3> keyboardRes{ c.first, j.first, l.first };
	SolutionArray<3, 2> solutionRes{ c.second, j.second, l.second };
	testEqual(keyboardRes, solutionRes, s.getResult());
	EXPECT_THAT(s.getIdealPoint(), ElementsAreClose(9.0f, 9.0f));
}

TYPED_TEST_P(NonDominatedSetTests, TwoDimensional5)
{
	using A = std::pair<Keyboard<1>, std::array<float, 2>>;
	A a = { Keyboard<1>({ 1 }),{ 2.0f, 8.0f } };
	A b = { Keyboard<1>({ 2 }),{ 2.0f, 5.0f } };
	A c = { Keyboard<1>({ 3 }),{ 3.0f, 9.0f } };
	A d = { Keyboard<1>({ 4 }),{ 4.0f, 6.0f } };
	A e = { Keyboard<1>({ 5 }),{ 4.0f, 4.0f } };
	A f = { Keyboard<1>({ 6 }),{ 5.0f, 7.0f } };
	A g = { Keyboard<1>({ 7 }),{ 6.0f, 4.0f } };
	A h = { Keyboard<1>({ 8 }),{ 7.0f, 6.0f } };
	A i = { Keyboard<1>({ 9 }),{ 7.0f, 2.0f } };
	A j = { Keyboard<1>({ 10 }),{ 8.0f, 8.0f } };
	A k = { Keyboard<1>({ 11 }),{ 8.0f, 3.0f } };
	A l = { Keyboard<1>({ 12 }),{ 9.0f, 1.0f } };
	KeyboardArray<12> keyboards{ a.first, f.first, k.first, c.first, g.first, i.first, d.first, h.first, j.first, e.first, l.first, b.first };
	SolutionArray<12, 2> solutions{ a.second, f.second, k.second, c.second, g.second, i.second, d.second, h.second, j.second, e.second, l.second, b.second };
	NonDominatedSet<1, 2, TypeParam::value> s(keyboards, solutions);
	ASSERT_EQ(3, s.size());
	KeyboardArray<3> keyboardRes{ c.first, j.first, l.first };
	SolutionArray<3, 2> solutionRes{ c.second, j.second, l.second };
	testEqual(keyboardRes, solutionRes, s.getResult());
	EXPECT_THAT(s.getIdealPoint(), ElementsAreClose(9.0f, 9.0f));
}

TYPED_TEST_P(NonDominatedSetTests, TwoDimensional6)
{
	using A = std::pair<Keyboard<1>, std::array<float, 2>>;
	A a = { Keyboard<1>({1}), {2.0f, 8.0f} };
	A b = { Keyboard<1>({2}), {2.0f, 5.0f} };
	A c = { Keyboard<1>({3}), {3.0f, 9.0f} };
	A d = { Keyboard<1>({4}), {4.0f, 6.0f} };
	A e = { Keyboard<1>({5}), {4.0f, 4.0f} };
	A f = { Keyboard<1>({6}), {5.0f, 7.0f} };
	A g = { Keyboard<1>({7}), {6.0f, 4.0f} };
	A h = { Keyboard<1>({8}), {7.0f, 6.0f} };
	A i = { Keyboard<1>({9}), {7.0f, 2.0f} };
	A j = { Keyboard<1>({10}), {8.0f, 8.0f} };
	A k = { Keyboard<1>({11}), {8.0f, 3.0f} };
	A l = { Keyboard<1>({12}), {9.0f, 1.0f} };
	KeyboardArray<12> keyboards{ a.first, b.first, c.first, d.first, e.first, f.first, g.first, h.first, i.first, j.first, k.first, l.first };
	SolutionArray<12, 2> solutions{ a.second, b.second, c.second, d.second, e.second, f.second, g.second, h.second, i.second, j.second, k.second, l.second };
	std::mt19937 twister1(215);
	std::mt19937 twister2(215);

	std::shuffle(keyboards.begin(), keyboards.end(), twister1);
	std::shuffle(solutions.begin(), solutions.end(), twister2);
	NonDominatedSet<1, 2, TypeParam::value> s(keyboards, solutions);
	ASSERT_EQ(3, s.size()); 
	KeyboardArray<3> keyboardRes{ c.first, j.first, l.first };
	SolutionArray<3, 2> solutionRes{ c.second, j.second, l.second };
	testEqual(keyboardRes, solutionRes, s.getResult());
	EXPECT_THAT(s.getIdealPoint(), ElementsAreClose(9.0f, 9.0f));
}

#if 0
TYPED_TEST_P(NonDominatedSetTests, TwoDimensional5)
{
	using A = std::pair<Keyboard<1>, std::array<float, 2>>;
	A a = { Keyboard<1>({1}), {2.0f, 8.0f} };
	A b = { Keyboard<1>({2}), {2.0f, 5.0f} };
	A c = { Keyboard<1>({3}), {3.0f, 9.0f} };
	A d = { Keyboard<1>({4}), {4.0f, 6.0f} };
	A e = { Keyboard<1>({5}), {4.0f, 4.0f} };
	A f = { Keyboard<1>({6}), {5.0f, 7.0f} };
	A g = { Keyboard<1>({7}), {6.0f, 4.0f} };
	A h = { Keyboard<1>({8}), {7.0f, 6.0f} };
	A i = { Keyboard<1>({9}), {7.0f, 2.0f} };
	A j = { Keyboard<1>({10}), {8.0f, 8.0f} };
	A k = { Keyboard<1>({11}), {8.0f, 3.0f} };
	A l = { Keyboard<1>({12}), {9.0f, 1.0f} };
	for (unsigned int seed = 0; seed < 1000; seed++)
	{
		KeyboardArray<12> keyboards{ a.first, b.first, c.first, d.first, e.first, f.first, g.first, h.first, i.first, j.first, k.first, l.first };
		SolutionArray<12, 2> solutions{ a.second, b.second, c.second, d.second, e.second, f.second, g.second, h.second, i.second, j.second, k.second, l.second };
		std::mt19937 twister1(seed);
		std::mt19937 twister2(seed);

		std::shuffle(keyboards.begin(), keyboards.end(), twister1);
		std::shuffle(solutions.begin(), solutions.end(), twister2);
		NonDominatedSet<1, 2, TypeParam::value> s(keyboards, solutions);
		ASSERT_EQ(3, s.size()); 
		KeyboardArray<3> keyboardRes{ c.first, j.first, l.first };
		SolutionArray<3, 2> solutionRes{ c.second, j.second, l.second };
		testEqual(keyboardRes, solutionRes, s.getResult());
		EXPECT_THAT(s.getIdealPoint(), ElementsAreClose(9.0f, 9.0f));
	}
}
#endif

REGISTER_TYPED_TEST_CASE_P(NonDominatedSetTests, SimpleOneDimensionalOneValue, OneDimensionalTwoValues, OneDimensionalTwoDifferentOrder, OneDimensionalThreeValues1,
	OneDimensionalThreeValues2, OneDimensionalThreeValues3, OneDimensionalThreeValues4, OneDimensionalThreeValues5,
	OneDimensionalThreeValues6, TwoDimensional, TwoDimensional2, TwoDimensional3, TwoDimensional4, TwoDimensional5, TwoDimensional6);
typedef ::testing::Types <
	std::integral_constant<size_t, 1>,
	std::integral_constant<size_t, 2>,
	std::integral_constant<size_t, 3>,
	std::integral_constant<size_t, 4>,
	std::integral_constant<size_t, 5>,
	std::integral_constant<size_t, 6>,
	std::integral_constant<size_t, 7>,
	std::integral_constant<size_t, 8>,
	std::integral_constant<size_t, std::numeric_limits<size_t>::max()>> Sizes;
INSTANTIATE_TYPED_TEST_CASE_P(NonDominatedSetTests, NonDominatedSetTests, Sizes);

TEST(SelectPivotPointTests, SelectPivotPoint)
{
	auto a = make_array(-2.0f, -8.0f);
	auto b = make_array(-2.0f, -5.0f);
	auto c = make_array(-3.0f, -9.0f);
	auto d = make_array(-4.0f, -6.0f);
	auto e = make_array(-4.0f, -4.0f);
	auto f = make_array(-5.0f, -7.0f);
	auto g = make_array(-6.0f, -4.0f);
	auto h = make_array(-7.0f, -6.0f);
	auto i = make_array(-7.0f, -2.0f);
	auto j = make_array(-8.0f, -8.0f);
	auto k = make_array(-8.0f, -3.0f);
	auto l = make_array(-9.0f, -1.0f);
	using Solution = NonDominatedSet<1, 2>::Solution;
	auto makeSolution = [](std::array<float, 2>& s)
	{
		return Solution(Keyboard<1>(), std::begin(s), std::end(s));
	};
	std::vector<Solution> solutionVector{ makeSolution(a), makeSolution(b), makeSolution(c), makeSolution(d), makeSolution(e),
		makeSolution(f), makeSolution(g), makeSolution(h), makeSolution(i), makeSolution(j), makeSolution(k), makeSolution(l) };
	nondominatedset_detail::selectPivotPoint(solutionVector);
	EXPECT_EQ(solutionVector[0].m_solution, makeSolution(e).m_solution);
}

TEST(MapPointToRegionTests, MapPointToRegion2D)
{
	auto b = make_array(-2.0f, -5.0f);
	auto e = make_array(-4.0f, -4.0f);
	auto g = make_array(-6.0f, -4.0f);
	auto h = make_array(-7.0f, -6.0f);
	auto i = make_array(-7.0f, -2.0f);
	unsigned int res = nondominatedset_detail::mapPointToRegion(e, e);
	EXPECT_EQ(0b11, res);
	res = nondominatedset_detail::mapPointToRegion(e, b);
	EXPECT_EQ(0b10, res);
	res = nondominatedset_detail::mapPointToRegion(e, g);
	EXPECT_EQ(0b11, res);
	res = nondominatedset_detail::mapPointToRegion(e, h);
	EXPECT_EQ(0b11, res);
	res = nondominatedset_detail::mapPointToRegion(e, i);
	EXPECT_EQ(0b01, res);
	res = nondominatedset_detail::mapPointToRegion(h, e);
	EXPECT_EQ(0b00, res);
}