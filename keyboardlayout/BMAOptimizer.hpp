#pragma once
#include <functional>
#include "Keyboard.hpp"
#include "NonDominatedSet.hpp"
#include <random>
#include <vector>
#include <utility>
#include <numeric>

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
		size_t triangularNumber = static_cast<size_t>(static_cast<float>(KeyboardSize * (KeyboardSize + 1)) / 2.0f);
		size_t numIterations = std::max<size_t>(numEvaluations / triangularNumber, 1);
		generateRandomPopulation(begin, end);
		shortImprovement(begin, end, numIterations);
		updateNonDominatedSet();
		return m_NonDominatedSet;
	}

protected:
	typedef std::array<std::array<float, KeyboardSize>, KeyboardSize> DeltaArray;
	typedef std::array<std::array<size_t, KeyboardSize>, KeyboardSize> IndexArray;

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
	void shortImprovement(Itr begin, Itr end, size_t maxIterations)
	{
		for (size_t i = 0; i < m_populationSize; i++)
		{
			//TODO: Hardcoded number of iterations
			localSearch(i, std::min<size_t>(maxIterations, 5000), begin, end);
		}
	}

	template<typename Itr>
	void localSearch(size_t index, size_t numIterations, Itr begin, Itr end)
	{
		auto& keyboard = m_population[index];
		auto& solution = m_populationSolutions[index];

		Keyboard<KeyboardSize> currentKeyboard = keyboard;

		DeltaArray delta;
		IndexArray lastSwapped;
		IndexArray frequency;

		size_t iterWithoutImprovement = 0;
		size_t iterLastImprovement = 0;

		bool escapedLocalMinimum = true;
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
				delta[i][j] = computeDelta(currentKeyboard, solution[0], i, j, begin, end);
			}
		}

		float currentCost = solution[0];
		float bestCost = solution[0];

		size_t perturbStr = std::max<size_t>(static_cast<size_t>(std::ceil(m_jumpMagnitude * KeyboardSize)), 2);
		std::uniform_real_distribution<float> stagnationDistribution(m_minStagnationMagnitude, m_maxStagnationMagnitude);

		for (size_t currentIteration = 1; currentIteration <= numIterations; currentIteration++)
		{
			size_t iRetained = 0;
			size_t jRetained = 0;
			bool foundImprovement = false;

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
					iterWithoutImprovement = 0;
					iterLastImprovement = currentIteration;
					solution[0] = currentCost;
					keyboard = currentKeyboard;

				}
				for (size_t i = 0;i < KeyboardSize; i++)
				{
					for (size_t j = i + 1;j < KeyboardSize; j++)
					{
						delta[i][j] = computeDelta(currentKeyboard, currentCost, i, j, begin, end);
					}
				}
				//update_matrix_of_move_cost(i_retained, j_retained, n, delta, p, a, b);
				escapedLocalMinimum = true;
			}
			else
			{
				if (iterWithoutImprovement > m_stagnationAfter)
				{
					iterWithoutImprovement = 0;
					perturbStr = std::max<size_t>(static_cast<size_t>(KeyboardSize * stagnationDistribution(m_randomGenerator)), 2);
				}
				else if (escapedLocalMinimum == true) // Escaped from the previous local optimum. New local optimum reached
				{
					iterWithoutImprovement++;					perturbStr = std::max<size_t>(static_cast<size_t>(std::ceil(m_jumpMagnitude * KeyboardSize)), 2);
				}
				else
				{					perturbStr += 1;
				}

				perturbe(currentKeyboard, delta, currentCost, lastSwapped, frequency, iterWithoutImprovement, solution[0], perturbStr, iteration, begin, end);
				escapedLocalMinimum = false;
			}
		};
	}
	
	template<typename Itr>
	float computeDelta(const Keyboard<KeyboardSize>& keyboard, float solution, size_t i, size_t j, Itr begin, Itr end)
	{
		Keyboard<KeyboardSize> k = keyboard;
		std::swap(k.m_keys[i], k.m_keys[j]);
		return begin->evaluate(k) - solution;
	}

	void updateNonDominatedSet()
	{
		for (size_t i = 0; i < m_populationSize; i++)
		{
			m_NonDominatedSet.insert(m_population[i], m_populationSolutions[i]);
		}
	}

	template<typename Itr>
	void perturbe(Keyboard<KeyboardSize>& p, DeltaArray& delta, float& currentCost, 
		IndexArray& lastSwapped, IndexArray& frequency, size_t iterWithoutImprovement, float& bestBestCost, size_t perturbStr, size_t& iteration, Itr begin, Itr end)
	{
		size_t iRetained = std::numeric_limits<size_t>::max();
		size_t jRetained = iRetained;
		float cost = currentCost;
		bool bit = false;
		float d = static_cast<float>(iterWithoutImprovement) / m_stagnationAfter;
		float e = std::exp(-d);
		e = std::min(0.75f, e);
		std::uniform_real_distribution<float> dist(0.0f, std::nextafter(1.0f, 2.0f));
		std::uniform_int_distribution<int> keyDist(0, KeyboardSize - 1);
		std::uniform_real_distribution<float> iterDist(0.0f, KeyboardSize * 0.2f);

		if (e > dist(m_randomGenerator))
			bit = true;
		for (size_t k = 0; k < perturbStr; k++)
		{
			if (bit)
			{
				float maxDelta = std::numeric_limits<float>::lowest();
				for (size_t i = 0;i < KeyboardSize; i++)
				{
					for (size_t j = i + 1;j < KeyboardSize; j++)
					{
						if ((currentCost + delta[i][j]) != cost && delta[i][j] > maxDelta &&
							(static_cast<size_t>(lastSwapped[i][j] + KeyboardSize * 0.9f + iterDist(m_randomGenerator)) < iteration || (currentCost + delta[i][j]) > bestBestCost))
						{
							iRetained = i; 
							jRetained = j;
							maxDelta = delta[i][j];
							//TODO: Doesn't seem to be a good idea, since it doesn't update the keyboard...
							if ((currentCost + delta[i][j]) > bestBestCost)
							{
								bestBestCost = currentCost + delta[i][j];
							}
						}
					}
				}
			}
			else
			{
				iRetained = keyDist(m_randomGenerator);
				jRetained = keyDist(m_randomGenerator);
				if (iRetained > jRetained)
					std::swap(iRetained, jRetained);
				while (iRetained == jRetained || (currentCost + delta[iRetained][jRetained]) == cost)
				{
					jRetained = keyDist(m_randomGenerator);
					if (iRetained > jRetained)
						std::swap(iRetained, jRetained);
				}
			}

			if (iRetained != std::numeric_limits<size_t>::max())
			{
				lastSwapped[iRetained][jRetained] = iteration;
				frequency[iRetained][jRetained] = frequency[iRetained][jRetained] + 1;
				std::swap(p.m_keys[iRetained], p.m_keys[jRetained]);
				currentCost = currentCost + delta[iRetained][jRetained];
				for (size_t i = 0;i < KeyboardSize; i++)
				{
					for (size_t j = i + 1;j < KeyboardSize; j++)
					{
						delta[i][j] = computeDelta(p, currentCost, i, j, begin, end);
					}
				}
				//update_matrix_of_move_cost(i_retained, j_retained, n, delta, p, a, b);
			}
			iteration++;
		}
	}
	std::vector<Keyboard<KeyboardSize>> m_population;
	std::vector<std::vector<float>> m_populationSolutions;
	size_t m_populationSize = 0;
	float m_jumpMagnitude = 0.15f;
	size_t m_stagnationAfter = 250;
	float m_minStagnationMagnitude = 0.2f;
	float m_maxStagnationMagnitude = 0.4f;
	std::mt19937 m_randomGenerator;
	NonDominatedSet<KeyboardSize, NumObjectives, MaxLeafSize> m_NonDominatedSet;
};

template<size_t KeyboardSize, size_t NumObjectives, size_t MaxLeafSize>
std::random_device BMAOptimizer<KeyboardSize, NumObjectives, MaxLeafSize>::rd;