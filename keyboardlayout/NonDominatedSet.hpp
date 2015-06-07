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
	using iterator = typename SolutionsVector::iterator;
	using const_iterator = typename SolutionsVector::const_iterator;
	using value_type = typename SolutionsVector::value_type;

	NonDominatedSet()
	{

	}

	template<typename T>
	NonDominatedSet( T b, T e)
	{
		size_t num_elements = e - b;
		if (num_elements == 0)
		{
			return;
		}
		size_t num_dimensions = b->second.size();
		m_solutions.reserve(e - b);
		for (auto i = b;i != e;++i)
		{
			insert(i->first, std::begin(i->second), std::end(i->second));
		}
	}

	size_t size() { return m_solutions.size(); }

	template<typename T>
	bool insert(T&& solution)
	{
		return insert(solution.first, std::begin(solution.second), std::end(solution.second));
	}

	template<typename T>
	bool insert(const KeyboardType& keyboard, T solutionBegin, T solutionEnd)
	{
		bool dominated = false;
		auto i = std::remove_if(m_solutions.begin(), m_solutions.end(), 
		[&solutionBegin, &solutionEnd, &dominated](Solution& rhs)
		{
			if (dominated)
			{
				return false;
			}
			else if (isDominated(rhs, solutionBegin, solutionEnd))
			{
				return true;
			}
			else if (isDominated(solutionBegin, solutionEnd, rhs))
			{
				dominated = true;
			}
			return false;
		});
		m_solutions.erase(i, m_solutions.end());
		if (!dominated)
		{
			std::vector<float> temp(solutionBegin, solutionEnd);
			m_solutions.emplace_back(std::make_pair(keyboard, std::move(temp)));
		}
		return !dominated;
	}

	template<typename Itr, typename Cont>
	static bool isDominated(Itr begin, Itr end, Cont&& container)
	{
		return isDominated(begin, end, std::begin(container.second), std::end(container.second));
	}

	template<typename Cont, typename Itr>
	static bool isDominated(Cont&& container, Itr begin, Itr end)
	{
		return isDominated(std::begin(container.second), std::end(container.second), begin, end);
	}

	template<typename Cont1, typename Cont2>
	static bool isDominated(Cont1&& cont1, Cont2&& cont2)
	{
		return isDominated(std::begin(cont1.second), std::end(cont1.second), std::begin(cont2.second), std::end(cont2.second));
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

	void removeDuplicates()
	{
		std::sort(m_solutions.begin(), m_solutions.end(),
		[](const Solution& lhs, const Solution& rhs)
		{
			return std::lexicographical_compare(lhs.second.begin(), lhs.second.end(), rhs.second.begin(), rhs.second.end());
		});

		auto e=std::unique(m_solutions.begin(), m_solutions.end(),
		[](const Solution& lhs, const Solution& rhs)
		{
			return std::equal(lhs.second.begin(), lhs.second.end(), rhs.second.begin(), rhs.second.end());
		});
		m_solutions.erase(e, m_solutions.end());
	}

	iterator begin() { return m_solutions.begin(); }
	iterator end() { return m_solutions.end(); }
	const_iterator begin() const { return m_solutions.begin(); }
	const_iterator end() const { return m_solutions.end(); }
private:
	SolutionsVector m_solutions;
};