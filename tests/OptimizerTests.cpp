#include "optimizer.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <array>
#include "Objective.hpp"

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


TEST(OptimizerTests, IncreasingOrder)
{
	auto evaluate = [](const Keyboard<3>& keyboard)
	{
		return keyboard.m_keys[0] * 10.0f +
			keyboard.m_keys[1] * 100.0f +
			keyboard.m_keys[2] * 1000.0f;
	};
	auto o = Optimizer<3>(); 
	auto objectives = { TestObjective<3>(evaluate) };
	auto keyboard = o.optimize(std::begin(objectives), std::end(objectives), 20);
	EXPECT_THAT(keyboard.m_keys, ElementsAre(0, 1, 2));
}

TEST(OptimizerTests, DecreasingOrder)
{
	auto evaluate = [](const Keyboard<3>& keyboard)
	{
		return keyboard.m_keys[0] * 1000.0f +
			keyboard.m_keys[1] * 100.0f +
			keyboard.m_keys[2] * 10.0f;
	};
	auto o = Optimizer<3>(); 
	auto objectives = { TestObjective<3>(evaluate) };
	auto keyboard = o.optimize(std::begin(objectives), std::end(objectives), 20);
	EXPECT_THAT(keyboard.m_keys, ElementsAre(2, 1, 0));
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
