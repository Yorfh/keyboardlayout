#pragma once
#include "Keyboard.hpp"
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

	// We need to fake that we are a random_access_iterator
	template<typename KeyboardIterator, typename SolutionsIterator>
	class IteratorHelper
		: public boost::iterator_adaptor<
		IteratorHelper<KeyboardIterator, SolutionsIterator>
		, boost::zip_iterator<boost::tuple<KeyboardIterator, SolutionsIterator>>
		, boost::use_default
		, std::random_access_iterator_tag
		>
	{
	public:
		using base_iterator = boost::zip_iterator<boost::tuple<KeyboardIterator, SolutionsIterator>>;
		IteratorHelper() {}
		explicit IteratorHelper(const base_iterator& b) : iterator_adaptor_(b) {}
	private:
	};

	using iterator = IteratorHelper<typename KeyboardVector::iterator, typename SolutionsVector::iterator>;
	using const_iterator = IteratorHelper<typename KeyboardVector::const_iterator, typename SolutionsVector::const_iterator>;
	using value_type = typename SolutionsVector::value_type;

	using KeyboardSolution = boost::tuple<KeyboardType&, Solution&>;

	NonDominatedSet()
	{

	}

	template<typename T>
	NonDominatedSet(const T& elements)
	{
		size_t num_elements = boost::size(elements);
		if (num_elements == 0)
		{
			return;
		}
		size_t num_dimensions = boost::size(boost::get<1>(*std::begin(elements)));
		m_solutions.reserve(num_elements);
		for (auto&& e : elements)
		{
			insert(e);
		}
	}

	size_t size() { return m_solutions.size(); }

	template<typename T>
	bool insert(T&& keyboard_solution_pair)
	{
		static_assert(std::is_convertible<T, KeyboardSolution>::value, "You need to pass a KeyboardSolution");
		bool dominated = false;
		auto i = std::remove_if(begin(), end(), 
		[&keyboard_solution_pair, &dominated](typename iterator::value_type& rhs)
		{
			if (dominated)
			{
				return false;
			}
			else if (isDominated(rhs, keyboard_solution_pair))
			{
				return true;
			}
			else if (isDominated(keyboard_solution_pair, rhs))
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
			m_keyboards.emplace_back(keyboard_solution_pair.get<0>());
			m_solutions.emplace_back(keyboard_solution_pair.get<1>().begin(),
				keyboard_solution_pair.get<1>().end());
		}
		return !dominated;
	}

	template<typename T1, typename T2>
	static bool isDominated(T1&& lhs, T2&& rhs)
	{
		auto b = boost::make_zip_iterator(boost::make_tuple(lhs.get<1>().begin(), rhs.get<1>().begin()));
		auto e = boost::make_zip_iterator(boost::make_tuple(lhs.get<1>().end(), rhs.get<1>().end()));
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
		using V = typename iterator::value_type;
		std::sort(begin(), end(),
		[](const V& lhs, const V& rhs)
		{
			return std::lexicographical_compare(lhs.get<1>().begin(), lhs.get<1>().end(), rhs.get<1>().begin(), rhs.get<1>().end());
		});

		auto e=std::unique(begin(), end(),
		[](const V& lhs, const V& rhs)
		{
			return std::equal(std::begin(lhs.get<1>()), std::end(lhs.get<1>()), std::begin(rhs.get<1>()), std::end(rhs.get<1>()));
		});
		auto index = e - begin();
		m_solutions.erase(m_solutions.begin() + index, m_solutions.end());
		m_keyboards.erase(m_keyboards.begin() + index, m_keyboards.end());
	}

	iterator begin()
	{
		return iterator(boost::make_zip_iterator(boost::make_tuple(m_keyboards.begin(), m_solutions.begin())));
	}
	iterator end()
	{
		return iterator(boost::make_zip_iterator(boost::make_tuple(m_keyboards.end(), m_solutions.end())));
	}

	const_iterator begin() const
	{
		return const_iterator(boost::make_zip_iterator(boost::make_tuple(m_keyboards.begin(), m_solutions.begin())));
	}

	const_iterator end() const
	{
		return const_iterator(boost::make_zip_iterator(boost::make_tuple(m_keyboards.end(), m_solutions.end())));
	}

private:
	SolutionsVector m_solutions;
	KeyboardVector m_keyboards;
};