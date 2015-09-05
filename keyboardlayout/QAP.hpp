#pragma once
#include <fstream>
#include "Objective.hpp"

template<size_t NumLocations>
class QAP : public Objective<NumLocations>
{
public:
	QAP(const std::string& filename)
	{
		std::ifstream stream(filename);
		std::getline(stream, std::string());
		std::getline(stream, std::string());
		for (size_t i = 0; i < NumLocations; i++)
		{
			for (size_t j = 0; j < NumLocations; j++)
			{
				stream >> m_distances[i][j];
			}
			std::getline(stream, std::string());
		}
		std::getline(stream, std::string());
		for (size_t i = 0; i < NumLocations; i++)
		{
			for (size_t j = 0; j < NumLocations; j++)
			{
				stream >> m_flow[i][j];
			}
			std::getline(stream, std::string());
		}
	}

	float evaluate(const Keyboard<NumLocations>& keyboard) const override
	{
		uint64_t sum = 0;
		for (size_t i = 0; i < NumLocations; i++)
		{
			for (size_t j = 0; j < NumLocations; j++)
			{
				int a = keyboard.m_keys[i];
				int b = keyboard.m_keys[j];
				int dist = m_distances[i][j];
				int flow = m_flow[a][b];
				sum += dist * flow;
			}
		}
		return -static_cast<float>(sum);
	}
private:
	std::array<std::array<int, NumLocations>, NumLocations> m_distances;
	std::array<std::array<int, NumLocations>, NumLocations> m_flow;

};