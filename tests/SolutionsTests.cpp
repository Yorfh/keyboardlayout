#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Solutions.hpp"
#include "Keyboard.hpp"

TEST(SolutionsTests, IteratorTraversal)
{
	std::array<Keyboard<1>, 2> keyboards{ Keyboard<1>({1}), Keyboard<1>({2}) };
	std::array<std::array<float, 1>, 2> solutions{ {{3.0}, {5.0}} };
	Solutions<decltype(keyboards), decltype(solutions)> s(keyboards, solutions);
	auto itr = s.begin();
	EXPECT_EQ(Keyboard<1>({ 1 }).m_keys, itr->keyboard().m_keys);
	EXPECT_EQ((std::array<float, 1>{ 3.0 }), itr->solution());
	++itr;
	EXPECT_EQ(Keyboard<1>({ 2 }).m_keys, itr->keyboard().m_keys);
	EXPECT_EQ((std::array<float, 1>{ 5.0 }), itr->solution());
	++itr;
	EXPECT_EQ(s.end(), itr);
}

TEST(SolutionsTests, IteratorAssignment)
{
	std::array<Keyboard<1>, 2> keyboards{ Keyboard<1>({1}), Keyboard<1>({2}) };
	std::array<std::array<float, 1>, 2> solutions{ {{3.0}, {5.0}} };
	Solutions<decltype(keyboards), decltype(solutions)> s(keyboards, solutions);
	auto itr = s.begin();
	*itr =*(itr + 1);
	EXPECT_EQ(Keyboard<1>({ 2 }).m_keys, itr->keyboard().m_keys);
	EXPECT_EQ((std::array<float, 1>{ 5.0 }), itr->solution());
	++itr;
	EXPECT_EQ(Keyboard<1>({ 2 }).m_keys, itr->keyboard().m_keys);
	EXPECT_EQ((std::array<float, 1>{ 5.0 }), itr->solution());
}

TEST(SolutionsTests, IteratorSwap)
{
	std::array<Keyboard<1>, 2> keyboards{ Keyboard<1>({1}), Keyboard<1>({2}) };
	std::array<std::array<float, 1>, 2> solutions{ {{3.0}, {5.0}} };
	Solutions<decltype(keyboards), decltype(solutions)> s(keyboards, solutions);
	auto itr = s.begin();
	using std::swap;
	swap(*itr, *(itr + 1));
	EXPECT_EQ(Keyboard<1>({ 2 }).m_keys, itr->keyboard().m_keys);
	EXPECT_EQ((std::array<float, 1>{ 5.0 }), itr->solution());
	++itr;
	EXPECT_EQ(Keyboard<1>({ 1 }).m_keys, itr->keyboard().m_keys);
	EXPECT_EQ((std::array<float, 1>{ 3.0 }), itr->solution());
}

