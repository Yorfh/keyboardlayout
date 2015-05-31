#pragma once
#include <functional>
#include "Keyboard.hpp"
#include <random>
#include <vector>
class Objective;

class Optimizer
{
public:
	template<typename Itr>
	Keyboard optimize(Itr begin, Itr end, size_t numGenerations)
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
		auto k = Keyboard();
		auto best = std::make_tuple(k, begin->evaluate(k));

		for (auto&& i : weights)
		{
			i = 0.0f;
		}
		std::array<Keyboard, populationSize> population;
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
				k.randomize(m_randomGenerator);
				float v = begin->evaluate(k);
				if (v > std::get<1>(best))
				{
					best = std::make_pair(k, v);
				}
			}
		}
		return std::get<0>(best);
	}
private:
	Keyboard produceChild(const Keyboard& parent1, const Keyboard& parent2)
	{
		return parent1;
	}

	void mutate(Keyboard& keyboard)
	{
	}

	void localSearch(Keyboard& keyboard)
	{

	}

	float annealingProbability(Keyboard& first, Keyboard& second, float weight, float t)
	{
		return 0.0;
	}

	std::mt19937 m_randomGenerator;
};