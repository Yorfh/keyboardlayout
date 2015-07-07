#pragma once
#include "Objective.hpp"

template<size_t NumCities>
class TravelingSalesman : public Objective<NumCities - 1>
{
public:
	TravelingSalesman(std::array<double, NumCities>& latitudes, std::array<double, NumCities>& longitudes)
	{
		auto convertToRadians = [](auto v) -> double
		{
			const double PI = 3.141592;
			double deg = int(v);
			double m = v - deg;
			double rad = PI * (deg + 5.0 * m / 3.0) / 180.0;
			return rad;
		};
		std::transform(latitudes.begin(), latitudes.end(), m_latitudes.begin(), convertToRadians);
		std::transform(longitudes.begin(), longitudes.end(), m_longitudes.begin(), convertToRadians);
	}

	float evaluate(const Keyboard<NumCities - 1>& keyboard) const override
	{
		int distance = 0;
		for (size_t i = 0; i < NumCities; i++)
		{
			size_t index1;
			size_t index2;
			if (i == 0)
			{
				index1 = 0;
				index2 = keyboard.m_keys[0] + 1;
			}
			else if (i == NumCities - 1)
			{
				index1 = keyboard.m_keys[i - 1] + 1;
				index2 = 0;
			}
			else
			{
				index1 = keyboard.m_keys[i - 1] + 1;
				index2 = keyboard.m_keys[i] + 1;

			}
			distance += calculateDistance(index1, index2);
		}
		return -static_cast<float>(distance);
	}

	int calculateDistance(size_t index1, size_t index2) const
	{
		const double RRR = 6378.388;
		double q1 = std::cos(m_longitudes[index1] - m_longitudes[index2]);
		double q2 = std::cos(m_latitudes[index1] - m_latitudes[index2]);
		double q3 = std::cos(m_latitudes[index1] + m_latitudes[index2]);
		int dij = int(RRR * std::acos(0.5*((1.0 + q1)*q2 - (1.0 - q1)*q3)) + 1.0);
		return dij;
	}

	std::array<double, NumCities> m_latitudes;
	std::array<double, NumCities> m_longitudes;
};
