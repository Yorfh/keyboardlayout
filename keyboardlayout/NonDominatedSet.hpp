#pragma once
#include "Keyboard.hpp"
#include "Solutions.hpp"
#include <algorithm>
#include "boost/range.hpp"
#include "boost/tuple/tuple.hpp"
#include "boost/range/algorithm/remove_if.hpp"
#include "boost/iterator/zip_iterator.hpp"
#include "boost/range/algorithm/unique.hpp"
#include "boost/fusion/tuple/tuple.hpp"

template<size_t KeyboardSize>
class NonDominatedSet
{
public:


	using KeyboardType = Keyboard<KeyboardSize>;
	using Solution = std::vector<float>;
	using SolutionsVector = std::vector<Solution>;
	using KeyboardVector = std::vector<KeyboardType>;
	template<typename KeyboardType, typename SolutionType>
	using Reference = solutions_detail::Reference<KeyboardType, SolutionType>;
	using S = Solutions<KeyboardVector, SolutionsVector>;

	using iterator = typename S::iterator;
	using const_iterator = typename S::const_iterator;

	NonDominatedSet()
		: m_solution(m_keyboards, m_solutions)
	{

	}

	template<typename KeyboardVector, typename SolutionsVector>
	NonDominatedSet(const Solutions<KeyboardVector, SolutionsVector>& elements)
		: m_solution(m_keyboards, m_solutions)
	{
		size_t num_elements = boost::size(elements);
		if (num_elements == 0)
		{
			return;
		}
		size_t num_dimensions = boost::size(std::begin(elements)->solution());
		m_solutions.reserve(num_elements);
		for (auto&& e : elements)
		{
			insert(e);
		}
	}

	NonDominatedSet& operator=(const NonDominatedSet& rhs) = default;

	size_t size() { return m_solutions.size(); }

	template<typename KeyboardType, typename SolutionType>
	bool insert(const solutions_detail::Reference<KeyboardType, SolutionType> value)
	{
		bool dominated = false;
		auto i = std::remove_if(begin(), end(), 
		[&value, &dominated](typename iterator::reference rhs)
		{
			if (dominated)
			{
				return false;
			}
			else if (isDominated(rhs, value))
			{
				return true;
			}
			else if (isDominated(value, rhs))
			{
				dominated = true;
			}
			return false;
		});

		auto index = i - begin();
		m_solutions.erase(m_solutions.begin() + index, m_solutions.end());
		m_keyboards.erase(m_keyboards.begin() + index, m_keyboards.end());
		if (!dominated)
		{
			m_keyboards.emplace_back(value.keyboard());
			m_solutions.emplace_back(value.solution().begin(), value.solution().end());
		}
		return !dominated;
	}

	template<typename T1, typename T2, typename T3, typename T4>
	static bool isDominated(const Reference<T1, T2> lhs, const Reference<T3, T4> rhs)
	{
		auto b = boost::make_zip_iterator(boost::make_tuple(lhs.solution().begin(), rhs.solution().begin()));
		auto e = boost::make_zip_iterator(boost::make_tuple(lhs.solution().end(), rhs.solution().end()));
		auto r = boost::make_iterator_range(b, e);

		bool found = false;
		for (auto&& e: r)
		{
			
			if (e.get<0>() > e.get<1>())
			{
				return false;
			}
			else if (e.get<0>() < e.get<1>())
			{
				found = true;
			}
		}
		return found;
	}

	void removeDuplicates()
	{
		using V = typename iterator::reference;
		std::sort(begin(), end(),
		[](V lhs, V rhs)
		{
			return std::lexicographical_compare(lhs.solution().begin(), lhs.solution().end(), rhs.solution().begin(), rhs.solution().end());
		});

		auto e=std::unique(begin(), end(),
		[](V lhs, V rhs)
		{
			return std::equal(std::begin(lhs.solution()), std::end(lhs.solution()), std::begin(rhs.solution()), std::end(rhs.solution()));
		});
		auto index = e - begin();
		m_solutions.erase(m_solutions.begin() + index, m_solutions.end());
		m_keyboards.erase(m_keyboards.begin() + index, m_keyboards.end());
	}

	iterator begin()
	{
		return m_solution.begin();
	}
	iterator end()
	{
		return m_solution.end();
	}

	const_iterator begin() const
	{
		return m_solution.begin();
	}

	const_iterator end() const
	{
		return m_solution.end();
	}

private:
	SolutionsVector m_solutions;
	KeyboardVector m_keyboards;
	S m_solution;
};