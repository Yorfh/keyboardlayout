#pragma once
#include "boost/iterator/iterator_facade.hpp"

template<typename KeyboardVector, typename SolutionVector>
class Solutions
{
public:
	Solutions(KeyboardVector& keyboards, SolutionVector& solutions)
		:
		m_keyboards(keyboards),
		m_solutions(solutions)
	{

	}

	using KeyboardType = typename KeyboardVector::value_type;
	using SolutionType = typename SolutionVector::value_type;

	class Value
	{
		using KeyboardType = typename Solutions::KeyboardType;
		using SolutionType = typename Solutions::SolutionType;
		friend class Solutions::iterator;
	public:
		KeyboardType& keyboard()
		{
			return m_keyboard;
		}

		SolutionType& solution()
		{
			return m_solution;
		}

		Value& operator=(const Value& rhs)
		{
			m_keyboard = rhs.m_keyboard;
			m_solution = rhs.m_solution;
			return *this;
		}
	private:
		Value(KeyboardType& keyboard, SolutionType& solution)
			: m_keyboard(keyboard),
			m_solution(solution)
		{

		}
		KeyboardType& m_keyboard;
		SolutionType& m_solution;
	};

	class iterator : public boost::iterator_facade<iterator, Value, std::random_access_iterator_tag, Value>
	{
		friend class boost::iterator_core_access;
		friend class Solutions;
	public:
		iterator()
			: m_pParent(nullptr),
			m_index(-1)
		{
		}

		iterator(const iterator& rhs)
			: m_pParent(rhs.m_pParent),
			m_index(rhs.m_index)
		{

		}

		iterator(iterator&& rhs)
			: m_pParent(rhs.m_pParent),
			m_index(rhs.m_index)
		{

		}

		iterator& operator=(const iterator& rhs)
		{
			if (&rhs != this)
			{
				m_pParent = rhs.m_pParent;
				m_index = rhs.m_index;
			}
		}

		iterator& operator=(iterator&& rhs)
		{
			if (&rhs != this)
			{
				m_pParent = rhs.m_pParent;
				m_index = rhs.m_index;
			}
		}
	private:
		iterator(Solutions* pParent, size_t index)
			: m_pParent(pParent),
			m_index(index)
		{

		}

		void increment()
		{
			++m_index;
		}

		void decrement()
		{
			--m_index;
		}

		void advance(size_t n)
		{
			m_index += n;
		}

		typename iterator::difference_type distance_to(const iterator& rhs)
		{
			return rhs.m_index - m_index;

		}

		bool equal(const iterator& rhs) const
		{
			return m_pParent == rhs.m_pParent && m_index == rhs.m_index;
			return false;
		}

		Value dereference() const
		{
			return Value(m_pParent->m_keyboards[m_index], m_pParent->m_solutions[m_index]);
		}

		Solutions* m_pParent;
		size_t m_index;
	};

	iterator begin()
	{
		return iterator(this, 0);
	}

	iterator end()
	{
		return iterator(this, m_keyboards.size());
	}
private:
	KeyboardVector& m_keyboards;
	SolutionVector& m_solutions;
};
