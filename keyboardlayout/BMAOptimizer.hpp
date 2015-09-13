#pragma once
#include <functional>
#include "Keyboard.hpp"
#include "NonDominatedSet.hpp"
#include <random>
#include <vector>
#include <utility>
#include <numeric>
#include "Optimizer.hpp"

template<size_t KeyboardSize>
class Objective;

// The algorithm is based on "Memetic search for the quadratic assignment problem" (Una Benlic and Jin-Kao Hao)

enum class CrossoverType
{
	Uniform,
	PartiallyMatched,
};

enum class PerturbType
{
	Disabled,
	Normal,
	Annealed,
};

template<size_t KeyboardSize, size_t NumObjectives, size_t MaxLeafSize = std::numeric_limits<size_t>::max()>
class BMAOptimizer
{
	static std::random_device rd;
public:
	typedef std::vector<std::pair<NonDominatedSet<KeyboardSize, NumObjectives, MaxLeafSize>, size_t>> SnapshotArray;


	BMAOptimizer(unsigned int seed = BMAOptimizer::rd())
	{
		m_randomGenerator.seed(seed);
	}

	void populationSize(size_t size)
	{
		m_populationSize = size;
	}

	void improvementDepth(size_t shortImprovement, size_t longImprovement)
	{
		m_shortImprovementDepth = shortImprovement;
		m_imporvementDepth = longImprovement;
	}

	void jumpMagnitude(float magnitude)
	{
		m_jumpMagnitude = magnitude;
	}

	void stagnation(size_t iterations, float minMagnitude, float maxMagnitude)
	{
		m_stagnationAfter = iterations;
		m_minStagnationMagnitude = minMagnitude;
		m_maxStagnationMagnitude = maxMagnitude;
	}

	void tabuTenure(float minTenure, float maxTenure)
	{
		m_minTabuTenureDist = static_cast<size_t>(minTenure * KeyboardSize);
		m_maxTabuTenureDist = static_cast<size_t>(maxTenure * KeyboardSize);
	}

	void minDirectedPertubation(float v)
	{
		m_minDirectedPerturbation = v;
	}

	void tournamentPool(size_t poolSize)
	{
		m_tournamentSize = poolSize;
	}

	void mutation(size_t frequency, float mutationStrengthMin, size_t mutationStrengthGrowth)
	{
		m_mutationFrequency = frequency;
		m_mutationStrenghtMin = mutationStrengthMin;
		m_mutationStrenghtGrowth = mutationStrengthGrowth;
	}

	void crossover(CrossoverType t)
	{
		m_crossoverType = t;
	}

	void perturbType(PerturbType perturb)
	{
		m_perturbType = perturb;
	}

	void snapshots(size_t snapshotEvery)
	{
		m_snapshotEvery = snapshotEvery;
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
		m_numEvaluationsLeft = static_cast<int>(numEvaluations);
		m_totalEvaluations = numEvaluations;
		generateRandomPopulation(begin, end);
		shortImprovement(begin, end);
		updateNonDominatedSet();
		size_t numWithoutImprovement = 0;
		size_t numCounter = 0;

		std::vector<float> solution(NumObjectives);
		while(m_numEvaluationsLeft > 0)
		{
			size_t num_of_parents = 2;
			auto parents = parentSelection();
			auto child = produceChild(m_population[parents.first], m_population[parents.second]);
			evaluate(solution, child, begin, end);
			localSearch(child, solution, m_imporvementDepth, begin, end);
			float resultingCost = m_NonDominatedSet[0].m_solution[0];
			float childCost = solution[0];
			if (childCost > resultingCost)
			{
				numWithoutImprovement = 0;
				numCounter = 0;
			}
			else
			{
				numWithoutImprovement++;
			}

			if (numWithoutImprovement == m_mutationFrequency)
			{
				size_t i = 0;
				do 
				{
					float t = static_cast<float>(numCounter) / m_mutationStrenghtGrowth;
					const float tMax = 1.0f - m_mutationStrenghtMin;
					t *= tMax;
					size_t mutationStrength = static_cast<size_t>(std::round(m_populationSize * (m_mutationStrenghtMin + t)));
					mutatePopulation(mutationStrength);
					evaluatePopulation(begin, end);
					updateNonDominatedSet();
					shortImprovement(begin, end);
					i++;
				} while (!populationIsUnique() && i <= 5);
				updateNonDominatedSet();
				numWithoutImprovement = 0;
				numCounter++;
			}
			if (numCounter > m_mutationStrenghtGrowth)
			{
				numCounter = 0;
			}
			replaceSolution(child, solution);
		}
		updateNonDominatedSet();
		return m_NonDominatedSet;
	}

