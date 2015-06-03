#pragma once

class NonDominatingSet
{
public:
	using SolutionsVector = std::vector<std::vector<float>>;
	using iterator = SolutionsVector::iterator;
	using const_iterator = SolutionsVector::const_iterator;
	using value_type = SolutionsVector::value_type;
	
	template<typename T>
	NonDominatingSet(T b, T e)
	{
		SolutionsVector p;
		p.reserve(e - b);
		m_solutions.reserve(e - b);
		for (auto i = b;i != e;++i)
		{
			p.emplace_back(std::vector<float>(std::begin(*i), std::end(*i)));
		}
		using V = std::vector<float>;
		std::sort(p.begin(), p.end(), [](const V& lhs, const V& rhs)
		{
			return lhs[0] < rhs[0];
		});
		auto r = front(std::make_pair(p.begin(), p.end()));
		m_solutions.assign(r.first, r.second);
	}

	size_t size() { return m_solutions.size(); }

	iterator begin() { return m_solutions.begin(); }
	iterator end() { return m_solutions.end(); }
	const_iterator begin() const { return m_solutions.begin(); }
	const_iterator end() const { return m_solutions.end(); }
private:
	std::pair<iterator, iterator> front(std::pair<iterator, iterator> input)
	{
		auto len = input.second - input.first;
		if (len == 1 || len == 0)
		{
			return input;
		}
		else
		{
			auto half = len / 2;
			auto t = front(std::make_pair(input.first, input.first + half));
			auto b = front(std::make_pair(input.first + half, input.second));
			for (auto i = b.first;i != b.second;++i)
			{
		

			}
			return t;
		}

	}
	SolutionsVector m_solutions;
};