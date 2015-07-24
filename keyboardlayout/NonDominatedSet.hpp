#pragma once
#include "Keyboard.hpp"
#include "Helpers.hpp"
#include <algorithm>
#include <numeric>
#include <cassert>
#include <memory>


namespace nondominatedset_detail
{

	template<typename S>
	float distance(const S& s)
	{
		auto maximum = *std::max_element(std::begin(s), std::end(s));
		auto minimum = *std::min_element(std::begin(s), std::end(s));
		return maximum - minimum;
	}

	template<typename SolutionArray>
	void selectPivoitPoint(SolutionArray& solutions)
	{
		// This is based on the paper:
		// BSkyTree: Scalable Skyline Computation Using A Balanced Pivot Selection

		if (solutions.size() >= 2)
		{
			auto head = std::begin(solutions);
			auto cur = head + 1;
			auto tail = std::end(solutions) - 1;
			float minDist = distance(head->m_solution);
			while (cur <= tail)
			{
				if (isDominated(cur->m_solution, head->m_solution))
				{
					if (cur != tail)
					{
						*cur = std::move(*tail);
					}
					solutions.erase(tail);
					tail--;
				}
				else if (isDominated(head->m_solution, cur->m_solution))
				{
					*head = std::move(*cur);
					if (cur != tail)
					{
						*cur = std::move(*tail);
					}
					solutions.erase(tail);
					tail--;
					cur = head + 1;
					minDist = distance(head->m_solution);
				}
				else
				{
					float curDist = distance(cur->m_solution);
					if (curDist < minDist)
					{
						bool dominated = false;
						for (auto i = head + 1; i != cur; i++)
						{
							if (isDominated(cur->m_solution, i->m_solution))
							{
								dominated = true;
								break;
							}
						}
						if (!dominated)
						{
							std::swap(*head, *cur);
							minDist = curDist;
						}
					}
					++cur;
				}
			}
		}
	}

	template<typename Point>
	unsigned int mapPointToRegion(Point& reference, Point& p)
	{
		unsigned int mask = 1;
		unsigned int ret = 0;
		for (size_t i = 0; i < reference.size(); i++)
		{
			if (p[i] <= reference[i])
			{
				ret |= mask;
			}
			mask <<= 1;
		}
		return ret;
	}
}

template<size_t KeyboardSize, size_t NumObjectives, size_t MaxLeafSize = std::numeric_limits<size_t>::max()>
class NonDominatedSet
{
public:
	using KeyboardType = Keyboard<KeyboardSize>;
	struct Solution
	{
		template<typename Itr>
		Solution(const KeyboardType& keyboard, Itr begin, Itr end)
			: m_keyboard(keyboard)
			, m_pruningPower(0)
		{
			std::copy(begin, end, std::begin(m_solution));
		}

		KeyboardType m_keyboard;
		std::array<float, NumObjectives> m_solution;
		unsigned int m_pruningPower;
	};

	using SolutionsVector = std::vector<Solution>;
private:
	struct BaseNode
	{
		std::unique_ptr<BaseNode> m_child;
		std::unique_ptr<BaseNode> m_nextSibling;
	};
	
	struct Node : public BaseNode
	{
		Node(unsigned int region, Solution&& solution)
			: m_region(region)
			, m_solution(std::move(solution))
		{}
		Solution m_solution;
		unsigned int m_region;
	};

	struct LeafNode : public BaseNode
	{
		LeafNode(unsigned int region)
			: m_region(region)
		{}
		SolutionsVector m_solutions;
		unsigned int m_region;
	};
public:

	NonDominatedSet()
	{
	}

	template<typename KeyboardArray, typename SolutionsArray>
	NonDominatedSet(const KeyboardArray& keyboards, const SolutionsArray& solutions)
	{
		size_t num_elements = keyboards.size();
		if (num_elements == 0)
		{
			return;
		}
		assert(solutions[0].size() == NumObjectives);
		m_idealPoint.assign(NumObjectives, std::numeric_limits<float>::min());
		auto s = solutions.begin();
		for (auto k = keyboards.begin(); k != keyboards.end(); ++k, ++s)
		{
			insert(*k, *s);
		}
	}

	NonDominatedSet(const NonDominatedSet& rhs)
		:
		m_keyboards(rhs.m_keyboards),
		m_solutions(rhs.m_solutions),
		m_indices(rhs.m_indices)
	{
		m_firstFree = m_firstFree + (rhs.m_firstFree - rhs.m_indices.begin());
	}

	NonDominatedSet(NonDominatedSet&& rhs)
		:
		m_keyboards(std::move(rhs.m_keyboards)),
		m_solutions(std::move(rhs.m_solutions)),
		m_indices(std::move(rhs.m_indices)),
		m_firstFree(std::move(rhs.m_firstFree))
	{
	}

	NonDominatedSet& operator=(const NonDominatedSet& rhs)
	{
		m_keyboards = rhs.m_keyboards;
		m_solutions = rhs.m_solutions;
		m_indices = rhs.m_indices;
		m_firstFree = m_firstFree + (rhs.m_firstFree - rhs.m_indices.begin());
		return *this;
	}

	NonDominatedSet& operator=(NonDominatedSet&& rhs)
	{
		m_root = std::move(rhs.m_root);
		return *this;
	}

