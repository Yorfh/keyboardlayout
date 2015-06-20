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
		using ParentHelper = Solutions<typename Reference::keyboard_vector_t, typename Reference::solution_vector_t>;
		using Parent = typename std::conditional<std::is_const<Value>::value, const ParentHelper, ParentHelper>::type;
		friend typename Parent;
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
			return *this;
		}

		iterator& operator=(iterator&& rhs)
		{
			if (&rhs != this)
			{
				m_pParent = rhs.m_pParent;
				m_index = rhs.m_index;
			}
			return *this;
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

		typename iterator::difference_type distance_to(const iterator& rhs) const
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
			return Reference(m_pParent->m_keyboards.get()[m_index], m_pParent->m_solutions.get()[m_index]);
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
		friend class solutions_detail::iterator<const Reference<KeyboardVector, SolutionVector>, const Value<KeyboardVector, SolutionVector>>;
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

		Value(const Value& rhs)
			: m_keyboard(rhs.m_keyboard),
			m_solution(rhs.m_solution)
		{
		}

		Value(const Reference& rhs)
			: m_keyboard(rhs.m_keyboard),
			m_solution(rhs.m_solution)
		{
		}

		Value(Value&& rhs)
			: m_keyboard(std::move(rhs.m_keyboard)),
			m_solution(std::move(rhs.m_solution))
		{
		}

		Value(Reference&& rhs)
			: m_keyboard(std::move(rhs.m_keyboard)),
			m_solution(std::move(rhs.m_solution))
		{
		}

		Value& operator=(const Value& rhs)
		{
			m_keyboard = rhs.m_keyboard;
			m_solution = rhs.m_solution;
		}

		Value& operator=(const Reference& rhs)
		{
			m_keyboard = rhs.m_keyboard;
			m_solution = rhs.m_solution;
		}

		Value& operator=(Value&& rhs)
		{
			m_keyboard = std::move(rhs.m_keyboard);
			m_solution = std::move(rhs.m_solution);
		}
		
		Value& operator=(Reference&& rhs)
		{
			m_keyboard = std::move(rhs.m_keyboard);
			m_solution = std::move(rhs.m_solution);
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
		friend class solutions_detail::iterator<const Reference<KeyboardVector, SolutionVector>, const Value<KeyboardVector, SolutionVector>>;
	public:
		using KeyboardType = typename KeyboardVector::value_type;
		using SolutionType = typename SolutionVector::value_type;
		using keyboard_vector_t = KeyboardVector;
		using solution_vector_t = SolutionVector;
		using Value = Value<KeyboardVector, SolutionVector>;
		friend class Value;
		Reference(Reference& rhs)
			: m_keyboard(rhs.m_keyboard),
			m_solution(rhs.m_solution)
		{
		}

		Reference(Reference&& rhs)
			: m_keyboard(rhs.m_keyboard),
			m_solution(rhs.m_solution)
		{
		}

		Reference(Value& rhs)
			: m_keyboard(rhs.m_keyboard),
			m_solution(rhs.m_solution)
		{
		}

		Reference(Value&& rhs)
			: m_keyboard(rhs.m_keyboard),
			m_solution(rhs.m_solution)
		{
		}

		KeyboardType& keyboard()
		{
			return m_keyboard;
		}

		SolutionType& solution()
		{
			return m_solution;
		}

		const KeyboardType& keyboard() const
		{
			return m_keyboard;
		}

		const SolutionType& solution() const
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

		Reference& operator=(Value&& rhs)
		{
			m_keyboard = std::move(rhs.m_keyboard);
			m_solution = std::move(rhs.m_solution);
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

	Solutions& operator=(const Solutions& rhs)
	{
		m_keyboards = rhs.m_keyboards;
		m_solutions = rhs.m_solutions;
		return *this;
	}

	using KeyboardType = typename KeyboardVector::value_type;
	using SolutionType = typename SolutionVector::value_type;
	using Value = solutions_detail::Value<KeyboardVector, SolutionVector>;
	using Reference = solutions_detail::Reference<KeyboardVector, SolutionVector>;
	using iterator = solutions_detail::iterator<Reference, Value>;
	using const_iterator = solutions_detail::iterator<const Reference, const Value>;
	friend class iterator;
	friend class const_iterator;

	iterator begin()
	{
		return iterator(this, 0);
	}

	iterator end()
	{
		return iterator(this, m_keyboards.get().size());
	}

	const_iterator begin() const
	{
		return const_iterator(this, 0);
	}

	const_iterator end() const
	{
		return const_iterator(this, m_keyboards.get().size());
	}
private:
	std::reference_wrapper<KeyboardVector> m_keyboards;
	std::reference_wrapper<SolutionVector> m_solutions;
};

template<typename KeyboardVector, typename SolutionsVector>
Solutions<KeyboardVector, SolutionsVector> makeSolutions(KeyboardVector& keyboards, SolutionsVector& solutions)
{
	return Solutions<KeyboardVector, SolutionsVector>(keyboards, solutions);
}
