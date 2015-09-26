#pragma once

template<size_t KeyboardSize>
class Keyboard;

template<size_t KeyboardSize>
class Objective
{
public:
	static const size_t NoSwap = std::numeric_limits<size_t>::max();

	virtual float evaluate(const Keyboard<KeyboardSize>& keyboard) const = 0;

	void evaluateFirstNeighbourhood(const Keyboard<KeyboardSize>& keyboard, float v, std::array<std::array<float, KeyboardSize>, KeyboardSize>& delta)
	{
		evaluateNeighbourhood(keyboard, v, NoSwap, NoSwap, delta);
	}

	virtual void evaluateNeighbourhood(const Keyboard<KeyboardSize>& keyboard, float v, size_t lastSwapI, size_t lastSwapJ, std::array<std::array<float, KeyboardSize>, KeyboardSize>& delta) const
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
