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
			insert(Solution(std::begin(*i), std::end(*i)));
		}
	}

	size_t size() { return m_solutions.size(); }

	bool insert(Solution&& solution)
	{
		bool dominated = false;
		auto i = std::remove_if(m_solutions.begin(), m_solutions.end(), 
		[&solution, &dominated](Solution& rhs)
		{
			if (dominated)
			{
				return false;
			}
			else if (isDominated(rhs, solution))
			{
				return true;
			}
			else if (isDominated(solution, rhs))
			{
				dominated = true;
			}
			return false;
		});
		m_solutions.erase(i, m_solutions.end());
		if (!dominated)
		{
			m_solutions.emplace_back(std::move(solution));
		}
		return !dominated;
	}

	static bool isDominated(const Solution& lhs, const Solution& rhs)
	{
		auto numDimensions = lhs.size();
		auto counter = 0;
		for (auto i = 0; i < numDimensions; i++)
		{
			if (lhs[i] > rhs[i])
			{
				return false;
			}
			else if (lhs[i] == rhs[i])
			{
				counter++;
			}
		}
		return counter!=numDimensions;
	}

	iterator begin() { return m_solutions.begin(); }
	iterator end() { return m_solutions.end(); }
	const_iterator begin() const { return m_solutions.begin(); }
	const_iterator end() const { return m_solutions.end(); }
private:
	SolutionsVector m_solutions;
};