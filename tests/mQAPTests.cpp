#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mQAP.hpp"
#include "Optimizer.hpp"
#include <fstream>

using namespace testing;

TEST(mQAPTests, KC10_2fl_1uni)
{
	std::string filename = "../../tests/mQAPData/KC10-2fl-1uni.dat";
	mQAP<10> objective1(filename, 0);
	mQAP<10> objective2(filename, 1);
	Optimizer<10> o;
	o.populationSize(50);
	o.numIterations(20);
	o.temperature(252.0f, 3.9513f, 5000);
	auto objectives = { objective1, objective2 };
	auto& solutions = o.optimize(std::begin(objectives), std::end(objectives));
	std::vector<std::array<int, 10>> actual, expected;
	std::string resultFilename = "../../tests/mQAPData/KC10-2fl-1uni.po";
	std::ifstream stream(resultFilename);
	while (stream)
	{
		expected.emplace_back();
		for (size_t i = 0; i < 10; i++)
		{
			stream >> expected.back()[i];
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
	EXPECT_THAT(actual, UnorderedElementsAreArray(expected));
}