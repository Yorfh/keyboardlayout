#include "optimizer.hpp"
#include "gtest/gtest.h"
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

TEST(OptimizerTSPTests, Burma14)
{
	auto citiesX = make_array(
		16.47f,
		16.47f,
		20.09f,
		22.39f,
		25.23f,
		22.00f,
		20.47f,
		17.20f,
		16.30f,
		14.05f,
		16.53f,
		21.52f,
		19.41f,
		20.09f
	);
	auto citiesY = make_array(
		96.10f,
		94.44f,
		92.54f,
		93.37f,
		97.24f,
		96.05f,
		97.02f,
		96.29f,
		97.38f,
		98.12f,
		97.38f,
		95.59f,
		97.13f,
		94.55f
		); 
	auto convertToRadians = [](auto v)
	{
		const float PI = 3.141592f;
		float deg = std::floor(v);
		float m = v - deg;
		float rad = PI * (deg + 5.0f * m / 3.0f) / 180.0f;
		return rad;
	};
	std::transform(citiesX.begin(), citiesX.end(), citiesX.begin(), convertToRadians);
	std::transform(citiesY.begin(), citiesY.end(), citiesY.begin(), convertToRadians);
	Keyboard<14> keyboard;

	auto evaluate = [&citiesX, &citiesY](const Keyboard<14>& keyboard)
	{
		float distance = 0.0f;
		for (size_t i = 0; i < 15; i++)
		{
			size_t index1;
			size_t index2;
			if (i == 0)
			{
				index1 = 0;
				index2 = keyboard.m_keys[0];
			}
			else if (i == 14)
			{
				index1 = keyboard.m_keys[13];
				index2 = 0;
			}
			else
			{
				index1 = keyboard.m_keys[i - 1];
				index2 = keyboard.m_keys[i];

			}
			float x1 = citiesX[index1];
			float x2 = citiesX[index2];
			float y1 = citiesY[index1];
			float y2 = citiesY[index2];
			const float RRR = 6378.388f;
			float q1 = std::cos(x1 - x2);
			float q2 = std::cos(y1 - y2);
			float q3 = std::cos(y1 + y2);
			float dij = std::floor(RRR * std::acos(0.5f*((1.0f + q1)*q2 - (1.0f - q1)*q3)) + 1.0f);
			distance += dij;
		}
		return distance;
	};
	Optimizer<14> o;
	o.populationSize(3);
	o.localSearchDept(20);
	o.numIterations(10);
	auto objectives = { TestObjective<14>(evaluate) };
	auto& solutions = o.optimize(std::begin(objectives), std::end(objectives), 20);
	ASSERT_EQ(1, solutions.size());
	auto result = solutions.getResult()[0].first;
	int resultValue = static_cast<int>(std::round(evaluate(result)));
	EXPECT_EQ(3323, resultValue);
}
