#pragma once
#include <algorithm>

template<typename First, typename Second>
bool isDominated(First first, Second second)
{
	bool found = false;
	auto j = std::begin(second);
	for (auto i = std::begin(first); i != std::end(first); ++i, ++j)
	{
		if (*i > *j)
		{
			return false;
		}
		else if (*i < *j)
		{
			found = true;
		}
	}
	return found;
}

class FitnessCalculator
{
public:
	template<typename NDS, typename SolutionArray, typename OutItr>
	void calculateFitness(const NDS& nds, const SolutionArray& solutions, OutItr output)
	{
		auto& indices = nds.m_indices;
		size_t ndsSize = nds.size();
		using IndexItr = typename NDS::IndexVector::const_iterator;
		IndexItr indexBegin = indices.begin();
		IndexItr indexEnd = nds.m_firstFree;
		m_ndsFitnesses.reserve(ndsSize);
		m_ndsFitnesses.clear();
		std::transform(indexBegin, indexEnd, std::back_inserter(m_ndsFitnesses),
		[&solutions, &nds](size_t index)
		{
			auto& ndsSolution = nds.m_solutions[index];
			auto numDominating = std::count_if(std::begin(solutions), std::end(solutions), 
			[&ndsSolution](auto& s)
			{
				return isDominated(s, ndsSolution);
			});
			size_t populationSize = solutions.size();
			return static_cast<float>(numDominating) / populationSize;
		});
		std::transform(std::begin(solutions), std::end(solutions), output,
		[&indexBegin, &indexEnd, this, &nds](auto& solution)
		{
			float sum = 1.0f;
			auto ndsFittness = std::begin(m_ndsFitnesses);
			for (auto i = indexBegin; i != indexEnd; ++i)
			{
				auto& ndsSolution = nds.m_solutions[*i];
				if (isDominated(solution, ndsSolution))
				{
					sum += *ndsFittness;
				}
				++ndsFittness;
			}
			return 1.0f / sum;
		});
	}
private:
	std::vector<float> m_ndsFitnesses;
};

