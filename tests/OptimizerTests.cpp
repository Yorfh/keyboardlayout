#include "optimizer.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <array>
#include "Objective.hpp"
#include "MakeArray.hpp"
#include "TestUtilities.hpp"

using namespace testing;

template<size_t KeyboardSize>
class TestObjective : public Objective<KeyboardSize>
{
public:
	TestObjective(std::function<float(const Keyboard<KeyboardSize>& keyboard)> func)
		: m_func(std::move(func))
	{

	}

	float evaluate(const Keyboard<KeyboardSize>& keyboard) const override
	{
		return m_func(keyboard);
	}

	std::function<float(const Keyboard<KeyboardSize>& keyboard)> m_func;
};

TEST(OptimizerTests, IncreasingOrderNoLocalSearch)
{
	auto evaluate = [](const Keyboard<3>& keyboard)
	{
		return keyboard.m_keys[0] * 10.0f +
			keyboard.m_keys[1] * 100.0f +
			keyboard.m_keys[2] * 1000.0f;
	};
	Optimizer<3> o; 
	o.populationSize(50);
	o.numIterations(0);
	auto objectives = { TestObjective<3>(evaluate) };
	auto& solutions = o.optimize(std::begin(objectives), std::end(objectives));
	ASSERT_EQ(1, solutions.size());
	EXPECT_THAT(solutions.getResult()[0].first.m_keys, ElementsAre(0, 1, 2));
}

TEST(OptimizerTests, DecreasingOrderNoLocalSearch)
{
	auto evaluate = [](const Keyboard<3>& keyboard)
	{
		return keyboard.m_keys[0] * 1000.0f +
			keyboard.m_keys[1] * 100.0f +
			keyboard.m_keys[2] * 10.0f;
	};
	Optimizer<3> o; 
	o.populationSize(50);
	o.numIterations(0);
	auto objectives = { TestObjective<3>(evaluate) };
	auto& solutions = o.optimize(std::begin(objectives), std::end(objectives));
	ASSERT_EQ(1, solutions.size());
	EXPECT_THAT(solutions.getResult()[0].first.m_keys, ElementsAre(2, 1, 0));
}

TEST(OptimizerTests, IncreasingOrderSmallPopulation)
{
	auto evaluate = [](const Keyboard<3>& keyboard)
	{
		return keyboard.m_keys[0] * 10.0f +
			keyboard.m_keys[1] * 100.0f +
			keyboard.m_keys[2] * 1000.0f;
	};
	Optimizer<3> o; 
	o.populationSize(3);
	o.numIterations(1);
	auto objectives = { TestObjective<3>(evaluate) };
	auto& solutions = o.optimize(std::begin(objectives), std::end(objectives));
	ASSERT_EQ(1, solutions.size());
	EXPECT_THAT(solutions.getResult()[0].first.m_keys, ElementsAre(0, 1, 2));
}

TEST(OptimizerTests, DecreasingOrderSmallPopulation)
{
	auto evaluate = [](const Keyboard<3>& keyboard)
	{
		return keyboard.m_keys[0] * 1000.0f +
			keyboard.m_keys[1] * 100.0f +
			keyboard.m_keys[2] * 10.0f;
	};
	Optimizer<3> o; 
	o.populationSize(3);
	o.numIterations(1);
	auto objectives = { TestObjective<3>(evaluate) };
	auto& solutions = o.optimize(std::begin(objectives), std::end(objectives));
	ASSERT_EQ(1, solutions.size());
	EXPECT_THAT(solutions.getResult()[0].first.m_keys, ElementsAre(2, 1, 0));
}

