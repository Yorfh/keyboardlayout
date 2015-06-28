#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Helpers.hpp"
#include "MakeArray.hpp"
#include "NonDominatedSet.hpp"
#include "Keyboard.hpp"

using namespace testing;

TEST(HelpersTest, isDominatedSingleDimension)
{
	EXPECT_FALSE(isDominated(make_array(5), make_array(3)));
	EXPECT_TRUE(isDominated(make_array(2), make_array(3)));
	EXPECT_FALSE(isDominated(make_array(3), make_array(3)));
}

TEST(HelpersTest, isDominatedTwoDimensions)
{
	EXPECT_FALSE(isDominated(make_array(5, 3), make_array(2, 1)));
	EXPECT_FALSE(isDominated(make_array(5, 3), make_array(5, 2)));
	EXPECT_FALSE(isDominated(make_array(5, 3), make_array(4, 4)));
	EXPECT_FALSE(isDominated(make_array(5, 3), make_array(5, 3)));
	EXPECT_TRUE(isDominated(make_array(5, 3), make_array(6, 3)));
	EXPECT_TRUE(isDominated(make_array(5, 3), make_array(5, 4)));
	EXPECT_TRUE(isDominated(make_array(5, 3), make_array(6, 4)));
}

Keyboard<1> make_keyboard(unsigned char key)
{
	return Keyboard<1>(make_array(key));
}

std::array<float, 1> make_solution(float v)
{
	return make_array<float>(v);
}

template<typename... T>
std::array<Keyboard<1>, sizeof...(T)> make_keyboards(T&&... keyboards)
{
	return make_array(Keyboard<1>(make_keyboard(std::forward<T>(keyboards)))...);
}

template<typename... T>
std::array<std::array<float, 1>, sizeof...(T)> make_solutions(T&&... solutions)
{
	return make_array(make_array(std::forward<T>(solutions))...);
}

template<typename... T>
std::array<float, sizeof...(T)> make_solution_dimension(T&&... solutionDimensions)
{
	return make_array(std::forward<T>(solutionDimensions)...);
}

static const float eps = 0.0001f;

TEST(HelpersTest, CalculateFitnessOneElement)
{
	auto keyboards = make_keyboards(1);
	auto solutions = make_solutions(1.0f);
	NonDominatedSet<1> nds(keyboards, solutions);
	std::array<float, 1> output;
	FitnessCalculator().calculateFitness(nds, solutions, output.begin());
	EXPECT_THAT(output, ElementsAre(1.0f));
}

TEST(HelpersTest, CalculateFitnessTwoElements)
{
	auto keyboards = make_keyboards(1, 2);
	auto solutions = make_solutions(1.0f, 2.0f);
	NonDominatedSet<1> nds(keyboards, solutions);
	std::array<float, 2> output;
	FitnessCalculator().calculateFitness(nds, solutions, output.begin());
	EXPECT_THAT(output, ElementsAre(FloatNear(0.66667f, eps), 1.0f));
}

TEST(HelpersTest, CalculateFitnessTwoElementsDifferentOrder)
{
	auto keyboards = make_keyboards(2, 1);
	auto solutions = make_solutions(2.0f, 1.0f);
	NonDominatedSet<1> nds(keyboards, solutions);
	EXPECT_EQ(1, nds.size());
	std::array<float, 2> output;
	FitnessCalculator().calculateFitness(nds, solutions, output.begin());
	EXPECT_THAT(output, ElementsAre(1.0f, FloatNear(0.66667f, eps)));
}

TEST(HelpersTest, CalculateFitnessThreeElements)
{
	auto keyboards = make_keyboards(1, 2, 3);
	auto solutions = make_solutions(1.0f, 2.0f, 3.0f);
	NonDominatedSet<1> nds(keyboards, solutions);
	std::array<float, 3> output;
	FitnessCalculator().calculateFitness(nds, solutions, output.begin());
	EXPECT_THAT(output, ElementsAre(FloatNear(0.6f, eps), FloatNear(0.6f, eps), 1.0f));
}

TEST(HelpersTest, CalculateFitnessTwoDimensional)
{
	auto keyboards = make_keyboards(1, 2, 3);
	auto a = make_solution_dimension(10.0f, 5.0f);
	auto b = make_solution_dimension(5.0f, 10.0f);
	auto c = make_solution_dimension(7.0f, 4.0f);
	auto solutions = make_array(a, b, c);
	NonDominatedSet<1> nds(keyboards, solutions);
	std::array<float, 3> output;
	FitnessCalculator().calculateFitness(nds, solutions, output.begin());
	EXPECT_EQ(1.0f, output[0]);
	EXPECT_EQ(1.0f, output[1]);
	EXPECT_GT(1.0f, output[2]);
}

TEST(HelpersTest, CalculateFitnessTwoDimensionalDominatedByTwoIsWorseThanDominatedByOne)
{
	auto keyboards = make_keyboards(1, 2, 3, 4);
	auto a = make_solution_dimension(10.0f, 5.0f);
	auto b = make_solution_dimension(5.0f, 10.0f);
	auto c = make_solution_dimension(7.0f, 4.0f);
	auto d = make_solution_dimension(3.0f, 4.0f);
	auto solutions = make_array(a, b, c, d);
	NonDominatedSet<1> nds(keyboards, solutions);
	std::array<float, 4> output;
	FitnessCalculator().calculateFitness(nds, solutions, output.begin());
	EXPECT_EQ(1.0f, output[0]);
	EXPECT_EQ(1.0f, output[1]);
	EXPECT_GT(1.0f, output[2]);
	EXPECT_GT(1.0f, output[3]);
	EXPECT_GT(output[2], output[3]);
}

TEST(HelpersTest, CalculateFitnessTwoDimensionalDominatedByBetterSolutionIsWorse)
{
	auto keyboards = make_keyboards(1, 2, 3, 4);
	auto a = make_solution_dimension(10.0f, 5.0f);
	auto b = make_solution_dimension(5.0f, 10.0f);
	auto c = make_solution_dimension(7.0f, 4.0f);
	auto d = make_solution_dimension(6.0f, 4.0f);
	auto e = make_solution_dimension(4.0f, 6.0f);
	auto solutions = make_array(a, b, c, d, e);
	NonDominatedSet<1> nds(keyboards, solutions);
	std::array<float, 5> output;
	FitnessCalculator().calculateFitness(nds, solutions, output.begin());
	EXPECT_EQ(1.0f, output[0]);
	EXPECT_EQ(1.0f, output[1]);
	EXPECT_GT(1.0f, output[2]);
	EXPECT_GT(1.0f, output[3]);
	EXPECT_GT(1.0f, output[4]);
	EXPECT_EQ(output[2], output[3]);
	EXPECT_GT(output[4], output[2]);
	EXPECT_GT(output[4], output[3]);
}