#pragma once
#include "Keyboard.hpp"
#include "Helpers.hpp"
#include <algorithm>
#include <numeric>


namespace nondominatedset_detail
{

	template<typename S>
	float distance(const S& s)
	{
		auto maximum = *std::max_element(std::begin(s), std::end(s));
		auto minimum = *std::min_element(std::begin(s), std::end(s));
		return maximum - minimum;
	}

	template<typename SolutionArray>
	void selectPivoitPoint(SolutionArray& solutions)
	{
		// This is based on the paper:
		// BSkyTree: Scalable Skyline Computation Using A Balanced Pivot Selection

		if (solutions.size() >= 2)
		{
			auto head = std::begin(solutions);
			auto cur = head + 1;
			auto tail = std::end(solutions) - 1;
			float minDist = distance(*head);
			while (cur <= tail)
			{
				if (isDominated(*cur, *head))
				{
					if (cur != tail)
					{
						*cur = std::move(*tail);
					}
					solutions.erase(tail);
					tail--;
				}
				else if (isDominated(*head, *cur))
				{
					*head = std::move(*cur);
					if (cur != tail)
					{
						*cur = std::move(*tail);
					}
					solutions.erase(tail);
					tail--;
					cur = head + 1;
					minDist = distance(*head);
				}
				else
				{
					float curDist = distance(*cur);
					if (curDist < minDist)
					{
						bool dominated = false;
						for (auto i = head + 1; i != cur; i++)
						{
							if (isDominated(*cur, *i))
							{
								dominated = true;
								break;
							}
						}
						if (!dominated)
						{
							std::swap(*head, *cur);
							minDist = curDist;
						}
					}
					++cur;
				}
			}
		}
	}

	template<typename Point>
	unsigned int mapPointToRegion(Point& reference, Point& p)
	{
		unsigned int mask = 1;
		unsigned int ret = 0;
		for (size_t i = 0; i < reference.size(); i++)
		{
			if (p[i] <= reference[i])
			{
				ret |= mask;
			}
			mask <<= 1;
		}
		return ret;
	}
}

template<size_t KeyboardSize>
class NonDominatedSet
{
	friend class FitnessCalculator;
public:
	using KeyboardType = Keyboard<KeyboardSize>;
	struct Solution
	{
		KeyboardType m_keyboard;
		std::vector<float> m_solution;
		unsigned int m_pruningPower;
	};

	using SolutionsVector = std::vector<Solution>;

	NonDominatedSet()
	{
	}

	template<typename KeyboardArray, typename SolutionsArray>
	NonDominatedSet(const KeyboardArray& keyboards, const SolutionsArray& solutions)
	{
		size_t num_elements = keyboards.size();
		if (num_elements == 0)
		{
			return;
		}
		size_t numDimensions = solutions[0].size();
		m_idealPoint.assign(numDimensions, std::numeric_limits<float>::min());
		m_solutions.reserve(num_elements);
		auto s = solutions.begin();
		for (auto k = keyboards.begin(); k != keyboards.end(); ++k, ++s)
		{
			insert(*k, *s);
		}
	}

	NonDominatedSet(const NonDominatedSet& rhs)
		:
		m_keyboards(rhs.m_keyboards),
		m_solutions(rhs.m_solutions),
		m_indices(rhs.m_indices)
	{
		m_firstFree = m_firstFree + (rhs.m_firstFree - rhs.m_indices.begin());
	}

	NonDominatedSet(NonDominatedSet&& rhs)
		:
		m_keyboards(std::move(rhs.m_keyboards)),
		m_solutions(std::move(rhs.m_solutions)),
		m_indices(std::move(rhs.m_indices)),
		m_firstFree(std::move(rhs.m_firstFree))
	{
	}

	NonDominatedSet& operator=(const NonDominatedSet& rhs)
	{
		m_keyboards = rhs.m_keyboards;
		m_solutions = rhs.m_solutions;
		m_indices = rhs.m_indices;
		m_firstFree = m_firstFree + (rhs.m_firstFree - rhs.m_indices.begin());
		return *this;
	}

	NonDominatedSet& operator=(NonDominatedSet&& rhs)
	{
		m_solutions = std::move(rhs.m_solutions);
		return *this;
	}

	size_t size() const { return m_solutions.size(); }

	const std::vector<float> getIdealPoint() const
	{
		return m_idealPoint;
	}


	template<typename SolutionType>
	bool insert(const KeyboardType& keyboard, const SolutionType& solution)
	{
		if (m_idealPoint.empty())
		{
			m_idealPoint.assign(solution.size(), std::numeric_limits<float>::min());
		}
		bool dominated = false;
		auto sItr = m_solutions.begin();
		auto end = m_solutions.end();

		bool solutionAssigned = false;
		while (sItr != end)
		{
			if (sItr->m_keyboard == keyboard)
			{
				return false;
			}
			if (isDominated(sItr->m_solution, solution))
			{
				if (!solutionAssigned)
				{
					sItr->m_solution.assign(std::begin(solution), std::end(solution));
					sItr->m_keyboard = keyboard;
					// The pruning power stays the same
					solutionAssigned = true;
				}
				else
				{
					m_solutions.erase(std::remove_if(sItr, end, [&solution](auto& s) { return isDominated(s.m_solution, solution); }), end);
					break;
				}
			}
			else if (!solutionAssigned && isDominated(solution, sItr->m_solution))
			{
				sItr->m_pruningPower++;
				unsigned int pruningPower = sItr->m_pruningPower;
				while (sItr != m_solutions.begin())
				{
					auto prev = sItr - 1;
					if (prev->m_pruningPower < pruningPower)
					{
						std::swap(*sItr, *prev);
					}
					else
					{
						break;
					}
					sItr = prev;
				}
				dominated = true;
				break;
			}

			++sItr;
		}

		if (!dominated)
		{
			for (size_t i = 0; i < m_idealPoint.size(); i++)
			{
				m_idealPoint[i] = std::max(m_idealPoint[i], solution[i]);
			}
			if (!solutionAssigned)
			{
				m_solutions.emplace_back(Solution{ keyboard, {std::begin(solution), std::end(solution)}, 0});
			}
		}
		return !dominated;
	}


	const SolutionsVector& getResult() const
	{
		return m_solutions;
	}

private:
	SolutionsVector m_solutions;
	std::vector<float> m_idealPoint;
};