#pragma once
#include <array>
#include <random>
#include <algorithm>

template<size_t Size, bool small = (Size < 256)>
struct KeyTypeHelper
{
	using type = unsigned char;
};

template<size_t Size>
struct KeyTypeHelper<Size, false> 
{
	using type = unsigned int;
};

template<size_t Size> 
class Keyboard
{
public:
	using KeyType = typename KeyTypeHelper<Size>::type;
	Keyboard()
	{
		for (int i = 0;i < m_keys.max_size(); i++)
		{
			m_keys[i] = i;
		}
	}

	Keyboard(std::array<KeyType, Size>&& keys)
		: m_keys(std::move(keys))
	{
	}

	Keyboard(const std::array<KeyType, Size>& keys)
		: m_keys(keys)
	{
	}

	void randomize(std::mt19937& randomGenerator)
	{
		std::shuffle(m_keys.begin(), m_keys.end(), randomGenerator);
	}

	bool operator==(const Keyboard& rhs) const
	{
		return memcmp(m_keys.data(), rhs.m_keys.data(), Size * sizeof(KeyType)) == 0;
	}

	std::array<KeyType, Size> m_keys;
};