#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "BMAOptimizer.hpp"
#include "BMAReference.hpp"
#include "QAP.hpp"
#include <time.h>
#include "boost/accumulators/accumulators.hpp"
#include "boost/accumulators/statistics/median.hpp"

using namespace testing;

TEST(BMAHeadToHeadTests, Tai30a)
{
	using namespace boost::accumulators;
	std::string filename = "../../tests/QAPData/tai30a.dat";
	QAP<30> objective(filename);
	size_t numBetter = 0;
	const size_t bestOf = 50;
	const int target = 1818146;
	size_t totalEvaluationsO = 0;
	size_t totalEvaluationsR = 0;
	accumulator_set<double, features<tag::median>> accO;
	accumulator_set<double, features<tag::median>> accR;
	for (size_t i = 0; i < bestOf; i++)
	{
		clock_t before = clock();
		Keyboard<30> keyboard;
		BMAOptimizer<30> o; 
		o.crossover(CrossoverType::PartiallyMatched);
		o.jumpMagnitude(0.14936300421738202f);
		o.improvementDepth(676);
		o.perturbType(PerturbType::Normal);
		o.minDirectedPertubation(0.4308978010001765f);
		o.populationSize(17);
		o.stagnation(727, 7.294493769597043f, 7.861197213337471f);
		o.tabuTenure(0.825741764848783f, 1.8923911140546275f);
		o.mutation(13, 0.8047553939922597f, 19);
		o.tournamentPool(6);
		o.target(-target);
		auto& solution = o.optimize(objective, 200000000);
		float t = (clock() - before) / static_cast<double>(CLOCKS_PER_SEC);
		printf("BMAOptimizer time %f\n", t);
		int resultValue = static_cast<int>(-std::round(std::get<0>(solution)));
		EXPECT_EQ(target, resultValue);

		BMAReference r;
		r.run(target, filename, "", 1);
		EXPECT_EQ(target, r.getLastResult());
		if (resultValue != r.getLastResult())
		{
			if (resultValue < r.getLastResult())
			{
				numBetter++;
			}
		}
		else if (o.getNumEvaluations() <= r.getNumEvaluations())
		{
			numBetter++;
		}
		printf("Num evaluations %zu vs %i\n", o.getNumEvaluations(), r.getNumEvaluations());
		totalEvaluationsO += o.getNumEvaluations();
		totalEvaluationsR += r.getNumEvaluations();
		accO(static_cast<double>(o.getNumEvaluations()));
		accR(static_cast<double>(r.getNumEvaluations()));
	}
	printf("TotalEvaluations %zu vs %zu\n", totalEvaluationsO, totalEvaluationsR);
	printf("Result %zu vs %zu\n", numBetter, bestOf - numBetter);
	printf("Median %f vs %f\n", median(accO), median(accR));
	EXPECT_GT(numBetter, bestOf - numBetter);
	EXPECT_LE(totalEvaluationsO, totalEvaluationsR);
}
