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
				// TODO it seems like the flow and distances are the wrong way around
				int a = keyboard.m_keys[i];
				int b = keyboard.m_keys[j];
				int dist = m_distances[i][j];
				int flow = m_flow[a][b];
				sum += dist * flow;
			}
		}
		return -static_cast<float>(sum);
	}
	
	virtual void evaluateNeighbourhood(const Keyboard<NumLocations>& keyboard, float v, std::array<std::array<float, NumLocations>, NumLocations>& delta) const override
	{
		for (size_t i = 0; i < NumLocations; i++)
		{
			for (size_t j = i + 1; j < NumLocations; j++)
			{
				delta[i][j] = -static_cast<float>(compute_delta(keyboard, i, j));
			}
		}
	}

private:
	int compute_delta(const Keyboard<NumLocations>& keyboard, size_t i, size_t j) const
	{
		auto& a = m_distances;
		auto& b = m_flow;
		auto& p = keyboard.m_keys;
		auto d = (a[i][i] - a[j][j])*(b[p[j]][p[j]] - b[p[i]][p[i]]) +
			(a[i][j] - a[j][i])*(b[p[j]][p[i]] - b[p[i]][p[j]]);

		for (size_t k = 0; k < NumLocations; k++)
		{
			if (k != i && k != j)
			{
				d = d + (a[k][i] - a[k][j])*(b[p[k]][p[j]] - b[p[k]][p[i]]) +
					(a[i][k] - a[j][k])*(b[p[j]][p[k]] - b[p[i]][p[k]]);
			}
		}

		return d;
	}

	std::array<std::array<int, NumLocations>, NumLocations> m_distances;
	std::array<std::array<int, NumLocations>, NumLocations> m_flow;
};