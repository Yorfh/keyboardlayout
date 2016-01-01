#pragma once
#include <algorithm>

template<typename First, typename Second>
bool isDominated(const First& first, const Second& second)
{
	bool found = false;
	auto j = std::begin(second);
	for (auto i = std::begin(first); i != std::end(first); ++i, ++j)
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

template<typename T>
struct InOut : public std::reference_wrapper<T>
{
public:
	explicit InOut(T& v)
		: reference_wrapper<T>(v)
	{
	}

	InOut(const InOut&) = delete;
	InOut(InOut&& rhs)
		: reference_wrapper(rhs.get())
	{
	}

	InOut& operator=(const InOut) = delete;
	InOut& operator=(InOut&&) = delete;

	InOut& operator=(const T& v)
	{
		get() = v;
		return *this;
	}
};

template<typename T>
InOut<T> inOut(T& v)
{
	return InOut<T>(v);
}

template<typename T>
InOut<T> inOut(InOut<T>& i)
{
	return inOut(i.get());
}
