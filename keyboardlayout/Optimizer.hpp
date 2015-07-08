#pragma once
#include <functional>
#include "Keyboard.hpp"
#include "NonDominatedSet.hpp"
#include <random>
#include <vector>
#include <utility>

template<size_t KeyboardSize>
class Objective;

namespace detail
{
	template<size_t Size>
	Keyboard<Size> partiallyMatchedCrossover(const Keyboard<Size>& parent1, const Keyboard<Size>& parent2, size_t p1, size_t p2)
	{
		Keyboard<Size> child(parent1.m_keys);
		std::array<size_t, Size> mapping;
		for (size_t i = 0;i < Size;i++)
		{
			mapping[parent1.m_keys[i]] = i;
		}

		for (auto i = p1;i <= p2; i++)
		{
			auto v = parent2.m_keys[i];
			size_t index1 = i;
			size_t index2 = mapping[v];
			size_t value1 = child.m_keys[index1];
			size_t value2 = child.m_keys[index2];
			std::swap(child.m_keys[index1], child.m_keys[index2]);
			std::swap(mapping[value1], mapping[value2]);
		}
		return child;
	}
}

template<size_t KeyboardSize>
class Optimizer
{
	static std::random_device rd;
public:
	Optimizer(unsigned int seed = Optimizer::rd())
	{
		m_randomGenerator.seed(seed);
	}

	void populationSize(size_t size)
	{
		m_populationSize = size;
	}

	void localSearchDept(size_t size)
	{
		m_localSearchDepth = size;
	}

	void temperature(float maxT, float minT, size_t numSteps)
	{
		m_minT = minT;
		m_maxT = maxT;
		m_numTSteps = numSteps;
	}

