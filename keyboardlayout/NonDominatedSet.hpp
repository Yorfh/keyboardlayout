#pragma once

class NonDominatedSet
{
public:
	using SolutionsVector = std::vector<std::vector<float>>;
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
		if (num_dimensions == 1)
		{
			auto m = std::max_element(b, e, [](const T::value_type& lhs, const T::value_type& rhs)
			{
				return lhs[0] < rhs[0];
			});
			m_solutions.emplace_back(std::vector<float>(std::begin(*m), std::end(*m)));
		}
		else
		{
			m_solutions.reserve(e - b);
			for (auto i = b;i != e;++i)
			{
				m_solutions.emplace_back(std::vector<float>(std::begin(*i), std::end(*i)));
			}
			using V = std::vector<float>;
			std::sort(m_solutions.begin(), m_solutions.end(), [](const V& lhs, const V& rhs)
			{
				return lhs[0] > rhs[0];
			});
			if (num_dimensions == 2)
			{
				nonDominated2D();
			}
			else
			{
				auto r = front(std::make_pair(m_solutions.begin(), m_solutions.end()));
				m_solutions.erase(r.second, m_solutions.end());
			}
		}
	}

	size_t size() { return m_solutions.size(); }

	iterator begin() { return m_solutions.begin(); }
	iterator end() { return m_solutions.end(); }
	const_iterator begin() const { return m_solutions.begin(); }
	const_iterator end() const { return m_solutions.end(); }
private:
	void nonDominated2D()
	{
		float maximum = m_solutions[0][1];

		auto e = std::remove_if(m_solutions.begin() + 1, m_solutions.end(), 
		[&maximum](const std::vector<float>& v)
		{
			if (v[1] > maximum)
			{
				maximum = v[1];
				return false;
			}
			return true;
		});
		m_solutions.erase(e, m_solutions.end());
	}

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
				//TODO implement the non dominated check	

			}
			return t;
		}

	}
	SolutionsVector m_solutions;
};