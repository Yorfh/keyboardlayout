#pragma once
#include <algorithm>

template<typename First, typename Second>
bool isDominated(First first, Second second)
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