	void numIterations(size_t num)
	{
		m_numIterations = num;
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
	const NonDominatedSet<KeyboardSize>& optimize(Itr begin, Itr end)
	{
		// The algorithm is based on 
		// "An Adaptive Evolutionary Multi-objective Approach Based on Simulated Annealing"
		// "A Simulated Annealing based Genetic Local Search Algorithm for Multi-objective Multicast Routing Problems"

		selectWeightVectors(end - begin);
		std::vector<float> solution;
		solution.resize(end - begin);
		m_population.resize(m_populationSize);
		m_populationSolutions.resize(m_populationSize);
		m_tempSolution.resize(end - begin);

		for (auto i = 0; i < m_populationSize; i++)
		{
			m_population[i].randomize(m_randomGenerator);
			m_populationSolutions[i].resize(end - begin);
			Keyboard<KeyboardSize> keyboard = m_population[i];
			evaluate(m_populationSolutions[i], keyboard, begin, end);
		}
		m_NonDominatedSet = NonDominatedSet<KeyboardSize>(m_population, m_populationSolutions);

		for (size_t iteration = 0; iteration < m_numIterations; iteration++)
		{
			for (size_t i = 0; i < m_populationSize; ++i)
			{
				Keyboard<KeyboardSize> newKeyboard;
				simulatedAnnealing(i, begin, end, newKeyboard, solution);
				updatePopulation(i, newKeyboard, solution);
			}
			for (auto i = 0; i < m_populationSize; i++)
			{
				adaptWeightVectors();
			}
		}
		return m_NonDominatedSet;
	}

protected:
	template<typename Itr>
	void simulatedAnnealing(size_t index, Itr begin, Itr end, Keyboard<KeyboardSize>& outKeyboard, std::vector<float>& outSolution)
	{
		auto probability = std::uniform_real_distribution<float>(0, 1.0);
		outKeyboard = m_population[index];
		outSolution = m_populationSolutions[index];
		float alpha = std::pow(m_minT / m_maxT, 1.0f / m_numTSteps);
		for (float currentT = m_maxT; currentT >= m_minT; currentT *= alpha)
		{
			auto neighbour = mutate(outKeyboard);
			evaluate(m_tempSolution, neighbour, begin, end);
			if (!isDominated(m_tempSolution, outSolution))
			{
				m_NonDominatedSet.insert(neighbour, m_tempSolution);
			}
			if (annealingProbability(outSolution, m_tempSolution, m_weights[index], currentT) > probability(m_randomGenerator))
			{
				outKeyboard = neighbour;
				outSolution.swap(m_tempSolution);
			}
		}
	}

	float annealingProbability(const std::vector<float>& first, const std::vector<float>& second, std::vector<float>& weights, float t)
	{ 
		float sFirst =  weightedSum(first, weights);
		float sSecond = weightedSum(second, weights);
		float p = std::exp(-((sFirst - sSecond) / t));
		p = std::min(1.0f, p);
		return p;
	}

	void updatePopulation(size_t index, const Keyboard<KeyboardSize>& keyboard, std::vector<float>& solution)
	{
		float solutionValue = weightedSum(solution, m_weights[index]);
		float populationValue = weightedSum(m_populationSolutions[index], m_weights[index]);
		if (solutionValue > populationValue)
		{
			m_population[index] = keyboard;
			m_populationSolutions[index] = solution;
		}
	}

	float weightedSum(const std::vector<float>& solution, const std::vector<float>& weights)
	{
		auto weight = std::begin(weights);
		float sum = 0.0f;
		for (auto&& s : solution)
		{
			sum += s * (*weight);
			++weight;
		}
		return sum;
	}

	void selectWeightVectors(size_t numObjectives)
	{
		m_weights.reserve(m_populationSize);
		for (size_t i = 0; i < m_populationSize;i++)
		{
			m_weights.emplace_back(numObjectives, 1.0f);
		}
	}

	void adaptWeightVectors()
	{
	}

	size_t selectParent(std::vector<float>& fitnesses)
	{
		auto parentSelector = std::uniform_int_distribution<>(0, PopulationSize - 1);
		auto parent1 = parentSelector(m_randomGenerator);
		auto parent2 = parentSelector(m_randomGenerator);
		while (parent2 == parent1)
		{
			parent2 = parentSelector(m_randomGenerator);
		}
		if (fitnesses[parent1] > fitnesses[parent2])
		{
			return parent1;
		}
		else
		{
			return parent2;
		}
	}

	std::pair<size_t, size_t> selectParents(std::vector<float> fitnesses)
	{
		auto parent1 = selectParent(fitnesses);
		auto parent2 = selectParent(fitnesses);
		while (parent1 != parent2)
		{
			parent2 = selectParent(fitnesses);
		}
		return std::make_pair(parent1, parent2);
	}

	Keyboard<KeyboardSize> produceChild(const Keyboard<KeyboardSize>& parent1, const Keyboard<KeyboardSize>& parent2)
	{
		auto dist = std::uniform_int_distribution<size_t>(0, parent1.m_keys.size()-1);
		auto p1 = dist(m_randomGenerator);
		auto p2 = dist(m_randomGenerator);
		if (p2 < p1)
		{
			std::swap(p1, p2);
		}
		return detail::partiallyMatchedCrossover(parent1, parent2, p1, p2);
	}

	Keyboard<KeyboardSize> mutate(const Keyboard<KeyboardSize>& keyboard)
	{
		Keyboard<KeyboardSize> ret = keyboard;
		auto dist = std::uniform_int_distribution<size_t>(0, keyboard.m_keys.size()-1);
		auto k1 = dist(m_randomGenerator);
		auto k2 = dist(m_randomGenerator);
		std::swap(ret.m_keys[k1], ret.m_keys[k2]);
		return ret;
	}


	std::mt19937 m_randomGenerator;
	NonDominatedSet<KeyboardSize> m_NonDominatedSet;
	std::vector<Keyboard<KeyboardSize>> m_population;
	std::vector<std::vector<float>> m_populationSolutions;
	std::vector<std::vector<float>> m_weights;
	std::vector<float> m_tempSolution;
	size_t m_populationSize = 0;
	size_t m_localSearchDepth = 0;
	size_t m_numIterations = 0;
	float m_maxT = 1.0f;
	float m_minT = 0.1f;
	size_t m_numTSteps = 10;
};

template<size_t KeyboardSize>
std::random_device Optimizer<KeyboardSize>::rd;