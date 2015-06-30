#include "optimizer.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <array>
#include "Objective.hpp"
#include "MakeArray.hpp"

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
	o.localSearchDept(0);
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
	o.localSearchDept(0);
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
	o.localSearchDept(20);
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
	o.localSearchDept(20);
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
	o.localSearchDept(20);
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
