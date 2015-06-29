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
public:
	Optimizer()
	{
		std::random_device rd;
		m_randomGenerator.seed(rd());
	}

	void populationSize(size_t size)
	{
		m_populationSize = size;
	}

	void localSearchDept(size_t size)
	{
		m_localSearchDepth = size;
	}

	void temperature(float minT, float maxT, float tStep)
	{
		m_minT = minT;
		m_maxT = maxT;
		m_tStep = tStep;
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
	const NonDominatedSet<KeyboardSize>& optimize(Itr begin, Itr end, size_t numGenerations)
	{
		// The algorithm is based on 
		// "Evolutionary multi - objective simulated annealing with adaptive and competitive search direction"
		// "A Simulated Annealing based Genetic Local Search Algorithm for Multi-objective Multicast Routing Problems"

		const auto minT = m_minT;
		const auto maxT = m_maxT;
		const auto tStep = m_tStep;
		auto probability = std::uniform_real_distribution<float>(0, 1.0);
		std::vector<float> weights(m_populationSize);

		for (auto&& i : weights)
		{
			i = 0.0f;
		}
		std::vector<Keyboard<KeyboardSize>> population(m_populationSize);
		using Solution = typename NonDominatedSet<KeyboardSize>::Solution;
		std::vector<std::vector<float>> populationSolutions(m_populationSize);
		std::vector<float> solution;
		solution.resize(end - begin);

		for (auto i = 0; i < m_populationSize; i++)
		{
			population[i].randomize(m_randomGenerator);
			populationSolutions[i].resize(end - begin);
			Keyboard<KeyboardSize> keyboard = population[i];
			evaluate(populationSolutions[i], keyboard, begin, end);
		}
		m_NonDominatedSet = NonDominatedSet<KeyboardSize>(population, populationSolutions);
		for (auto currentT = maxT; currentT > minT; currentT -= tStep)
		{
			for (size_t i = 0; i < m_populationSize; ++i)
			{
				for (size_t c = 0; c < m_localSearchDepth; ++c)
				{
					auto neighbour = mutate(population[i]);
					evaluate(solution, neighbour, begin, end);
					if (!isDominated(solution, populationSolutions[i]))
					{
						m_NonDominatedSet.insert(neighbour, solution);
					}
					if (probability(m_randomGenerator) < annealingProbability(population[i], neighbour, weights[i], currentT))
					{
						population[i] = neighbour;
						populationSolutions[i].swap(solution);
					}
				}
			}
		}
		return m_NonDominatedSet;
	}

protected:
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

	void localSearch(Keyboard<KeyboardSize>& keyboard)
	{

	}

	float annealingProbability(Keyboard<KeyboardSize>& first, Keyboard<KeyboardSize>& second, float weight, float t)
	{
		return 0.5;
	}

	std::mt19937 m_randomGenerator;
	NonDominatedSet<KeyboardSize> m_NonDominatedSet;
	size_t m_populationSize = 0;
	size_t m_localSearchDepth = 0;
	float m_maxT = 1.0f;
	float m_minT = 0.1f;
	float m_tStep = 0.01f;
};