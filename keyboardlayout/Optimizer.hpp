#pragma once
#include <functional>
#include "Keyboard.hpp"
#include <random>
#include <vector>
class Objective;

class Optimizer
{
public:
	template<typename Itr>
	Keyboard optimize(Itr begin, Itr end, size_t numGenerations)
	{
		auto k = Keyboard();
		auto best = std::make_tuple(k, begin->evaluate(k));
		for (int i = 0;i < numGenerations; ++i)
		{
			k.randomize(m_randomGenerator);
			float v = begin->evaluate(k);
			if (v > std::get<1>(best))
			{
				best = std::make_pair(k, v);
			}
		}
		return std::get<0>(best);
	}
private:
	std::mt19937 m_randomGenerator;
};