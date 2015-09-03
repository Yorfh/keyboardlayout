#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "QAP.hpp"
#include "Keyboard.hpp"
#include "BMAOptimizer.hpp"

using namespace testing;


TEST(QAPTests, ObjectiveFunctionWorksCorrectly)
{
	std::string filename = "../../tests/QAPData/chr12a.dat";
	QAP<12> objective(filename);
	Keyboard<12> keyboard;
	keyboard.m_keys = { 6, 4, 11, 1, 0, 2, 8, 10, 9, 5, 7, 3 };
	EXPECT_EQ(-9552, objective.evaluate(keyboard));
}

TEST(QAPTests, QAPchr12a)
{
	std::string filename = "../../tests/QAPData/chr12a.dat";
	QAP<12> objective(filename);
	Keyboard<12> keyboard;
	BMAOptimizer<12, 1> o;
	o.populationSize(5);
	o.improvementDepth(1000, 5000);
	auto objectives = { objective };
	auto& solutions = o.optimize(std::begin(objectives), std::end(objectives), 100000);
	// The reverse direction is also a solution
	auto result = solutions.getResult()[0].m_keyboard;
	int resultValue = static_cast<int>(-std::round(objective.evaluate(result)));
	EXPECT_EQ(9552, resultValue);
}