#include "BMAOptimizer.hpp"
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

TEST(BMAOptimizerTests, IncreasingOrder)
{
	auto evaluate = [](const Keyboard<3>& keyboard)
	{
		return keyboard.m_keys[0] * 10.0f +
			keyboard.m_keys[1] * 100.0f +
			keyboard.m_keys[2] * 1000.0f;
	};
	BMAOptimizer<3> o; 
	o.populationSize(1);
	auto objective = TestObjective<3>(evaluate);
	auto& solution = o.optimize(objective, 8);
	EXPECT_THAT(std::get<1>(solution).m_keys, ElementsAre(0, 1, 2));
}

TEST(BMAOptimizerTests, DecreasingOrder)
{
	auto evaluate = [](const Keyboard<3>& keyboard)
	{
		return keyboard.m_keys[0] * 1000.0f +
			keyboard.m_keys[1] * 100.0f +
			keyboard.m_keys[2] * 10.0f;
	};
	BMAOptimizer<3> o; 
	o.populationSize(1);
	auto objective = TestObjective<3>(evaluate);
	auto& solution = o.optimize(objective, 8);
	EXPECT_THAT(std::get<1>(solution).m_keys, ElementsAre(2, 1, 0));
}