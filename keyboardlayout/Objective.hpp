#pragma once

template<size_t KeyboardSize>
class Keyboard;

template<size_t KeyboardSize>
class Objective
{
public:
	virtual float evaluate(const Keyboard<KeyboardSize>& keyboard) const = 0;
	virtual void evaluateNeighbourhood(const Keyboard<KeyboardSize>& keyboard, float v, std::array<std::array<float, KeyboardSize>, KeyboardSize>& delta) const
	{
		for (size_t i = 0;i < KeyboardSize; i++)
		{
			for (size_t j = i + 1;j < KeyboardSize; j++)
			{
				Keyboard<KeyboardSize> k = keyboard;
				std::swap(k.m_keys[i], k.m_keys[j]);
				delta[i][j] = evaluate(k) - v;
			}
		}
	}
};