	size_t size() const
	{
		return getResult().size();
	}

	const std::vector<float> getIdealPoint() const
	{
		return m_idealPoint;
	}

	template<typename SolutionType>
	bool insert(const KeyboardType& keyboard, const SolutionType& solution)
	{
		if (m_idealPoint.empty())
		{
			m_idealPoint.assign(solution.size(), std::numeric_limits<float>::min());
		}
		bool inserted = false;
		if (!m_root)
		{
			auto root = std::make_unique<LeafNode>(0);
			root->m_solutions.emplace_back(keyboard, std::begin(solution), std::end(solution));
			m_root = std::move(root);
			inserted = true;
		}
		else
		{
			if (m_root->m_child)
			{
				inserted = insertToChild(keyboard, solution, static_cast<Node&>(*m_root));
			}
			else
			{
				auto res = insertToLeaf(keyboard, solution, static_cast<LeafNode&>(*m_root));
				inserted = res.first;
				if (res.second)
				{
					m_root = std::move(res.second);
				}
			}
		}
		if (inserted)
		{
			for (size_t i = 0; i < m_idealPoint.size(); i++)
			{
				m_idealPoint[i] = std::max(m_idealPoint[i], solution[i]);
			}
		}
		return inserted;
	}

	template<typename SolutionType>
	bool insertToChild(const KeyboardType& keyboard, const SolutionType& solution, Node& node)
	{
		return false;
	}

	template<typename SolutionType>
	std::pair<bool, std::unique_ptr<Node>> insertToLeaf(const KeyboardType& keyboard, const SolutionType& solution, LeafNode& node)
	{
		bool dominated = false;
		auto sItr = node.m_solutions.begin();
		auto end = node.m_solutions.end();

		bool solutionAssigned = false;
		while (sItr != end)
		{
			if (sItr->m_keyboard == keyboard)
			{
				return std::make_pair(false, nullptr);
			}
			if (isDominated(sItr->m_solution, solution))
			{
				if (!solutionAssigned)
				{
					std::copy(std::begin(solution), std::end(solution), std::begin(sItr->m_solution));
					sItr->m_keyboard = keyboard;
					// The pruning power stays the same
					solutionAssigned = true;
				}
				else
				{
					node.m_solutions.erase(std::remove_if(sItr, end, [&solution](auto& s) { return isDominated(s.m_solution, solution); }), end);
					break;
				}
			}
			else if (!solutionAssigned && isDominated(solution, sItr->m_solution))
			{
				sItr->m_pruningPower++;
				unsigned int pruningPower = sItr->m_pruningPower;
				while (sItr != node.m_solutions.begin())
				{
					auto prev = sItr - 1;
					if (prev->m_pruningPower < pruningPower)
					{
						std::swap(*sItr, *prev);
					}
					else
					{
						break;
					}
					sItr = prev;
				}
				dominated = true;
				break;
			}

			++sItr;
		}

		if (!dominated)
		{
			if (!solutionAssigned)
			{
				node.m_solutions.emplace_back(keyboard, std::begin(solution), std::end(solution));
			}

			if (node.m_solutions.size() > MaxLeafSize)
			{
				nondominatedset_detail::selectPivoitPoint(node.m_solutions);
				const unsigned int numRegions = 1u << NumObjectives;
				std::array<std::unique_ptr<LeafNode>, numRegions> regions;
				auto newNode = std::make_unique<Node>(node.m_region, std::move(node.m_solutions[0]));
				for (auto itr = node.m_solutions.begin() + 1; itr != node.m_solutions.end(); ++itr)
				{
					auto& s = *itr;
					unsigned int region = nondominatedset_detail::mapPointToRegion(newNode->m_solution.m_solution, s.m_solution);
					if (!regions[region])
					{
						regions[region] = std::make_unique<LeafNode>(region);
					}
					regions[region]->m_solutions.emplace_back(s);
					regions[region]->m_solutions.back().m_pruningPower = 0;
				}
				std::unique_ptr<LeafNode> firstChild;
				for (int i = numRegions - 1; i >= 0; i--)
				{
					if (regions[i])
					{
						if (firstChild)
						{
							regions[i]->m_nextSibling = std::move(firstChild);
						}
						firstChild = std::move(regions[i]);
					}
				}
				newNode->m_child = std::move(firstChild);
				return std::make_pair(true, std::move(newNode));
			}
		}
		return std::make_pair(true, nullptr);
	}


	SolutionsVector getResult() const
	{
		SolutionsVector res;
		if (m_root)
		{
			getResult(res, *m_root);
		}
		return res;
	}

	const Solution operator[](size_t index)
	{
		return getResult()[index];
	}

private:
	void getResult(SolutionsVector& res, BaseNode& node) const
	{
		if (!node.m_child)
		{
			auto& leafNode = static_cast<LeafNode&>(node);
			res.insert(res.end(), leafNode.m_solutions.begin(), leafNode.m_solutions.end());
		}
		else
		{
			auto& n = static_cast<Node&>(node);
			getResult(res, *n.m_child);
			if (n.m_nextSibling)
			{
				getResult(res, *n.m_nextSibling);
			}
		}
	}

	std::vector<float> m_idealPoint;
	std::unique_ptr<BaseNode> m_root;
};