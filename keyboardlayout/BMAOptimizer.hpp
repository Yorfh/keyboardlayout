#pragma once
#include <functional>
#include "Keyboard.hpp"
#include "NonDominatedSet.hpp"
#include <random>
#include <vector>
#include <utility>
#include <numeric>
#include <set>
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

template<size_t KeyboardSize>
class BMAOptimizer
{
	static std::random_device rd;
	static const bool EnableLog = false;
public:
	typedef std::vector<std::pair<std::tuple<float, Keyboard<KeyboardSize>>, size_t>> SnapshotArray;


	BMAOptimizer(unsigned int seed = BMAOptimizer::rd())
	{
		m_randomGenerator.seed(seed);
	}

	void populationSize(size_t size)
	{
		m_populationSize = size;
		m_populationSize = 10;
	}

	void improvementDepth(size_t longImprovement)
	{
		m_imporvementDepth = longImprovement;
		m_imporvementDepth = 10000;
	}

	void jumpMagnitude(float magnitude)
	{
		m_jumpMagnitude = magnitude;
		m_jumpMagnitude = 0.15f;
	}

	void stagnation(size_t iterations, float minMagnitude, float maxMagnitude)
	{
		m_stagnationAfter = iterations;
		m_stagnationAfter = 250;
		m_minStagnationMagnitude = minMagnitude;
		m_minStagnationMagnitude = 0.2f;
		m_maxStagnationMagnitude = maxMagnitude;
		m_maxStagnationMagnitude = 0.4f;
	}

	void tabuTenure(float minTenure, float maxTenure)
	{
		m_minTabuTenureDist = minTenure;
		m_minTabuTenureDist = 0.9f;
		m_maxTabuTenureDist = maxTenure;
		m_maxTabuTenureDist = 1.1f;
	}

	void minDirectedPertubation(float v)
	{
		m_minDirectedPerturbation = v;
		m_minDirectedPerturbation = 0.75f;
	}

	void tournamentPool(size_t poolSize)
	{
		m_tournamentSize = poolSize;
		m_tournamentSize = 4;
	}

	void mutation(size_t frequency, float mutationStrengthMin, size_t mutationStrengthGrowth)
	{
		m_mutationFrequency = frequency;
		m_mutationFrequency = 10;
		m_mutationStrenghtMin = mutationStrengthMin;
		m_mutationStrenghtMin = 0.5;
		m_mutationStrenghtGrowth = mutationStrengthGrowth;
		m_mutationStrenghtGrowth = 5;
	}

	void crossover(CrossoverType t)
	{
		m_crossoverType = t;
		m_crossoverType = CrossoverType::Uniform;
	}

	void perturbType(PerturbType perturb)
	{
		m_perturbType = perturb;
		m_perturbType = PerturbType::Normal;
	}

	void snapshots(size_t snapshotEvery)
	{
		m_snapshotEvery = snapshotEvery;
	}

	void annealing(float min_t)
	{
		m_minT = min_t;
	}

	void target(float target)
	{
		m_target = target;
	}

	template<typename Objective>
	float evaluate(Keyboard<KeyboardSize>& keyboard, Objective& objective)
	{
		return objective.evaluate(keyboard);
	}

