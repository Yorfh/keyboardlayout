#include "optimizer.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <array>
#include "Objective.hpp"

using namespace testing;

class TestObjective : public Objective
{
public:
	TestObjective(std::function<float(const Keyboard& keyboard)> func)
		: m_func(std::move(func))
	{

	}

	float evaluate(const Keyboard& keyboard) const override
	{
		return m_func(keyboard);
	}

	std::function<float(const Keyboard& keyboard)> m_func;
};

TEST(OptimizerTests, IncreasingOrder)
{
	auto evaluate = [](const Keyboard& keyboard)
	{
		return keyboard.m_keys[0] * 10.0f +
			keyboard.m_keys[1] * 100.0f +
			keyboard.m_keys[2] * 1000.0f;
	};
	auto o = Optimizer(); 
	auto objectives = { TestObjective(evaluate) };
	auto keyboard = o.optimize(std::begin(objectives), std::end(objectives), 20);
	EXPECT_THAT(keyboard.m_keys, ElementsAre(0, 1, 2));
}

TEST(OptimizerTests, DecreasingOrder)
{
	auto evaluate = [](const Keyboard& keyboard)
	{
		return keyboard.m_keys[0] * 1000.0f +
			keyboard.m_keys[1] * 100.0f +
			keyboard.m_keys[2] * 10.0f;
	};
	auto o = Optimizer(); 
	auto objectives = { TestObjective(evaluate) };
	auto keyboard = o.optimize(std::begin(objectives), std::end(objectives), 20);
	EXPECT_THAT(keyboard.m_keys, ElementsAre(2, 1, 0));
}
