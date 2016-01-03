#pragma once

template<size_t KeyboardSize>
class Keyboard;

template<size_t KeyboardSize, typename FloatingPoint = float>
class Objective
{
public:
	using floating_point_t = FloatingPoint;
	static const size_t NoSwap = std::numeric_limits<size_t>::max();

	virtual FloatingPoint evaluate(const Keyboard<KeyboardSize>& keyboard) const = 0;

	void evaluateFirstNeighbourhood(const Keyboard<KeyboardSize>& keyboard, FloatingPoint v, std::array<std::array<FloatingPoint, KeyboardSize>, KeyboardSize>& delta) const
	{
		evaluateNeighbourhood(keyboard, v, NoSwap, NoSwap, delta);
	}

	virtual void evaluateNeighbourhood(const Keyboard<KeyboardSize>& keyboard, FloatingPoint v, size_t lastSwapI, size_t lastSwapJ, std::array<std::array<FloatingPoint, KeyboardSize>, KeyboardSize>& delta) const
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
