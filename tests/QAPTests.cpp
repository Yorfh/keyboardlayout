#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "QAP.hpp"
#include "Keyboard.hpp"

using namespace testing;


TEST(QAPTests, ObjectiveFunctionWorksCorrectly)
{
	std::string filename = "../../tests/QAPData/chr12a.dat";
	QAP<12> objective(filename);
	Keyboard<12> keyboard;
	keyboard.m_keys = { 6, 4, 11, 1, 0, 2, 8, 10, 9, 5, 7, 3 };
	EXPECT_EQ(-9552, objective.evaluate(keyboard));
}
