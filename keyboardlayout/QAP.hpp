#pragma once
#include <fstream>
#include "Objective.hpp"

template<size_t NumLocations, typename FloatingPoint = double>
class QAP : public Objective<NumLocations, FloatingPoint>
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

	FloatingPoint evaluate(const Keyboard<NumLocations>& keyboard) const override
	{
		uint64_t sum = 0;
		for (size_t i = 0; i < NumLocations; i++)
		{
			for (size_t j = 0; j < NumLocations; j++)
			{
				// TODO it seems like the flow and distances are the wrong way around
				int a = keyboard.m_keys[i];
				int b = keyboard.m_keys[j];
				uint64_t dist = m_distances[i][j];
				uint64_t flow = m_flow[a][b];
				sum += dist * flow;
			}
		}
		return -static_cast<float>(sum);
	}
	
	virtual void evaluateNeighbourhood(const Keyboard<NumLocations>& keyboard, FloatingPoint v, size_t lastSwapI, size_t lastSwapJ, std::array<std::array<FloatingPoint, NumLocations>, NumLocations>& delta) const override
	{
		bool firstSwap = lastSwapI == std::numeric_limits<size_t>::max() || lastSwapJ == std::numeric_limits<size_t>::max();
		for (size_t i = 0; i < NumLocations; i++)
		{
			for (size_t j = i + 1; j < NumLocations; j++)
			{
				if (firstSwap == false && i != lastSwapI && i != lastSwapJ && j != lastSwapI && j != lastSwapJ)
				{
					delta[i][j] += -static_cast<FloatingPoint>(computeDeltaPart(keyboard, delta, i, j, lastSwapI, lastSwapJ));
				}
				else
				{
					delta[i][j] = -static_cast<FloatingPoint>(computeDelta(keyboard, i, j));
				}
			}
		}
	}

private:
	int64_t computeDelta(const Keyboard<NumLocations>& keyboard, size_t i, size_t j) const
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

	int64_t computeDeltaPart(const Keyboard<NumLocations>& keyboard, const std::array<std::array<FloatingPoint, NumLocations>, NumLocations>& delta,
		size_t i, size_t j, size_t r, size_t s) const
	{
		auto& a = m_distances;
		auto& b = m_flow;
		auto& p = keyboard.m_keys;

		return((a[r][i] - a[r][j] + a[s][j] - a[s][i])*
			(b[p[s]][p[i]] - b[p[s]][p[j]] + b[p[r]][p[j]] - b[p[r]][p[i]]) +
			(a[i][r] - a[j][r] + a[j][s] - a[i][s])*
			(b[p[i]][p[s]] - b[p[j]][p[s]] + b[p[j]][p[r]] - b[p[i]][p[r]]));

	}

	std::array<std::array<int64_t, NumLocations>, NumLocations> m_distances;
	std::array<std::array<int64_t, NumLocations>, NumLocations> m_flow;
};