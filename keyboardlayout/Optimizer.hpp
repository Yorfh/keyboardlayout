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

	template<typename T>
	void generateWeightVectorsHelper(T& output, size_t k, size_t maxDimension, size_t currentK, size_t currentDimension, size_t sum, std::vector<size_t>& current)
	{
		if (sum > k)
		{
			return;
		}
		if (currentDimension == maxDimension)
		{
			if (sum == k)
			{
				const float stepSize = 1.0f / (k - 1);
				do
				{
					output.emplace_back(maxDimension);
					std::transform(current.begin(), current.end(), output.back().begin(), [stepSize](size_t element)
					{
						return element * stepSize;
					});
				} while (std::next_permutation(current.begin(), current.end()));
			}
		}
		else
		{
			for (size_t i = currentK; i < k; i++)
			{
				current[currentDimension] = i;
				generateWeightVectorsHelper(output, k, maxDimension, i, currentDimension + 1, sum + i, current);
			}
		}
	}

	template<typename T>
	inline void generateWeightVectors(T& output, size_t populationSize, size_t numObjectives, std::mt19937* randomGenerator = nullptr)
	{
		output.reserve(populationSize);
		if (populationSize == 1)
		{
			output.emplace_back(numObjectives, 1.0f / numObjectives);
		}
		else if (numObjectives == 1)
		{
			std::fill_n(std::back_inserter(output), populationSize, std::vector<float>(1, 1.0f));
		}
		else
		{
			const float stepSize = 1.0f / (populationSize - 1);
			size_t h = 1;
			while (static_cast<size_t>(boost::math::binomial_coefficient<double>(static_cast<unsigned int>(h + numObjectives - 1), static_cast<unsigned int>(numObjectives - 1))) < populationSize)
			{
				h++;
			}
			std::vector<size_t> current;
			current.resize(numObjectives);
			generateWeightVectorsHelper(output, h + 1, numObjectives, 0, 0, 1, current);
			if (output.size() > populationSize)
			{
				std::unique_ptr<std::mt19937> g;
				if (!randomGenerator)
				{
					std::random_device rd;
					g = std::make_unique<std::mt19937>(rd());
					randomGenerator = g.get();
				}
				std::shuffle(output.begin(), output.end(), *randomGenerator);
				output.erase(output.begin() + populationSize, output.end());
			}
		}
	}

	template<typename T1, typename T2, typename T3>
	void solutionToChebycheff(const T1& reference, const T2& solution, T3& output)
	{
		// This formula is based on a eq(5) and eq(6) in the paper "MOEA/D with adaptive weight adjustment"
		size_t numObjectives = reference.size();
		float denominator = 0.0f;
		float epsilon = 0.0f;
		for (size_t i = 0; i < numObjectives; i++)
		{
			if (reference[i] - solution[i] == 0)
			{
				epsilon = 0.00000000000000000001f;
				break;
			}
		}

		for (size_t i = 0; i < numObjectives; i++)
		{
			denominator += 1.0f / (reference[i] - solution[i] + epsilon );
		}
		for (size_t i = 0; i < numObjectives; i++)
		{
			float numerator = 1.0f / (reference[i] - solution[i] + epsilon);
			output[i] = numerator / denominator;
		}
	}

	template<typename T1, typename T2, typename T3>
	float evaluateChebycheff(const T2& solution, const T1& reference, const T3& weights)
	{
		size_t numObjectives = reference.size();
		float maxElement = std::numeric_limits<float>::min();
		for (size_t i = 0; i < numObjectives; i++)
		{
			float v = weights[i] * std::abs(solution[i] - reference[i]);
			maxElement = std::max(maxElement, v);
		}
		return -maxElement;
	}

	template<typename T1, typename T2, typename T3>
	void generateWeightVectorsFromFront(const T1& front, const T2& reference, T3& output)
	{
		std::vector<std::pair<float, unsigned int>> distances;
		distances.reserve(front.size());
		const size_t numDimensions = reference.size();
		for (unsigned int i = 0; i < front.size() - 1; i++)
		{
			float dist = 0.0f;
			for (size_t j = 0; j < numDimensions; j++)
			{
				float temp = front[i][j] - front[i + 1][j];
				temp *= temp;
				dist += temp;
			}
			distances.push_back(std::make_pair(dist, i));
		}
		std::sort(distances.begin(), distances.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs.first > rhs.first;
		});

		size_t outputSize = output.size();
		std::vector<float> newPoint(numDimensions);
		size_t numPointsPerPair = output.size() / distances.size();
		if (numPointsPerPair == 0)
		{
			numPointsPerPair = 1;
		}
		size_t currentOutput = 0;
		for (size_t i = 0; i < distances.size(); i++)
		{
			auto& a = front[distances[i].second];
			auto& b = front[distances[i].second + 1];

			float multiplier = 1.0f / (numPointsPerPair + 1.0f);

			size_t numPoints = std::min(numPointsPerPair, outputSize - currentOutput);

			for (size_t j = 1; j <= numPoints; j++)
			{
				for (size_t k = 0; k < numDimensions; k++)
				{
					newPoint[k] = a[k] +  j * multiplier * (b[k] - a[k]);
				}
				solutionToChebycheff(reference, newPoint, output[currentOutput++]);
			}

		}
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
		detail::generateWeightVectors(m_weights, m_populationSize, numObjectives, &m_randomGenerator);
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
	size_t m_numIterations = 0;
	float m_maxT = 1.0f;
	float m_minT = 0.1f;
	size_t m_numTSteps = 10;
};

template<size_t KeyboardSize>
std::random_device Optimizer<KeyboardSize>::rd;