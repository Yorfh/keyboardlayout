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
		std::copy(r.first.m_keys.begin(), r.first.m_keys.end(), actual.back().begin());
	}
	std::sort(actual.begin(), actual.end());
	EXPECT_THAT(actual, ElementsAreArray(expected));
}

TEST(mQAPTests, KC10_2fl_1uni)
{
	std::string filename = "../../tests/mQAPData/KC10-2fl-1uni.dat";
	mQAP<10> objective1(filename, 0);
	mQAP<10> objective2(filename, 1);
	Optimizer<10> o;
	o.populationSize(50);
	o.initialTemperature(988, 900, 1000);
	o.fastCoolingTemperature(998, 900, 1000);
	auto objectives = { objective1, objective2 };
	auto& solutions = o.optimize(std::begin(objectives), std::end(objectives), 50 * 20 * 1000);
	checkResult("../../tests/mQAPData/KC10-2fl-1uni.po", solutions);
}

TEST(mQAPTests, KC10_2fl_1rl)
{
	std::string filename = "../../tests/mQAPData/KC10-2fl-1rl.dat";
	mQAP<10> objective1(filename, 0);
	mQAP<10> objective2(filename, 1);
	Optimizer<10> o;
	o.populationSize(919);
	o.initialTemperature(871.6187f, 527.6194f, 662);
	o.fastCoolingTemperature(871.6187f, 527.6194f, 662);
	auto objectives = { objective1, objective2 };
	auto& solutions = o.optimize(std::begin(objectives), std::end(objectives), 100000);
	checkResult("../../tests/mQAPData/KC10-2fl-1rl.po", solutions);
}