TEST(OptimizerTests, TwoObjectives)
{
	auto evaluate1 = [](const Keyboard<3>& keyboard)
	{
		return keyboard.m_keys[0] * 10.0f +
			keyboard.m_keys[1] * 100.0f +
			keyboard.m_keys[2] * 1000.0f;
	};
	
	auto evaluate2 = [](const Keyboard<3>& keyboard)
	{
		return keyboard.m_keys[0] * 1000.0f +
			keyboard.m_keys[1] * 100.0f +
			keyboard.m_keys[2] * 10.0f;
	};
	Optimizer<3> o;
	o.populationSize(3);
	o.numIterations(1);
	auto objectives = { TestObjective<3>(evaluate1), TestObjective<3>(evaluate2) };
	auto& solutions = o.optimize(std::begin(objectives), std::end(objectives));
	auto results = solutions.getResult();
	ASSERT_EQ(4, solutions.size());
	std::sort(results.begin(), results.end(), 
	[](auto& lhs, auto& rhs)
	{
		auto& lhsKeys = lhs.first.m_keys;
		auto& rhsKeys = rhs.first.m_keys;
		if (lhsKeys[0] == rhsKeys[0])
		{
			if (lhsKeys[1] == rhsKeys[1])
			{
				return lhsKeys[2] < rhsKeys[2];
			}
			else
			{
				return lhsKeys[1] < rhsKeys[1];
			}
		}
		else
		{
			return lhsKeys[0] < rhsKeys[0];
		}
	});
	EXPECT_THAT(results[0].first.m_keys, ElementsAre(0, 1, 2));
	EXPECT_THAT(results[1].first.m_keys, ElementsAre(1, 0, 2));
	EXPECT_THAT(results[2].first.m_keys, ElementsAre(2, 0, 1));
	EXPECT_THAT(results[3].first.m_keys, ElementsAre(2, 1, 0));
}

TEST(CrossOverTests, PartiallyMatchedCrossover1)
{
	Keyboard<7> parent1({ 5, 1, 2, 3, 0, 6, 4 });
	Keyboard<7> parent2({ 4, 1, 3, 0, 2, 6, 5 });
	auto child = detail::partiallyMatchedCrossover(parent1, parent2, 3, 5);
	EXPECT_THAT(child.m_keys, ElementsAre(5, 1, 3, 0, 2, 6, 4));
}

TEST(CrossOverTests, PartiallyMatchedCrossover2)
{
	Keyboard<7> parent1({ 4, 1, 3, 0, 2, 6, 5 });
	Keyboard<7> parent2({ 5, 1, 2, 3, 0, 6, 4 });
	auto child = detail::partiallyMatchedCrossover(parent1, parent2, 3, 5);
	EXPECT_THAT(child.m_keys, ElementsAre(4, 1, 2, 3, 0, 6, 5));
}

TEST(CrossOverTests, PartiallyMatchedCrossover3)
{
	Keyboard<8> parent1({ 3, 1, 4, 7, 0, 2, 5, 6 });
	Keyboard<8> parent2({ 0, 4, 1, 7, 6, 3, 2, 5 });
	auto child = detail::partiallyMatchedCrossover(parent1, parent2, 2, 4);
	EXPECT_THAT(child.m_keys, ElementsAre(3, 4, 1, 7, 6, 2, 5, 0));
}

TEST(WeightVectorTests, GenerateOneDimensionalOneValueVector)
{
	std::vector<std::vector<float>> output;
	detail::generateWeightVectors(output, 1, 1);
	ASSERT_EQ(1, output.size());
	EXPECT_THAT(output[0], ElementsAreClose(1.0f));
}

TEST(WeightVectorTests, GenerateOneDimensionalTwoValueVector)
{
	std::vector<std::vector<float>> output;
	detail::generateWeightVectors(output, 2, 1);
	ASSERT_EQ(2, output.size());
	EXPECT_THAT(output[0], ElementsAreClose(1.0f));
	EXPECT_THAT(output[1], ElementsAreClose(1.0f));
}

TEST(WeightVectorTests, GenerateOneDimensionalFourValueVector)
{
	std::vector<std::vector<float>> output;
	detail::generateWeightVectors(output, 4, 1);
	ASSERT_EQ(4, output.size());
	EXPECT_THAT(output[0], ElementsAreClose(1.0f));
	EXPECT_THAT(output[1], ElementsAreClose(1.0f));
	EXPECT_THAT(output[2], ElementsAreClose(1.0f));
	EXPECT_THAT(output[3], ElementsAreClose(1.0f));
}

TEST(WeightVectorTests, GenerateTwoDimensionalOneValueVector)
{
	std::vector<std::vector<float>> output;
	detail::generateWeightVectors(output, 1, 2);
	ASSERT_EQ(1, output.size());
	EXPECT_THAT(output[0], ElementsAreClose(0.5f, 0.5f));
}

