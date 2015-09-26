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
	BMAOptimizer<12> o; 
	o.crossover(CrossoverType::Uniform);
	o.jumpMagnitude(0.05337941137576252f);
	o.improvementDepth(4613, 4644);
	o.perturbType(PerturbType::Normal);
	o.minDirectedPertubation(0.07956319937402234f);
	o.populationSize(7);
	o.stagnation(792, 1.8702265013537944f, 9.90795080916275f);
	o.tabuTenure(0.6740803228413664f, 0.7841240524741843f);
	o.mutation(25, 0.887375951372175f, 10);
	o.tournamentPool(4);
	auto objectives = { objective };
	auto& solutions = o.optimize(std::begin(objectives), std::end(objectives), 200000);
	// The reverse direction is also a solution
	auto result = solutions.getResult()[0].m_keyboard;
	int resultValue = static_cast<int>(-std::round(objective.evaluate(result)));
	EXPECT_EQ(9552, resultValue);
}