#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mQAP.hpp"
#include "Optimizer.hpp"
#include <fstream>

using namespace testing;

TEST(mQAPTests, ObjectiveFunctionWorksCorrectly)
{
	std::string filename = "../../tests/mQAPData/KC10-2fl-1uni.dat";
	mQAP<10> objective1(filename, 0);
	mQAP<10> objective2(filename, 1);
	Keyboard<10> keyboard;
	keyboard.m_keys = { 1, 2, 7, 9, 6, 5, 0, 4, 3, 8};
	EXPECT_EQ(-228322, objective1.evaluate(keyboard));
	EXPECT_EQ(-193446, objective2.evaluate(keyboard));
}

template<typename Solutions>
void checkResult(const std::string& resultFilename, Solutions& solutions)
{
	std::vector<std::array<int, 10>> actual, expected;
	std::ifstream stream(resultFilename);
	while (stream)
	{
		expected.emplace_back();
		for (size_t i = 0; i < 10; i++)
		{
			stream >> expected.back()[i];
			expected.back()[i]--;
			if (!stream)
			{
				expected.pop_back();
				break;
			}
		}
		std::getline(stream, std::string());
	}

	ASSERT_EQ(expected.size(), solutions.size());
	auto solutionResult = solutions.getResult();
	for (auto&& r : solutionResult)
	{
		actual.emplace_back();
		std::copy(r.m_keyboard.m_keys.begin(), r.m_keyboard.m_keys.end(), actual.back().begin());
	}
	std::sort(actual.begin(), actual.end());
	EXPECT_THAT(actual, ElementsAreArray(expected));
}

TEST(mQAPTests, KC10_2fl_1uni)
{
	std::string filename = "../../tests/mQAPData/KC10-2fl-1uni.dat";
	mQAP<10> objective1(filename, 0);
	mQAP<10> objective2(filename, 1);
	Optimizer<10, 2> o;
	o.populationSize(902);
	o.initialTemperature(848.8709f, 447.3805f, 410);
	o.fastCoolingTemperature(675.0417f, 566.9724f, 396);
	auto objectives = { objective1, objective2 };
	auto& solutions = o.optimize(std::begin(objectives), std::end(objectives), 200000);
	checkResult("../../tests/mQAPData/KC10-2fl-1uni.po", solutions);
}

TEST(mQAPTests, KC10_2fl_1rl)
{
	std::string filename = "../../tests/mQAPData/KC10-2fl-1rl.dat";
	mQAP<10> objective1(filename, 0);
	mQAP<10> objective2(filename, 1);
	Optimizer<10, 2> o;
	o.populationSize(363);
	o.initialTemperature(860.2982f, 321.2859f, 195);
	o.fastCoolingTemperature(598.3387f, 155.8366f, 150);
	auto objectives = { objective1, objective2 };
	auto& solutions = o.optimize(std::begin(objectives), std::end(objectives), 200000);
	checkResult("../../tests/mQAPData/KC10-2fl-1rl.po", solutions);
}