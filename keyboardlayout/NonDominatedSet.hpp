#pragma once

class NonDominatedSet
{
public:
	using Solution = std::vector<float>;
	using SolutionsVector = std::vector<Solution>;
	using iterator = SolutionsVector::iterator;
	using const_iterator = SolutionsVector::const_iterator;
	using value_type = SolutionsVector::value_type;

	template<typename T>
	NonDominatedSet(T b, T e)
	{
		size_t num_elements = e - b;
		if (num_elements == 0)
		{
			return;
		}
		size_t num_dimensions = b->size();
		m_solutions.reserve(e - b);
		for (auto i = b;i != e;++i)
		{
			insert(std::begin(*i), std::end(*i));
		}
	}

	size_t size() { return m_solutions.size(); }

	template<typename T>
	bool insert(T&& solution)
	{
		return insert(std::begin(solution), std::end(solution));
	}

	template<typename T>
	bool insert(T solutionBegin, T solutionEnd)
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
			m_solutions.emplace_back(solutionBegin, solutionEnd);
		}
		return !dominated;
	}

	template<typename Itr, typename Cont>
	static bool isDominated(Itr begin, Itr end, Cont&& container)
	{
		return isDominated(begin, end, std::begin(container), std::end(container));
	}

	template<typename Cont, typename Itr>
	static bool isDominated(Cont&& container, Itr begin, Itr end)
	{
		return isDominated(std::begin(container), std::end(container), begin, end);
	}

	template<typename Cont1, typename Cont2>
	static bool isDominated(Cont1&& cont1, Cont2&& cont2)
	{
		return isDominated(std::begin(cont1), std::end(cont1), std::begin(cont2), std::end(cont2));
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

	iterator begin() { return m_solutions.begin(); }
	iterator end() { return m_solutions.end(); }
	const_iterator begin() const { return m_solutions.begin(); }
	const_iterator end() const { return m_solutions.end(); }
private:
	SolutionsVector m_solutions;
};