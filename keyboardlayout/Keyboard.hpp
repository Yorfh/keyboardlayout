#pragma once
#include <array>
#include <random>
#include <algorithm>

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

	void randomize(std::mt19937& randomGenerator)
	{
		std::shuffle(m_keys.begin(), m_keys.end(), randomGenerator);
	}

	std::array<int, 3> m_keys;
};