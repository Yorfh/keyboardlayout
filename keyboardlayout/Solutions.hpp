#pragma once
#include "boost/iterator/iterator_facade.hpp"

template<typename KeyboardVector, typename SolutionVector>
class Solutions;

namespace solutions_detail
{
	template<typename Reference, typename Value> 
	class iterator : public boost::iterator_facade<iterator<Reference, Value>, Value, std::random_access_iterator_tag, Reference>
	{
		friend class boost::iterator_core_access;
		using Parent = Solutions<typename Reference::keyboard_vector_t, typename Reference::solution_vector_t>;
		friend class Parent;
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
		iterator(Parent* pParent, size_t index)
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

		Reference dereference() const
		{
			return Reference(m_pParent->m_keyboards[m_index], m_pParent->m_solutions[m_index]);
		}

		Parent* m_pParent;
		size_t m_index;
	};

	template<typename KeyboardType, typename SolutionType>
	class Reference;

	template<typename KeyboardVector, typename SolutionVector>
	class Value
	{
		friend class solutions_detail::iterator<Reference<KeyboardVector, SolutionVector>, Value<KeyboardVector, SolutionVector>>;
	public:
		using KeyboardType = typename KeyboardVector::value_type;
		using SolutionType = typename SolutionVector::value_type;
		using Reference = Reference<KeyboardVector, SolutionVector>;
		friend class Reference;
		KeyboardType& keyboard()
		{
			return m_keyboard;
		}

		SolutionType& solution()
		{
			return m_solution;
		}

		Value(const Reference& rhs)
			: m_keyboard(rhs.m_keyboard),
			m_solution(rhs.m_solution)
		{
		}
	protected:
		Value(KeyboardType& keyboard, SolutionType& solution)
			: m_keyboard(keyboard),
			m_solution(solution)
		{

		}
		KeyboardType m_keyboard;
		SolutionType m_solution;
	};

	template<typename KeyboardVector, typename SolutionVector>
	class Reference
	{
		friend class solutions_detail::iterator<Reference<KeyboardVector, SolutionVector>, Value<KeyboardVector, SolutionVector>>;
	public:
		using KeyboardType = typename KeyboardVector::value_type;
		using SolutionType = typename SolutionVector::value_type;
		using keyboard_vector_t = KeyboardVector;
		using solution_vector_t = SolutionVector;
		using Value = Value<KeyboardVector, SolutionVector>;
		friend class Value;
		KeyboardType& keyboard()
		{
			return m_keyboard;
		}

		SolutionType& solution()
		{
			return m_solution;
		}

		Reference& operator=(const Reference& rhs)
		{
			m_keyboard = rhs.m_keyboard;
			m_solution = rhs.m_solution;
			return *this;
		}

		Reference& operator=(const Value& rhs)
		{
			m_keyboard = rhs.m_keyboard;
			m_solution = rhs.m_solution;
			return *this;
		}
	protected:
		Reference(KeyboardType& keyboard, SolutionType& solution)
			: m_keyboard(keyboard),
			m_solution(solution)
		{

		}
		KeyboardType& m_keyboard;
		SolutionType& m_solution;
	};

	template<typename KeyboardVector, typename SolutionVector>
	void swap(typename Reference<KeyboardVector, SolutionVector> lhs, Reference<KeyboardVector, SolutionVector> rhs)
	{
		Value<KeyboardVector, SolutionVector> temp = std::move(lhs);
		lhs = std::move(rhs);
		rhs = std::move(temp);
	}
}


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
	using Value = solutions_detail::Value<KeyboardVector, SolutionVector>;
	using Reference = solutions_detail::Reference<KeyboardVector, SolutionVector>;
	using iterator = solutions_detail::iterator<Reference, Value>;
	friend class iterator;

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

