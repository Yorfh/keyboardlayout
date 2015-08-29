#pragma once
#include <functional>
#include "Keyboard.hpp"
#include "NonDominatedSet.hpp"
#include <random>
#include <vector>
#include <utility>
#include <numeric>
#include <boost/math/special_functions/binomial.hpp>

template<size_t KeyboardSize>
class Objective;

// The algorithm is based on "Memetic search for the quadratic assignment problem" (Una Benlic and Jin-Kao Hao)

template<size_t KeyboardSize, size_t NumObjectives, size_t MaxLeafSize = std::numeric_limits<size_t>::max()>
class BMAOptimizer
{
	static std::random_device rd;
public:
	BMAOptimizer(unsigned int seed = BMAOptimizer::rd())
	{
		m_randomGenerator.seed(seed);
	}

	void populationSize(size_t size)
	{
		m_populationSize = size;
	}

	template<typename Solution, typename Itr>
	void evaluate(Solution& solution, Keyboard<KeyboardSize>& keyboard, Itr begin, Itr end)
	{
		std::transform(begin, end, solution.begin(),
			[&keyboard](typename std::iterator_traits<Itr>::reference objective)
		{
			return objective.evaluate(keyboard);
		});
	}

	template<typename Itr>
	const NonDominatedSet<KeyboardSize, NumObjectives, MaxLeafSize>& optimize(Itr begin, Itr end, size_t numEvaluations)
	{
		generateRandomPopulation(begin, end);
		shortImprovement(begin, end);
		updateNonDominatedSet();
		return m_NonDominatedSet;
	}

protected:
	template<typename Itr>
	void generateRandomPopulation(Itr begin, Itr end)
	{
		m_population.resize(m_populationSize);
		m_populationSolutions.resize(m_populationSize);

		for (auto i = 0; i < m_populationSize; i++)
		{
			m_population[i].randomize(m_randomGenerator);
			m_populationSolutions[i].resize(end - begin);
			Keyboard<KeyboardSize> keyboard = m_population[i];
			evaluate(m_populationSolutions[i], keyboard, begin, end);
		}

	}
	
	template<typename Itr>
	void shortImprovement(Itr begin, Itr end)
	{
		for (size_t i = 0; i < m_populationSize; i++)
		{
			//TODO: Hardcoded number of iterations
			localSearch(i, 5000, begin, end);
		}
	}

	template<typename Itr>
	void localSearch(size_t index, size_t numIterations, Itr begin, Itr end)
	{
		auto& keyboard = m_population[index];
		auto& solution = m_populationSolutions[index];

		Keyboard<KeyboardSize> currentKeyboard = keyboard;

		//type_vector p;                        // current solution

		//type_matrix delta;                    // store move costs
		typedef std::array<std::array<float, KeyboardSize>, KeyboardSize> DeltaArray;

		DeltaArray delta;
		std::array<std::array<size_t, KeyboardSize>, KeyboardSize> lastSwapped;
		std::array<std::array<size_t, KeyboardSize>, KeyboardSize> frequency;

		//int i, j, k, i_retained, j_retained, bit;  // indices
		size_t iter_without_improvement = 0;
		size_t iter_last_improvement = 0;
		//perturb_str = init_ptr*n;

		bool perturbed_once = false;
		long previous_cost = 1;
		size_t iteration = 0;

		for (size_t i = 0;i < KeyboardSize;i++)
		{
			lastSwapped[i].fill(0);
			frequency[i].fill(0);
		}

		for (size_t i = 0;i < KeyboardSize; i++)
		{
			for (size_t j = i + 1;j < KeyboardSize; j++)
			{
				delta[i][j] = computeDelta(currentKeyboard, solution, i, j, begin, end);
			}
		}

		float currentCost = solution[0];
		float bestCost = solution[0];

		for (size_t currentIteration = 1; currentIteration <= numIterations; currentIteration++)
		{
			size_t iRetained = 0;
			size_t jRetained = 0;

			float maxDelta = std::numeric_limits<float>::lowest();
			for (size_t i = 0; i < KeyboardSize; i++)
			{
				for (size_t j = 0;j < KeyboardSize; j++)
				{
					float d = delta[i][j];
					if (d > maxDelta)
					{
						maxDelta = d;
						iRetained = i;
						jRetained = j;
					}
				}
			}

			if ((currentCost + maxDelta) > currentCost)
			{
				std::swap(currentKeyboard.m_keys[iRetained], currentKeyboard.m_keys[jRetained]);
				lastSwapped[iRetained][jRetained] = iteration;
				frequency[iRetained][jRetained]++;
				iteration++;
				currentCost += delta[iRetained][jRetained];

				if (currentCost > solution[0])

				{
					iter_without_improvement = 0;
					iter_last_improvement = currentIteration;
					solution[0] = currentCost;
					keyboard = currentKeyboard;

				}
				for (size_t i = 0;i < KeyboardSize; i++)
				{
					for (size_t j = i + 1;j < KeyboardSize; j++)
					{
						delta[i][j] = computeDelta(currentKeyboard, solution, i, j, begin, end);
					}
				}
				//update_matrix_of_move_cost(i_retained, j_retained, n, delta, p, a, b);
				perturbed_once = false;
			}
			else
			{
#if 0
				if (iter_without_improvement > 250)
				{
					iter_without_improvement = 0;
					perturb_str = n*(0.2 + (static_cast<double>(rand() % 20) / 100.0));
				}
				else if ((perturbed_once == false and previous_cost != current_cost) or (perturbed_once and previous_cost != current_cost)) // Escaped from the previous local optimum. New local optimum reached
				{
					iter_without_improvement++;
					perturb_str = ceil(n*init_ptr); if (perturb_str < 2) perturb_str = 2;
				}
				else if (previous_cost == current_cost)//  or (perturbed_once and previous_cost != current_cost))
					perturb_str += 1;



				previous_cost = current_cost;
				perturbe(p, n, delta, current_cost, a, b, last_swaped, frequency, iter_without_improvement, best_best_cost);
				perturbed_once = true;
#endif

			}

		};
	}
	
	template<typename Itr>
	float computeDelta(const Keyboard<KeyboardSize>& keyboard, const std::vector<float> solution, size_t i, size_t j, Itr begin, Itr end)
	{
		Keyboard<KeyboardSize> k = keyboard;
		std::swap(k.m_keys[i], k.m_keys[j]);
		return begin->evaluate(k) - solution[0];
	}

	void updateNonDominatedSet()
	{
		for (size_t i = 0; i < m_populationSize; i++)
		{
			m_NonDominatedSet.insert(m_population[i], m_populationSolutions[i]);
		}
	}
	std::vector<Keyboard<KeyboardSize>> m_population;
	std::vector<std::vector<float>> m_populationSolutions;
	size_t m_populationSize = 0;
	std::mt19937 m_randomGenerator;
	NonDominatedSet<KeyboardSize, NumObjectives, MaxLeafSize> m_NonDominatedSet;
};

template<size_t KeyboardSize, size_t NumObjectives, size_t MaxLeafSize>
std::random_device BMAOptimizer<KeyboardSize, NumObjectives, MaxLeafSize>::rd;