	template<typename Objective>
	const std::tuple<float, Keyboard<KeyboardSize>>& optimize(Objective& objective, size_t numEvaluations)
	{
		m_numEvaluationsLeft = static_cast<int>(numEvaluations);
		m_totalEvaluations = numEvaluations;
		generateRandomPopulation(objective);
		shortImprovement(false, objective);
		updateBestSolution();
		updateEliteArchive();
		size_t numWithoutImprovement = 0;
		size_t numCounter = 0;

		while(m_numEvaluationsLeft > 0 && std::abs(std::get<0>(m_bestSolution) - m_target) > tolerance)
		{
			size_t num_of_parents = 2;
			auto parents = parentSelection();
			auto child = produceChild(m_population[parents.first], m_population[parents.second]);
			float childCost = evaluate(child, objective);
			m_numEvaluationsLeft--;
			localSearch(child, childCost, m_imporvementDepth, false, objective);
			float resultingCost = std::get<0>(m_bestSolution);
			if (EnableLog)
				std::cout << std::setprecision(9) << resultingCost << " " << m_numEvaluationsLeft << std::endl;
			if (childCost > resultingCost + tolerance)
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
				do 
				{
					size_t mutationStrength = static_cast<size_t>(KeyboardSize*(0.5 + static_cast<float>(numCounter) / 10));
					mutatePopulation(mutationStrength);
					evaluatePopulation(objective);
					updateBestSolution();
					shortImprovement(false, objective);
				} while (!populationIsUnique());
				updateBestSolution();
				updateEliteArchive();
				numWithoutImprovement = 0;
				numCounter++;
				if (EnableLog)
					std::cout << "Mutated" << std::endl;
			}
			if (numCounter > m_mutationStrenghtGrowth)
			{
				numCounter = 0;
			}
			replaceSolution(child, childCost);
			updateEliteArchive();
		}
		updateBestSolution();
		return m_bestSolution;
	}

	const SnapshotArray& getSnapshots()
	{
		return m_snapshots;
	}

	size_t getNumEvaluations() const
	{
		return m_totalEvaluations - m_numEvaluationsLeft;
	}

