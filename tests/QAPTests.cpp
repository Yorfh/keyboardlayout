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

TEST(QAPTests, NeighbourhoodFunctionWorksCorrectly)
{
	std::string filename = "../../tests/QAPData/chr12a.dat";
	QAP<12> objective(filename);
	Keyboard<12> keyboard;
	keyboard.m_keys = { 6, 4, 11, 1, 0, 2, 8, 10, 9, 5, 7, 3 };
	std::array<std::array<float, 12>, 12> delta;
	float startValue = objective.evaluate(keyboard);
	objective.evaluateFirstNeighbourhood(keyboard, startValue, delta);
	for (size_t i = 0; i < 12; i++)
	{
		for (size_t j = i + 1; j < 12; j++)
		{
			Keyboard<12> k2 = keyboard;
			std::swap(k2.m_keys[i], k2.m_keys[j]);
			float v = objective.evaluate(k2);
			SCOPED_TRACE(i);
			SCOPED_TRACE(j);
			SCOPED_TRACE(startValue);
			SCOPED_TRACE(delta[i][j]);
			EXPECT_EQ(v, startValue + delta[i][j]);
		}
	}
}

TEST(QAPTests, SwappedNeighbourhoodFunctionWorksCorrectly)
{
	std::string filename = "../../tests/QAPData/chr12a.dat";
	QAP<12> objective(filename);
	Keyboard<12> keyboard;
	keyboard.m_keys = { 6, 4, 11, 1, 0, 2, 8, 10, 9, 5, 7, 3 };
	std::array<std::array<float, 12>, 12> delta;
	float startValue = objective.evaluate(keyboard);
	objective.evaluateFirstNeighbourhood(keyboard, startValue, delta);
	std::swap(keyboard.m_keys[5], keyboard.m_keys[7]);
	startValue = objective.evaluate(keyboard);
	objective.evaluateNeighbourhood(keyboard, startValue, 5, 7, delta);
	for (size_t i = 0; i < 12; i++)
	{
		for (size_t j = i + 1; j < 12; j++)
		{
			Keyboard<12> k2 = keyboard;
			std::swap(k2.m_keys[i], k2.m_keys[j]);
			float v = objective.evaluate(k2);
			SCOPED_TRACE(i);
			SCOPED_TRACE(j);
			SCOPED_TRACE(startValue);
			SCOPED_TRACE(delta[i][j]);
			EXPECT_EQ(v, startValue + delta[i][j]);
		}
	}
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
	auto& solution = o.optimize(objective, 200000);
	// The reverse direction is also a solution
	int resultValue = static_cast<int>(-std::round(std::get<0>(solution)));
	EXPECT_EQ(9552, resultValue);
}