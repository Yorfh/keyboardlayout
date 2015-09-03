#pragma once
#include <fstream>
#include "Objective.hpp"

template<size_t NumLocations>
class QAP : public Objective<NumLocations>
{
public:
	QAP(const std::string& filename)
	{

	}

	float evaluate(const Keyboard<NumLocations>& keyboard) const override
	{
		return 0.0f;
	}
};