protected:
	typedef std::array<std::array<float, KeyboardSize>, KeyboardSize> DeltaArray;
	typedef std::array<std::array<size_t, KeyboardSize>, KeyboardSize> IndexArray;

	template<typename Objective>
	void generateRandomPopulation(Objective& objective)
	{
		m_population.resize(m_populationSize);
		m_populationSolutions.resize(m_populationSize);

		for (auto i = 0; i < m_populationSize; i++)
		{
			m_population[i].randomize(m_randomGenerator);
			m_numEvaluationsLeft--;
			m_populationSolutions[i] = evaluate(m_population[i], objective);
		}
	}

	template<typename Objective>
	void evaluatePopulation(Objective& objective)
	{
		for (auto i = 0; i < m_populationSize; i++)
		{
			m_numEvaluationsLeft--;
			m_populationSolutions[i] = evaluate(m_population[i], objective);
		}
	}
	
	template<typename Objective>
	void shortImprovement(bool steepestAscentOnly, Objective& objective)
	{
		for (size_t i = 0; i < m_populationSize; i++)
		{
			auto& keyboard = m_population[i];
			auto& solution = m_populationSolutions[i];
			localSearch(keyboard, solution, 5000, steepestAscentOnly, objective);
		}
	}

	template<typename Objective>
	bool tryToImproveAny(Objective& objective)
	{
		float best = std::get<0>(m_bestSolution);
		size_t i = 0;

		bool anyFound = true;
		size_t maxTimes = 1;
		while (anyFound)
		{
			anyFound = false;
			for (auto&& e : m_eliteSoFar)
			{
				if (e.m_improvedTimes < maxTimes)
				{
					anyFound = true;
					e.m_improvedTimes++;
					Keyboard<KeyboardSize> keyboard = e.m_keyboard;
					float solution = e.m_solution;
					localSearch(keyboard, solution, m_imporvementDepth, false, objective);
					if (solution > best + tolerance)
					{
						replaceSolution(keyboard, solution);
						return true;
					}
					i++;
					if (i >= m_populationSize)
					{
						return false;
					}
				}
			}
			maxTimes++;
		}

		return false;
	}

	template<typename Objective>
	void localSearch(Keyboard<KeyboardSize>& keyboard, float& solution, size_t numIterations, bool steepestAscentOnly, Objective& objective)
	{
		Keyboard<KeyboardSize> currentKeyboard = keyboard;

		DeltaArray delta;
		IndexArray lastSwapped;
		IndexArray frequency;

		size_t iterWithoutImprovement = 0;
		size_t iterLastImprovement = 0;

		bool hasImproved = true;
		size_t iteration = 0;
		float prevCost = std::numeric_limits<float>::lowest();

		for (size_t i = 0;i < KeyboardSize;i++)
		{
			lastSwapped[i].fill(0);
			frequency[i].fill(0);
		}

		computeAllDeltas(currentKeyboard, solution, objective, delta);

		float currentCost = solution;
		Keyboard<KeyboardSize> prevLocalOptimum = keyboard;

		size_t perturbStr = std::max<size_t>(static_cast<size_t>(std::ceil(m_jumpMagnitude * KeyboardSize)), 2);
		std::uniform_real_distribution<float> stagnationDistribution(m_minStagnationMagnitude, m_maxStagnationMagnitude);

		for (size_t currentIteration = 1; currentIteration <= numIterations && m_numEvaluationsLeft > 0 && solution != m_target; currentIteration++)
		{
			size_t iRetained = 0;
			size_t jRetained = 0;
			float maxDelta;

			std::tie(iRetained, jRetained, maxDelta) = steepestAscent(delta);

			if (maxDelta > 0.0f)
			{
				currentCost = swapKeys(iRetained, jRetained, currentKeyboard, currentCost, delta, iteration, lastSwapped, frequency, objective);
				if (currentCost > solution + tolerance)
				{
					iterWithoutImprovement = 0;
					iterLastImprovement = currentIteration;
					solution = currentCost;
					keyboard = currentKeyboard;
				}
				iteration++;
				hasImproved = true;
			}
			else
			{
				if (iterWithoutImprovement > m_stagnationAfter)
				{
					iterWithoutImprovement = 0;
					perturbStr = std::max<size_t>(static_cast<size_t>(KeyboardSize * stagnationDistribution(m_randomGenerator)), 2);
					//perturbStr = std::max(str, perturbStr);
				}
				else if ((hasImproved == true && prevCost != currentCost) || (hasImproved == false && prevCost != currentCost)) // Escaped from the previous local optimum. New local optimum reached
				{
					iterWithoutImprovement++;
					perturbStr = std::max<size_t>(static_cast<size_t>(std::ceil(m_jumpMagnitude * KeyboardSize)), 2);
				}
				else if(prevCost == currentCost)
				{
					perturbStr += 1;
				}

				if (hasImproved)
				{
					prevLocalOptimum = currentKeyboard;
				}
				prevCost = currentCost;
				perturbe(currentKeyboard, delta, currentCost, lastSwapped, frequency, iterWithoutImprovement, solution, perturbStr, iteration, objective);

				if (currentCost > solution + tolerance)
				{
					solution = currentCost;
					keyboard = currentKeyboard;
				}
				hasImproved = true;
			}
		};
	}

	template<typename Objective>
	void computeAllDeltas(const Keyboard<KeyboardSize> keyboard, float solution, Objective& objective, DeltaArray& delta, size_t from = Objective::NoSwap, size_t to = Objective::NoSwap)
	{
		objective.evaluateNeighbourhood(keyboard, solution, from, to, delta);
		m_numEvaluationsLeft-= KeyboardSize * (KeyboardSize - 1) / 2;
		if (m_snapshotEvery != 0)
		{
			size_t evaluations = m_totalEvaluations - m_numEvaluationsLeft;
			if (m_snapshots.empty())
			{
				if (evaluations >= m_snapshotEvery)
				{
					updateBestSolution();
					m_snapshots.emplace_back(std::make_pair(m_bestSolution, evaluations));
				}
			}
			else if (evaluations - m_snapshots.back().second >= m_snapshotEvery)
			{
				updateBestSolution();
				m_snapshots.emplace_back(std::make_pair(m_bestSolution, evaluations));
			}
		}
	}

	void updateBestSolution()
	{
		for (size_t i = 0; i < m_populationSize; i++)
		{
			if (m_populationSolutions[i] > std::get<0>(m_bestSolution))
			{
				m_bestSolution = std::make_tuple(m_populationSolutions[i], m_population[i]);
			}
		}
	}

	template<typename Objective>
	void perturbe(Keyboard<KeyboardSize>& currentKeyboard, DeltaArray& delta, float& currentCost,
		IndexArray& lastSwapped, IndexArray& frequency, size_t iterWithoutImprovement, float bestBestCost, size_t perturbStr, size_t& iteration, Objective& objective)
	{
		std::uniform_real_distribution<float> dist(0.0f, std::nextafter(1.0f, 2.0f));
		std::uniform_real_distribution<float> tenureDist(m_minTabuTenureDist, m_maxTabuTenureDist);
		std::uniform_int_distribution<int> keyDist(0, KeyboardSize - 1);
		const float d = static_cast<float>(iterWithoutImprovement) / (m_stagnationAfter - 1);
		bool useTabu = false;
		float startCost = currentCost;
		float e = std::exp(-d);
		e = std::max(m_minDirectedPerturbation, e);

		if (e > dist(m_randomGenerator))
			useTabu = true;
		for (size_t k = 0; k < perturbStr; k++)
		{

			size_t iRetained;
			size_t jRetained;
			if (useTabu)
			{
				std::tie(iRetained, jRetained) = tabuPerturbe(delta, lastSwapped, tenureDist, iteration, startCost, currentCost, bestBestCost);
			}
			else
			{
				std::tie(iRetained, jRetained) = randomPerturbe(delta, keyDist, currentCost, startCost);
			}

			if (iRetained != std::numeric_limits<size_t>::max())
			{
				currentCost = swapKeys(iRetained, jRetained, currentKeyboard, currentCost, delta, iteration, lastSwapped, frequency, objective);
				if (currentCost > bestBestCost + tolerance)
				{
					bestBestCost = currentCost;
				}
			}
			iteration++;
		}
	}

	std::tuple<size_t, size_t, float> steepestAscent(const DeltaArray delta)
	{
		size_t iRetained = std::numeric_limits<size_t>::max();
		size_t jRetained = iRetained;
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
		return std::make_tuple(iRetained, jRetained, maxDelta);
	}

	std::tuple<size_t, size_t> tabuPerturbe(const DeltaArray& delta, const IndexArray& lastSwapped, std::uniform_real_distribution<float>& tabuTenureDist, size_t iteration, float startCost, float currentCost, float bestBestCost)
	{
		size_t iRetained = std::numeric_limits<size_t>::max();
		size_t jRetained = iRetained;
		float maxDelta = std::numeric_limits<float>::lowest();
		for (size_t i = 0; i < KeyboardSize; i++)
		{
			for (size_t j = i + 1; j < KeyboardSize; j++)
			{
				float d = delta[i][j];
				if (d > maxDelta && std::abs(currentCost + delta[i][j] - startCost) >= tolerance)
				{
					if ((lastSwapped[i][j] + tabuTenureDist(m_randomGenerator) * KeyboardSize) < iteration || (currentCost + delta[i][j]) > bestBestCost + tolerance)
					{
						iRetained = i;
						jRetained = j;
						maxDelta = delta[i][j];
					}
				}
			}
		}
		return std::make_tuple(iRetained, jRetained);
	}

	std::tuple<size_t, size_t> randomPerturbe(const DeltaArray& delta, std::uniform_int_distribution<int>& keyDist, float currentCost, float startCost)
	{
		size_t iRetained = keyDist(m_randomGenerator);
		size_t jRetained = keyDist(m_randomGenerator);
		if (iRetained > jRetained)
			std::swap(iRetained, jRetained);
		while (iRetained == jRetained || std::abs(currentCost + delta[iRetained][jRetained] - startCost) < tolerance)
		{
			jRetained = keyDist(m_randomGenerator);
			if (iRetained > jRetained)
				std::swap(iRetained, jRetained);
		}
		return std::make_tuple(iRetained, jRetained);
	}

	template<typename Objective>
	float swapKeys(size_t from, size_t to, Keyboard<KeyboardSize> &currentKeyboard, float currentCost, DeltaArray& delta, size_t iteration, IndexArray& lastSwapped, IndexArray& frequency, Objective& objective)
	{
		lastSwapped[from][to] = iteration;
		frequency[from][to]++;
		std::swap(currentKeyboard.m_keys[from], currentKeyboard.m_keys[to]);
		float newCost = currentCost + delta[from][to];
		computeAllDeltas(currentKeyboard, newCost, objective, delta, from, to);
		return newCost;
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
				if (m_populationSolutions[tournamentPool[i]]> m)
				{
					m = m_populationSolutions[tournamentPool[i]];
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

	void replaceSolution(const Keyboard<KeyboardSize>& keyboard, float solution)
	{
		if (std::find(m_population.begin(), m_population.end(), keyboard) != m_population.end())
		{
			return;
		}

		auto worst = std::min_element(m_populationSolutions.begin(), m_populationSolutions.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs < rhs;
		});

		if (solution > std::get<0>(m_bestSolution))
		{
			m_bestSolution = std::make_tuple(solution, keyboard);
		}
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

	void updateEliteArchive()
	{
		for (size_t i = 0; i < m_population.size(); i++)
		{
			auto& keyboard = m_population[i];
			float solution = m_populationSolutions[i];
			updateEliteArchive(keyboard, solution);
		}
	}

	void updateEliteArchive(const Keyboard<KeyboardSize>& keyboard, float solution)
	{
		Elite e(keyboard, solution);
		if (m_eliteSoFar.find(e) == m_eliteSoFar.end())
		{
			m_eliteSoFar.insert(e);
		}
	}

	std::vector<Keyboard<KeyboardSize>> m_population;
	std::vector<float> m_populationSolutions;
	size_t m_populationSize = 0;
	float m_jumpMagnitude = 0.15f;
	size_t m_stagnationAfter = 250;
	float m_minStagnationMagnitude = 0.2f;
	float m_maxStagnationMagnitude = 0.4f;
	float m_minTabuTenureDist = 0.9f * KeyboardSize;
	float m_maxTabuTenureDist = 1.1f * KeyboardSize;
	float m_minDirectedPerturbation = 0.75f;
	size_t m_imporvementDepth = 10000;
	size_t m_tournamentSize = 4;
	size_t m_mutationFrequency = 5;
	float m_mutationStrenghtMin = 0.5f;
	size_t m_mutationStrenghtGrowth = 5;
	size_t m_snapshotEvery = 0;
	float m_minT = 0.1f;
	float m_target = std::numeric_limits<float>::lowest();
	CrossoverType m_crossoverType = CrossoverType::PartiallyMatched;
	PerturbType m_perturbType = PerturbType::Normal;
	std::mt19937 m_randomGenerator;
	std::tuple<float, Keyboard<KeyboardSize>> m_bestSolution = std::make_tuple(std::numeric_limits<float>::lowest(), Keyboard<KeyboardSize>());
	SnapshotArray m_snapshots;
	int m_numEvaluationsLeft;
	size_t m_totalEvaluations;

	static const float tolerance;
	
	struct Elite
	{
		Elite(const Keyboard<KeyboardSize>& keyboard, float solution) :
			m_keyboard(keyboard),
			m_solution(solution),
			m_improvedTimes(0)
		{
		}
		Keyboard<KeyboardSize> m_keyboard;
		float m_solution;
		mutable size_t m_improvedTimes;
	};

	struct CompareElite
	{
		bool operator()(const Elite& lhs, const Elite& rhs) const
		{
			if (lhs.m_solution == rhs.m_solution)
			{
				return lhs.m_keyboard.m_keys < rhs.m_keyboard.m_keys;
			}
			else
			{
				return lhs.m_solution < rhs.m_solution;
			}
		}
	};

	std::set<Elite, CompareElite> m_eliteSoFar;
};

template<size_t KeyboardSize>
std::random_device BMAOptimizer<KeyboardSize>::rd;

template<size_t KeyboardSize>
const float BMAOptimizer<KeyboardSize>::tolerance = 0.0000001f;