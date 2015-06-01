#pragma once
#include <array>
#include <random>
#include <algorithm>

template<size_t Size> 
class Keyboard
{
public:
	Keyboard()
	{
		for (int i = 0;i < m_keys.max_size(); i++)
		{
			m_keys[i] = i;
		}
	}

	Keyboard(std::array<int, Size>&& keys)
		: m_keys(std::move(keys))
	{
	}

	Keyboard(const std::array<int, Size>& keys)
		: m_keys(keys)
	{
	}

	void randomize(std::mt19937& randomGenerator)
	{
		std::shuffle(m_keys.begin(), m_keys.end(), randomGenerator);
	}

	std::array<int, Size> m_keys;
};