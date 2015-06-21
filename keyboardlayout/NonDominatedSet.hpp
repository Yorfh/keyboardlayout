#pragma once
#include "Keyboard.hpp"
#include <algorithm>

template<size_t KeyboardSize>
class NonDominatedSet
{
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
		size_t num_dimensions = solutions.size();
		m_solutions.reserve(num_elements);
		m_keyboards.reserve(num_elements);
		m_indices.reserve(num_elements);
		m_firstFree = m_indices.end();
		auto s = solutions.begin();
		for (auto k = keyboards.begin(); k != keyboards.end(); ++k, ++s)
		{
			insert(*k, std::begin(*s), std::end(*s));
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

	size_t size() const { return m_solutions.size(); }

	template<typename T>
	bool insert(T&& solution)
	{
		return insert(solution.first, std::begin(solution.second), std::end(solution.second));
	}

	template<typename T>
	bool insert(const KeyboardType& keyboard, T solutionBegin, T solutionEnd)
	{
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
		[&solutionBegin, &solutionEnd, &dominated, this](size_t index)
		{
			auto mBegin = m_solutions[index].begin();
			auto mEnd = m_solutions[index].end();
			if (dominated)
			{
				return true;
			}
			else if (isDominated(mBegin, mEnd, solutionBegin, solutionEnd))
			{
				return false;
			}
			else if (isDominated(solutionBegin, solutionEnd, mBegin, mEnd))
			{
				dominated = true;
			}
			return true;
		});
		m_firstFree = i;
		if (!dominated)
		{
			if (m_firstFree != m_indices.end())
			{
				unsigned int indexToInsert = *m_firstFree;
				auto firstFreeOffset = m_firstFree - m_indices.begin();
				auto i = std::lower_bound(m_indices.begin(), m_firstFree, *m_firstFree);
				m_indices.insert(i, indexToInsert);
				m_firstFree = m_indices.begin() + firstFreeOffset + 1;

				m_keyboards[indexToInsert] = keyboard;
				m_solutions[indexToInsert].assign(solutionBegin, solutionEnd);
			}
			else
			{
				m_keyboards.emplace_back(keyboard);
				m_solutions.emplace_back(solutionBegin, solutionEnd);
				m_indices.push_back(static_cast<unsigned int>(m_indices.size()));
				m_firstFree = m_indices.end();
			}
		}
		return !dominated;
	}

	template<typename T1, typename T2>
	static bool isDominated(T1 begin1, T1 end1, T2 begin2, T2 end2)
	{
		bool found = false;
		auto j = begin2;
		for (auto i = begin1; i != end1; ++i, ++j)
		{
			if (*i > *j)
			{
				return false;
			}
			else if (*i < *j)
			{
				found = true;
			}
		}
		return found;
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
};