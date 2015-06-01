#pragma once
#include <functional>
#include "Keyboard.hpp"
#include <random>
#include <vector>

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
	template<typename Itr>
	Keyboard<KeyboardSize> optimize(Itr begin, Itr end, size_t numGenerations)
	{
		// The algorithm is based on 
		// "A Simulated Annealing based Genetic Local Search Algorithm for Multi-objective Multicast Routing Problems"

		const auto maxT = 1.0f;
		const auto minT = 0.1f;
		const auto populationSize = 50;
		const auto mutationProbability = 0.0f;
		const auto temperatureStep = 0.01f;
		auto parentSelector = std::uniform_int_distribution<>(0, populationSize - 1);
		auto probability = std::uniform_real_distribution<float>(0, 1.0);
		std::array<float, populationSize> weights;
		// Not part of the actual algorithm
		auto k = Keyboard<KeyboardSize>();
		auto best = std::make_tuple(k, begin->evaluate(k));

		for (auto&& i : weights)
		{
			i = 0.0f;
		}
		std::array<Keyboard<KeyboardSize>, populationSize> population;
		for (auto&& k : population)
		{
			k.randomize(m_randomGenerator);
		}
		for (size_t i = 0;i < numGenerations; i++)
		{
			for (auto currentT = maxT; currentT > minT; currentT-=temperatureStep)
			{
				auto parent1 = parentSelector(m_randomGenerator);
				auto parent2 = parentSelector(m_randomGenerator);
				while (parent2 == parent1)
				{
					parent2 = parentSelector(m_randomGenerator);
					parent2 = parentSelector(m_randomGenerator);
				}
				auto child = produceChild(population[parent1], population[parent2]);
				if (probability(m_randomGenerator) < mutationProbability)
				{
					mutate(child);
				}
				localSearch(child);
				auto parentToReplace = parentSelector(m_randomGenerator);
				if (probability(m_randomGenerator) < annealingProbability(population[parentToReplace], child, weights[parent1], currentT))
				{
					population[parentToReplace] = child;
				}

				//Not part of the actual algorithm
				float v = begin->evaluate(child);
				if (v > std::get<1>(best))
				{
					best = std::make_pair(k, v);
				}
			}
		}
		return std::get<0>(best);
	}
protected:

	Keyboard<KeyboardSize> produceChild(const Keyboard<KeyboardSize>& parent1, const Keyboard<KeyboardSize>& parent2)
	{
		//This did not work....
		auto dist = std::uniform_int_distribution<size_t>(0, parent1.m_keys.size()-1);
		auto p1 = dist(m_randomGenerator);
		auto p2 = dist(m_randomGenerator);
		if (p2 < p1)
		{
			std::swap(p1, p2);
		}
		return detail::partiallyMatchedCrossover(parent1, parent2, p1, p2);
	}

	void mutate(Keyboard<KeyboardSize>& keyboard)
	{
	}

	void localSearch(Keyboard<KeyboardSize>& keyboard)
	{

	}

	float annealingProbability(Keyboard<KeyboardSize>& first, Keyboard<KeyboardSize>& second, float weight, float t)
	{
		return 0.0;
	}

	std::mt19937 m_randomGenerator;
};