TEST(WeightVectorTests, GenerateTwoDimensionalTwoValueVector)
{
	std::vector<std::vector<float>> output;
	detail::generateWeightVectors(output, 2, 2);
	ASSERT_EQ(2, output.size());
	EXPECT_THAT(output[0], ElementsAreClose(0.0f, 1.0f));
	EXPECT_THAT(output[1], ElementsAreClose(1.0f, 0.0f));
}

TEST(WeightVectorTests, GenerateTwoDimensionalThreeValueVector)
{
	std::vector<std::vector<float>> output;
	detail::generateWeightVectors(output, 3, 2);
	ASSERT_EQ(3, output.size());
	EXPECT_THAT(output[0], ElementsAreClose(0.0f, 1.0f));
	EXPECT_THAT(output[1], ElementsAreClose(1.0f, 0.0f));
	EXPECT_THAT(output[2], ElementsAreClose(0.5f, 0.5f));
}

TEST(WeightVectorTests, GenerateTwoDimensionalFourValueVector)
{
	std::vector<std::vector<float>> output;
	detail::generateWeightVectors(output, 4, 2);
	ASSERT_EQ(4, output.size());
	EXPECT_THAT(output[0], ElementsAreClose(0.0f, 1.0f));
	EXPECT_THAT(output[1], ElementsAreClose(1.0f, 0.0f));
	EXPECT_THAT(output[2], ElementsAreClose(0.3333333f, 0.6666667f));
	EXPECT_THAT(output[3], ElementsAreClose(0.6666667f, 0.3333333f));
}

TEST(WeightVectorTests, GenerateThreeDimensionalOneValueVector)
{
	std::vector<std::vector<float>> output;
	detail::generateWeightVectors(output, 1, 3);
	ASSERT_EQ(1, output.size());
	EXPECT_THAT(output[0], ElementsAreClose(0.3333333f, 0.3333333f, 0.3333333f));
}

TEST(WeightVectorTests, GenerateThreeDimensionalTwoValueVector)
{
	std::vector<std::vector<float>> output;
	detail::generateWeightVectors(output, 2, 3);
	ASSERT_EQ(2, output.size());
	auto expectation1 = ElementsAreClose(0.0f, 0.0f, 1.0f);
	auto expectation2 = ElementsAreClose(0.0f, 1.0f, 0.0f);
	auto expectation3 = ElementsAreClose(1.0f, 0.0f, 0.0f);
	EXPECT_THAT(output[0], AnyOf(expectation1, expectation2, expectation3));
	EXPECT_THAT(output[1], AnyOf(expectation1, expectation2, expectation3));
	EXPECT_NE(output[0], output[1]);
}

TEST(WeightVectorTests, GenerateThreeDimensionalThreeValueVector)
{
	std::vector<std::vector<float>> output;
	detail::generateWeightVectors(output, 3, 3);
	ASSERT_EQ(3, output.size());
	auto expectation1 = ElementsAreClose(0.0f, 0.0f, 1.0f);
	auto expectation2 = ElementsAreClose(0.0f, 1.0f, 0.0f);
	auto expectation3 = ElementsAreClose(1.0f, 0.0f, 0.0f);
	EXPECT_THAT(output[0], expectation1);
	EXPECT_THAT(output[1], expectation2);
	EXPECT_THAT(output[2], expectation3);
}

TEST(WeightVectorTests, GenerateThreeDimensionalFourValueVector)
{
	std::vector<std::vector<float>> output;
	detail::generateWeightVectors(output, 4, 3);
	ASSERT_EQ(4, output.size());
	auto expectation1 = ElementsAreClose(0.0f, 0.0f, 1.0f);
	auto expectation2 = ElementsAreClose(0.0f, 1.0f, 0.0f);
	auto expectation3 = ElementsAreClose(1.0f, 0.0f, 0.0f);
	auto expectation4 = ElementsAreClose(0.0f, 0.5f, 0.5f);
	auto expectation5 = ElementsAreClose(0.5f, 0.0f, 0.5f);
	auto expectation6 = ElementsAreClose(0.5f, 0.5f, 0.0f);
	EXPECT_THAT(output[0], AnyOf(expectation1, expectation2, expectation3, expectation4, expectation5, expectation6));
	EXPECT_THAT(output[1], AnyOf(expectation1, expectation2, expectation3, expectation4, expectation5, expectation6));
	EXPECT_THAT(output[2], AnyOf(expectation1, expectation2, expectation3, expectation4, expectation5, expectation6));
	EXPECT_THAT(output[3], AnyOf(expectation1, expectation2, expectation3, expectation4, expectation5, expectation6));
	EXPECT_NE(output[0], output[1]);
	EXPECT_NE(output[0], output[2]);
	EXPECT_NE(output[0], output[3]);
	EXPECT_NE(output[1], output[2]);
	EXPECT_NE(output[1], output[3]);
}

