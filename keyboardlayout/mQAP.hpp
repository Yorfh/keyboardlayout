#pragma once
#include "Objective.hpp"
#include "Keyboard.hpp"
#include <string>
#include <fstream>

template<size_t NumLocations>
class mQAP : public Objective<NumLocations>
{
public:
	mQAP(const std::string& filename, size_t objective)
	{
		std::ifstream stream(filename);
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
		for (size_t k = 0; k < objective; k++)
		{
			for (size_t i = 0; i < NumLocations; i++)
			{
				std::getline(stream, std::string());
			}
			std::getline(stream, std::string());
		}
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
		int sum = 0;
		for (size_t i = 0; i < NumLocations; i++)
		{
			for (size_t j = 0; j < NumLocations; j++)
			{
				int a = keyboard.m_keys[i];
				int b = keyboard.m_keys[j];
				int dist = m_distances[a][b];
				int flow = m_flow[i][j];
				sum += dist * flow;
			}
		}
		return -static_cast<float>(sum);
	}
private:
	std::array<std::array<int, NumLocations>, NumLocations> m_distances;
	std::array<std::array<int, NumLocations>, NumLocations> m_flow;
};