	const SnapshotArray& getSnapshots()
	{
		return m_snapshots;
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
			m_numEvaluationsLeft--;
			evaluate(m_populationSolutions[i], m_population[i], begin, end);
		}
	}

	template<typename Itr>
	void evaluatePopulation(Itr begin, Itr end)
	{
		for (auto i = 0; i < m_populationSize; i++)
		{
			m_numEvaluationsLeft--;
			evaluate(m_populationSolutions[i], m_population[i], begin, end);
		}
	}
	
	template<typename Itr>
	void shortImprovement(Itr begin, Itr end)
	{
		for (size_t i = 0; i < m_populationSize; i++)
		{
			auto& keyboard = m_population[i];
			auto& solution = m_populationSolutions[i];
			localSearch(keyboard, solution, m_shortImprovementDepth, begin, end);
		}
	}

	template<typename Itr>
	void localSearch(Keyboard<KeyboardSize>& keyboard, std::vector<float>& solution, size_t numIterations, Itr begin, Itr end)
	{
		Keyboard<KeyboardSize> currentKeyboard = keyboard;

		DeltaArray delta;
		IndexArray lastSwapped;
		IndexArray frequency;

		size_t iterWithoutImprovement = 0;
		size_t iterLastImprovement = 0;

		bool hasImproved = true;
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
		Keyboard<KeyboardSize> prevLocalOptimum = keyboard;

		size_t perturbStr = std::max<size_t>(static_cast<size_t>(std::ceil(m_jumpMagnitude * KeyboardSize)), 2);
		std::uniform_real_distribution<float> stagnationDistribution(m_minStagnationMagnitude, m_maxStagnationMagnitude);

		for (size_t currentIteration = 1; currentIteration <= numIterations && m_numEvaluationsLeft > 0; currentIteration++)
		{
			size_t iRetained = 0;
			size_t jRetained = 0;

			float maxDelta = std::numeric_limits<float>::lowest();
			for (size_t i = 0; i < KeyboardSize; i++)
			{
				for (size_t j = i + 1; j < KeyboardSize; j++)
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
				for (size_t i = 0; i < KeyboardSize; i++)
				{
					for (size_t j = i + 1; j < KeyboardSize; j++)
					{
						delta[i][j] = computeDelta(currentKeyboard, currentCost, i, j, begin, end);
					}
				}
				//update_matrix_of_move_cost(i_retained, j_retained, n, delta, p, a, b);
				hasImproved = true;
			}
			else if(m_perturbType != PerturbType::Disabled)
			{
				if (iterWithoutImprovement > m_stagnationAfter)
				{
					iterWithoutImprovement = 0;
					auto str = std::max<size_t>(static_cast<size_t>(KeyboardSize * stagnationDistribution(m_randomGenerator)), 2);
					perturbStr = std::max(str, perturbStr);
				}
				else if (hasImproved == true && prevLocalOptimum != currentKeyboard) // Escaped from the previous local optimum. New local optimum reached
				{
					iterWithoutImprovement++;
					perturbStr = std::max<size_t>(static_cast<size_t>(std::ceil(m_jumpMagnitude * KeyboardSize)), 2);
				}
				else
				{
					perturbStr += 1;
				}

				if (hasImproved)
				{
					prevLocalOptimum = currentKeyboard;
				}
				if (m_perturbType == PerturbType::Annealed)
				{
					annealed_perturbe(currentKeyboard, delta, currentCost, lastSwapped, frequency, iterWithoutImprovement, solution[0], perturbStr, iteration, begin, end);
				}
				else if(m_perturbType == PerturbType::Normal)
				{
					perturbe(currentKeyboard, delta, currentCost, lastSwapped, frequency, iterWithoutImprovement, solution[0], perturbStr, iteration, begin, end);
				}
			
				if (currentCost > solution[0])
				{
					solution[0] = currentCost;
					keyboard = currentKeyboard;
				}
				hasImproved = false;
			}
			else
			{
				break;
			}
		};
	}
	
	template<typename Itr>
	float computeDelta(const Keyboard<KeyboardSize>& keyboard, float solution, size_t i, size_t j, Itr begin, Itr end)
	{
		Keyboard<KeyboardSize> k = keyboard;
		std::swap(k.m_keys[i], k.m_keys[j]);
		m_numEvaluationsLeft--;
		if (m_snapshotEvery != 0)
		{
			size_t evaluations = m_totalEvaluations - m_numEvaluationsLeft;
			if (m_snapshots.empty())
			{
				if (evaluations >= m_snapshotEvery)
				{
					updateNonDominatedSet();
					m_snapshots.emplace_back(std::make_pair(m_NonDominatedSet, evaluations));
				}
			}
			else if(evaluations - m_snapshots.back().second >= m_snapshotEvery)
			{
				updateNonDominatedSet();
				m_snapshots.emplace_back(std::make_pair(m_NonDominatedSet, evaluations));
			}
		}
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
	void perturbe(Keyboard<KeyboardSize>& currentKeyboard, DeltaArray& delta, float& currentCost, 
		IndexArray& lastSwapped, IndexArray& frequency, size_t iterWithoutImprovement, float bestBestCost, size_t perturbStr, size_t& iteration, Itr begin, Itr end)
	{
		std::uniform_real_distribution<float> dist(0.0f, std::nextafter(1.0f, 2.0f));
		std::uniform_int_distribution<size_t> tabuTenureDist(m_minTabuTenureDist, m_maxTabuTenureDist);
		std::uniform_int_distribution<int> keyDist(0, KeyboardSize - 1);
		const float d = static_cast<float>(iterWithoutImprovement) / m_stagnationAfter;
		for (size_t k = 0; k < perturbStr; k++)
		{
			size_t iRetained = std::numeric_limits<size_t>::max();
			size_t jRetained = iRetained;
			bool bit = false;
			float e = std::exp(-d);
			e = std::max(m_minDirectedPerturbation, e);

			if (e > dist(m_randomGenerator))
				bit = true;

			if (bit)
			{
				float maxDelta = std::numeric_limits<float>::lowest();
				for (size_t i = 0; i < KeyboardSize; i++)
				{
					for (size_t j = i + 1; j < KeyboardSize; j++)
					{
						if (delta[i][j] > maxDelta &&
							((lastSwapped[i][j] + tabuTenureDist(m_randomGenerator)) < iteration || (currentCost + delta[i][j]) > bestBestCost))
						{
							iRetained = i; 
							jRetained = j;
							maxDelta = delta[i][j];
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
				while (iRetained == jRetained)
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
				std::swap(currentKeyboard.m_keys[iRetained], currentKeyboard.m_keys[jRetained]);
				currentCost = currentCost + delta[iRetained][jRetained];
				for (size_t i = 0;i < KeyboardSize; i++)
				{
					for (size_t j = i + 1;j < KeyboardSize; j++)
					{
						delta[i][j] = computeDelta(currentKeyboard, currentCost, i, j, begin, end);
					}
				}
				if (currentCost > bestBestCost)
				{
					bestBestCost = currentCost;
					iteration++;
					break;
				}
				//update_matrix_of_move_cost(i_retained, j_retained, n, delta, p, a, b);
			}
			iteration++;
		}
	}

	template<typename Itr>
	void annealed_perturbe(Keyboard<KeyboardSize>& currentKeyboard, DeltaArray& delta, float& currentCost,
		IndexArray& lastSwapped, IndexArray& frequency, size_t iterWithoutImprovement, float bestBestCost, size_t perturbStr, size_t& iteration, Itr begin, Itr end)
	{
		std::uniform_int_distribution<size_t> tabuTenureDist(m_minTabuTenureDist, m_maxTabuTenureDist);
		auto probability = std::uniform_real_distribution<float>(0, 1.0);
		float min_t = 0.0001f;
		float max_t = 1.0f;
		float t_steps = 10;
		float alpha = std::pow(min_t / max_t, 1.0f / t_steps);
		for (float currentT = max_t; currentT >= min_t; currentT *= alpha)
		{
			size_t iRetained = std::numeric_limits<size_t>::max();
			size_t jRetained = iRetained;
			float maxDelta = std::numeric_limits<float>::lowest();
			float minDelta = std::numeric_limits<float>::max();
			for (size_t i = 0; i < KeyboardSize; i++)
			{
				for (size_t j = i + 1; j < KeyboardSize; j++)
				{
					if (currentCost + delta[i][j] > bestBestCost)
					{
						iRetained = i;
						jRetained = j;
						break;
					}
					if ((lastSwapped[i][j] + tabuTenureDist(m_randomGenerator)) < iteration)
					{
						if (delta[i][j] > maxDelta)
						{
							maxDelta = delta[i][j];
						}
						else if (delta[i][j] < minDelta)
						{
							minDelta = delta[i][j];
						}
					}
				}
				if (iRetained != std::numeric_limits<size_t>::max())
				{
					break;
				}
			}
			if (iRetained == std::numeric_limits<size_t>::max())
			{
				std::array<size_t, KeyboardSize> a;
				std::array<size_t, KeyboardSize> b;
				std::iota(a.begin(), a.end(), 0);
				std::iota(b.begin(), b.end(), 0);
				std::shuffle(a.begin(), a.end(), m_randomGenerator);
				std::shuffle(b.begin(), b.end(), m_randomGenerator);
				for (size_t i = 0; i < KeyboardSize; i++)
				{
					for (size_t j = 0; j < KeyboardSize; j++)
					{
						auto ai = a[i];
						auto bj = b[j];
						if (bj > ai)
						{
							float d = delta[ai][bj];

							if (((lastSwapped[ai][bj] + tabuTenureDist(m_randomGenerator)) < iteration))
							{
								float sFirst =  0.0f;
								float sSecond = (d - minDelta) / (maxDelta - minDelta);
								float p = std::exp(-((sFirst - sSecond) / currentT));
								p = std::min(1.0f, p);
								if (p > probability(m_randomGenerator))
								{
									iRetained = ai;
									jRetained = bj;
									break;
								}
							}
						}
					}
					if (iRetained != std::numeric_limits<size_t>::max())
					{
						break;
					}
				}
			}
			if (iRetained != std::numeric_limits<size_t>::max())
			{
				lastSwapped[iRetained][jRetained] = iteration;
				frequency[iRetained][jRetained] = frequency[iRetained][jRetained] + 1;
				std::swap(currentKeyboard.m_keys[iRetained], currentKeyboard.m_keys[jRetained]);
				currentCost = currentCost + delta[iRetained][jRetained];
				for (size_t i = 0;i < KeyboardSize; i++)
				{
					for (size_t j = i + 1;j < KeyboardSize; j++)
					{
						delta[i][j] = computeDelta(currentKeyboard, currentCost, i, j, begin, end);
					}
				}
				if (currentCost > bestBestCost)
				{
					bestBestCost = currentCost;
					iteration++;
					break;
				}
				//update_matrix_of_move_cost(i_retained, j_retained, n, delta, p, a, b);
			}
			iteration++;
		}
	}

	std::pair<size_t, size_t> parentSelection()
	{
		const size_t numParents = 2;
		const size_t tournamentSize = m_tournamentSize;
		const size_t maxTournamentSize = 32;
		size_t a = 0;

		std::array<size_t, numParents> parents;

		while (a < numParents)
		{
			bool insert = true;
			std::array<size_t, maxTournamentSize> tournamentPool;
			std::uniform_int_distribution<size_t> gen(0, m_populationSize - 1);
			std::generate_n(tournamentPool.begin(), tournamentSize, [this, &gen]() { return gen(m_randomGenerator); });

			float m = std::numeric_limits<float>::lowest();
			size_t winner;
			for (size_t i = 0; i < tournamentSize; i++)
			{
				if (m_populationSolutions[tournamentPool[i]][0] > m)
				{
					m = m_populationSolutions[tournamentPool[i]][0];
					winner = tournamentPool[i];
				}
			}
			for (size_t i = 0; i < a; i++)
			{
				if (parents[i] == winner)
				{
					insert = false;
				}
			}
			if (insert)
			{
				parents[a] = winner;
				a++;
			} 
		}
		return std::make_pair(parents[0], parents[1]);
	}

	Keyboard<KeyboardSize> produceChild(const Keyboard<KeyboardSize>& parent1, const Keyboard<KeyboardSize>& parent2)
	{
		if (m_crossoverType == CrossoverType::PartiallyMatched)
		{
			auto dist = std::uniform_int_distribution<size_t>(0, parent1.m_keys.size() - 1);
			auto p1 = dist(m_randomGenerator);
			auto p2 = dist(m_randomGenerator);
			if (p2 < p1)
			{
				std::swap(p1, p2);
			}
			return detail::partiallyMatchedCrossover(parent1, parent2, p1, p2);
		}
		else
		{
			return detail::uniformCrossover(parent1, parent2, m_randomGenerator);
		}
	}

	void replaceSolution(const Keyboard<KeyboardSize>& keyboard, std::vector<float>& solution)
	{
		if (std::find(m_population.begin(), m_population.end(), keyboard) != m_population.end())
		{
			return;
		}

		auto worst = std::min_element(m_populationSolutions.begin(), m_populationSolutions.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs[0] < rhs[0];
		});

		m_NonDominatedSet.insert(keyboard, solution);
		std::swap(*worst, solution);
		size_t index = worst - m_populationSolutions.begin();
		m_population[index] = keyboard;
	}

	bool populationIsUnique()
	{
		for (auto i = m_population.begin(); i != m_population.end(); ++i)
		{
			for (auto j = m_population.begin(); j != m_population.end(); ++j)
			{
				if (i != j)
				{
					if (*i == *j)
					{
						return false;
					}
				}
			}
		}
		return true;
	}

	void mutatePopulation(size_t mutationStrength)
	{
		std::array<bool, KeyboardSize> mutated;
		std::uniform_int_distribution<size_t> dist(0, KeyboardSize - 1);

		for (int i = 0; i < m_populationSize; i++)
		{
			mutated.fill(false);
			size_t r1 = dist(m_randomGenerator);
			mutated[r1] = true;
			for (int j = 0; j < mutationStrength; j++)
			{
				size_t r2 = dist(m_randomGenerator);
				while (r1 == r2 && mutated[r2])
					r2 = dist(m_randomGenerator);
				std::swap(m_population[i].m_keys[r1], m_population[i].m_keys[r2]);
				r1 = r2;
				mutated[r2] = true;
			}
		}
	}

	std::vector<Keyboard<KeyboardSize>> m_population;
	std::vector<std::vector<float>> m_populationSolutions;
	size_t m_populationSize = 0;
	float m_jumpMagnitude = 0.15f;
	size_t m_stagnationAfter = 250;
	float m_minStagnationMagnitude = 0.2f;
	float m_maxStagnationMagnitude = 0.4f;
	size_t m_minTabuTenureDist = static_cast<size_t>(0.9f * KeyboardSize);
	size_t m_maxTabuTenureDist = static_cast<size_t>(1.1f * KeyboardSize);
	float m_minDirectedPerturbation = 0.75f;
	size_t m_shortImprovementDepth = 5000;
	size_t m_imporvementDepth = 10000;
	size_t m_tournamentSize = 4;
	size_t m_mutationFrequency = 5;
	float m_mutationStrenghtMin = 0.5f;
	size_t m_mutationStrenghtGrowth = 5;
	size_t m_snapshotEvery = 0;
	CrossoverType m_crossoverType = CrossoverType::PartiallyMatched;
	PerturbType m_perturbType = PerturbType::Normal;
	std::mt19937 m_randomGenerator;
	NonDominatedSet<KeyboardSize, NumObjectives, MaxLeafSize> m_NonDominatedSet;
	SnapshotArray m_snapshots;
	int m_numEvaluationsLeft;
	size_t m_totalEvaluations;
};

template<size_t KeyboardSize, size_t NumObjectives, size_t MaxLeafSize>
std::random_device BMAOptimizer<KeyboardSize, NumObjectives, MaxLeafSize>::rd;