TEST(WeightVectorTests, GenerateThreeDimensionalSixValueVector)
{
	std::vector<std::vector<float>> output;
	detail::generateWeightVectors(output, 6, 3);
	ASSERT_EQ(6, output.size());
	auto expectation1 = ElementsAreClose(0.0f, 0.0f, 1.0f);
	auto expectation2 = ElementsAreClose(0.0f, 1.0f, 0.0f);
	auto expectation3 = ElementsAreClose(1.0f, 0.0f, 0.0f);
	auto expectation4 = ElementsAreClose(0.0f, 0.5f, 0.5f);
	auto expectation5 = ElementsAreClose(0.5f, 0.0f, 0.5f);
	auto expectation6 = ElementsAreClose(0.5f, 0.5f, 0.0f);
	EXPECT_THAT(output[0], expectation1);
	EXPECT_THAT(output[1], expectation2);
	EXPECT_THAT(output[2], expectation3);
	EXPECT_THAT(output[3], expectation4);
	EXPECT_THAT(output[4], expectation5);
	EXPECT_THAT(output[5], expectation6);
}

TEST(TchebycheffTests, SolutionToChebycheffDiagonal)
{
	std::array<float, 2> solution = { 0.0f, 0.0f };
	std::array<float, 2> reference = { 1.0f, 1.0f };
	std::array<float, 2> output;
	detail::solutionToChebycheff(reference, solution, output);
	EXPECT_THAT(output, ElementsAreClose(0.5f, 0.5f));
}

TEST(TchebycheffTests, SolutionToChebycheffFullWeightOneDimension)
{
	std::array<float, 2> solution = { 0.0f, 0.0f };
	std::array<float, 2> reference = { 0.000000000000001f, 1.0f };
	std::array<float, 2> output;
	detail::solutionToChebycheff(reference, solution, output);
	EXPECT_THAT(output, ElementsAreClose(1.0f, 0.0f));
}

TEST(TchebycheffTests, SolutionToChebycheffFullWeightOneDimensionSameAsReference)
{
	std::array<float, 2> solution = { 1.0f, 1.0f };
	std::array<float, 2> reference = { 2.0f, 1.0f };
	std::array<float, 2> output;
	detail::solutionToChebycheff(reference, solution, output);
	EXPECT_THAT(output, ElementsAreClose(0.0f, 1.0f));
}

TEST(TchebycheffTests, SolutionToChebycheffSameAsReference)
{
	std::array<float, 2> solution = { 1.0f, 1.0f };
	std::array<float, 2> reference = { 1.0f, 1.0f };
	std::array<float, 2> output;
	detail::solutionToChebycheff(reference, solution, output);
	EXPECT_THAT(output, ElementsAreClose(0.5f, 0.5f));
}

TEST(TchebycheffTests, SolutionToChebycheffThreeDimensionalOneElementTheSame)
{
	std::array<float, 3> solution = { 0.0f, 1.0f, 0.0f };
	std::array<float, 3> reference = { 1.0f, 1.0f, 1.0f };
	std::array<float, 3> output;
	detail::solutionToChebycheff(reference, solution, output);
	EXPECT_THAT(output, ElementsAreClose(0.0f, 1.0f, 0.0f));
}

TEST(TchebycheffTests, SolutionToChebycheffThreeDimensionalTwoElementsBetween)
{
	std::array<float, 3> solution = { 0.5f, 0.5f, 0.0f };
	std::array<float, 3> reference = { 1.0f, 1.0f, 1.0f };
	std::array<float, 3> output;
	detail::solutionToChebycheff(reference, solution, output);
	EXPECT_THAT(output, ElementsAreClose(0.4f, 0.4f, 0.2f));
}