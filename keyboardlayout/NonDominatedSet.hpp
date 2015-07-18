#pragma once
#include "Keyboard.hpp"
#include "Helpers.hpp"
#include <algorithm>

template<size_t KeyboardSize>
class NonDominatedSet
{
	friend class FitnessCalculator;
public:
	using KeyboardType = Keyboard<KeyboardSize>;
	using Solution = std::pair<KeyboardType, std::vector<float>>;
	using SolutionsVector = std::vector<Solution>;

	NonDominatedSet()
	{
		m_firstFree = m_indices.end();
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
		m_keyboards.reserve(num_elements);
		m_indices.reserve(num_elements);
		m_firstFree = m_indices.end();
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
		m_keyboards = std::move(rhs.m_keyboards);
		m_solutions = std::move(rhs.m_solutions);
		m_indices = std::move(rhs.m_indices);
		m_firstFree = std::move(rhs.m_firstFree);
		return *this;
	}

	size_t size() const { return m_firstFree - m_indices.begin(); }

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
		if (std::find_if(m_indices.begin(), m_firstFree, 
		[this, &keyboard](size_t index)
		{
			return m_keyboards[index] == keyboard;
		}) != m_firstFree)
		{
			return false;
		}
		bool dominated = false;
		auto i = std::stable_partition(m_indices.begin(), m_firstFree, 
		[&solution, &dominated, this](size_t index)
		{
			if (dominated)
			{
				return true;
			}
			else if (isDominated(m_solutions[index], solution))
			{
				return false;
			}
			else if (isDominated(solution, m_solutions[index]))
			{
				dominated = true;
			}
			return true;
		});
		m_firstFree = i;
		if (!dominated)
		{

			for (size_t i = 0; i < m_idealPoint.size(); i++)
			{
				m_idealPoint[i] = std::max(m_idealPoint[i], solution[i]);
			}

			if (m_firstFree != m_indices.end())
			{
				unsigned int indexToInsert = *m_firstFree;
				auto firstFreeOffset = m_firstFree - m_indices.begin();
				m_firstFree = m_indices.erase(m_firstFree);
				auto i = std::lower_bound(m_indices.begin(), m_firstFree, indexToInsert);
				m_indices.insert(i, indexToInsert);
				m_firstFree = m_indices.begin() + firstFreeOffset + 1;

				m_keyboards[indexToInsert] = keyboard;
				m_solutions[indexToInsert].assign(std::begin(solution), std::end(solution));
			}
			else
			{
				m_keyboards.emplace_back(keyboard);
				m_solutions.emplace_back(std::begin(solution), std::end(solution));
				m_indices.push_back(static_cast<unsigned int>(m_indices.size()));
				m_firstFree = m_indices.end();
			}
		}
		return !dominated;
	}


	SolutionsVector getResult() const
	{
		SolutionsVector res;
		res.reserve(m_firstFree - m_indices.begin());
		for (auto i = m_indices.begin(); i != m_firstFree;++i)
		{

			res.emplace_back( Solution{ m_keyboards[*i], m_solutions[*i]});
		}
		return res;
	}

private:
	std::vector<KeyboardType> m_keyboards;
	std::vector<std::vector<float>> m_solutions;
	using IndexVector = std::vector<unsigned int>;
	IndexVector m_indices;
	IndexVector::iterator m_firstFree;
	std::vector<float> m_idealPoint;
};