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

	inline float weightedSum(const std::vector<float>& solution, const std::vector<float>&, const std::vector<float>& weights)
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
}

template<size_t KeyboardSize, size_t NumObjectives, size_t MaxLeafSize = std::numeric_limits<size_t>::max()>
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

	void initialTemperature(float maxT, float minT, size_t numSteps)
	{
		m_initialMinT = minT;
		m_initialMaxT = maxT;
		m_initialTSteps = numSteps;
	}

	void fastCoolingTemperature(float maxT, float minT, size_t numSteps)
	{
		m_fastCoolingMinT = minT;
		m_fastCoolingMaxT = maxT;
		m_fastCoolingTSteps = numSteps;
	}

	void paretoTemperature(float maxT, float minT)
	{
		m_paretoMaxT = maxT;
		m_paretoMinT = minT;
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
		m_NonDominatedSet = NonDominatedSet<KeyboardSize, NumObjectives, MaxLeafSize>(m_population, m_populationSolutions);
		
		int numEvaluationsLeft = static_cast<int>(numEvaluations);
		m_minT = m_initialMinT;
		m_maxT = m_initialMaxT;
		m_numTSteps = m_initialTSteps;
		for (size_t i = 0; i < m_populationSize; ++i)
		{
			Keyboard<KeyboardSize> newKeyboard;
			simulatedAnnealing(i, begin, end, newKeyboard, solution, detail::weightedSum, false);
			numEvaluationsLeft -= static_cast<int>(m_numTSteps);
			if (numEvaluationsLeft < 0)
				break;
		}

		m_minT = m_fastCoolingMinT;
		m_maxT = m_fastCoolingMaxT;
		m_numTSteps = m_fastCoolingTSteps;
		
		while(numEvaluationsLeft > 0)
		{
			auto selector = std::uniform_int<size_t>(0, m_NonDominatedSet.size() - 1);
			auto index = selector(m_randomGenerator);
			const auto& selectedSolution = m_NonDominatedSet[index];
			m_population[0] = selectedSolution.m_keyboard;
			m_populationSolutions[0].assign(std::begin(selectedSolution.m_solution), std::end(selectedSolution.m_solution));

			auto weightGenerator = std::uniform_real_distribution<float>(0.0, 1.0f);
			for (auto&& w : m_weights[0])
			{
				w = weightGenerator(m_randomGenerator);
			}
			auto sum = std::accumulate(m_weights[0].begin(), m_weights[0].end(), 0.0f);
			for (auto&& w : m_weights[0])
			{
				w = w / sum;
			}

			typedef std::vector<float> V;
			Keyboard<KeyboardSize> newKeyboard;
			simulatedAnnealing(0, begin, end, newKeyboard, solution, detail::evaluateChebycheff<V, V, V>, true);
			numEvaluationsLeft -= static_cast<int>(m_numTSteps);
		}
		return m_NonDominatedSet;
	}

protected:
	template<typename Itr, typename ScalarizeFunc>
	void simulatedAnnealing(size_t index, Itr begin, Itr end, Keyboard<KeyboardSize>& outKeyboard, std::vector<float>& outSolution, ScalarizeFunc& scalarize, bool paretoDominance)
	{
		auto probability = std::uniform_real_distribution<float>(0, 1.0);
		outKeyboard = m_population[index];
		outSolution = m_populationSolutions[index];
		float alpha = std::pow(m_minT / m_maxT, 1.0f / m_numTSteps);
		float paretoAlpha = std::pow(m_paretoMinT / m_paretoMaxT, 1.0f / m_numTSteps);
		float prevParetoDistance = 0.0f;
		for (float currentT = m_maxT, paretoCurrentT = m_paretoMinT; currentT >= m_minT; currentT *= alpha, paretoCurrentT *= paretoAlpha)
		{
			auto neighbour = mutate(outKeyboard);
			evaluate(m_tempSolution, neighbour, begin, end);
			float paretoDistance = 0.0f;
			bool dominating = false;
			if (!isDominated(m_tempSolution, outSolution))
			{
				m_NonDominatedSet.insert(neighbour, m_tempSolution);
				paretoDistance = m_NonDominatedSet.getLastParetoDistance();
				dominating = true;
			}
			if (annealingProbability(outSolution, m_tempSolution, m_weights[index], currentT, scalarize) > probability(m_randomGenerator))
			{
				bool paretoValid = true;
				if (paretoDominance && !dominating)
				{
					paretoValid = false;
					float p = std::exp(-((paretoDistance - prevParetoDistance) / paretoCurrentT));
					p = std::min(1.0f, p);
					if (p > probability(m_randomGenerator))
					{
						paretoValid = true;
					}
				}

				if (paretoValid)
				{
					outKeyboard = neighbour;
					outSolution.swap(m_tempSolution);
				}
			}
		}
	}

	template<typename ScalarizeFunc>
	float annealingProbability(const std::vector<float>& first, const std::vector<float>& second, std::vector<float>& weights, float t, ScalarizeFunc& scalarize)
	{ 
		float sFirst =  scalarize(first, m_NonDominatedSet.getIdealPoint(), weights);
		float sSecond = scalarize(second, m_NonDominatedSet.getIdealPoint(), weights);
		float p = std::exp(-((sFirst - sSecond) / t));
		p = std::min(1.0f, p);
		return p;
	}

	void updatePopulation(size_t index, const Keyboard<KeyboardSize>& keyboard, std::vector<float>& solution)
	{
		float solutionValue = detail::weightedSum(solution, m_NonDominatedSet.getIdealPoint(), m_weights[index]);
		float populationValue = detail::weightedSum(m_populationSolutions[index], m_NonDominatedSet.getIdealPoint(), m_weights[index]);
		if (solutionValue > populationValue)
		{
			m_population[index] = keyboard;
			m_populationSolutions[index] = solution;
		}
	}


	void selectWeightVectors(size_t numObjectives)
	{
		detail::generateWeightVectors(m_weights, m_populationSize, numObjectives, &m_randomGenerator);
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
	NonDominatedSet<KeyboardSize, NumObjectives, MaxLeafSize> m_NonDominatedSet;
	std::vector<Keyboard<KeyboardSize>> m_population;
	std::vector<std::vector<float>> m_populationSolutions;
	std::vector<std::vector<float>> m_weights;
	std::vector<float> m_tempSolution;
	size_t m_populationSize = 0;
	float m_initialMaxT = 1.0f;
	float m_initialMinT = 0.1f;
	size_t m_initialTSteps = 10;
	float m_fastCoolingMaxT = 1.0f;
	float m_fastCoolingMinT = 0.1f;
	size_t m_fastCoolingTSteps = 10;
	float m_paretoMaxT = 1.0f;
	float m_paretoMinT = 0.1f;


	float m_maxT;
	float m_minT;
	size_t m_numTSteps;
};

template<size_t KeyboardSize, size_t NumObjectives, size_t MaxLeafSize>
std::random_device Optimizer<KeyboardSize, NumObjectives, MaxLeafSize>::rd;