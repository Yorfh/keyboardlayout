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

	template<typename P1, typename P2>
	float distanceBetweenPoints(const P1& p1, const P2& p2)
	{
		float dist = 0.0f;
		for (size_t i = 0; i < p1.size(); i++)
		{
			float d = p2[i] - p1[i];
			dist += d * d;
		}
		return std::sqrt(dist);
	}

	template<typename SolutionArray>
	void selectPivotPoint(SolutionArray& solutions)
	{
		// This is based on the paper:
		// BSkyTree: Scalable Skyline Computation Using A Balanced Pivot Selection

		if (solutions.size() >= 2)
		{
			auto ret = std::min_element(std::begin(solutions), std::end(solutions), 
			[](auto& element1, auto& element2)
			{
				return distance(element1.m_solution) < distance(element2.m_solution);
			});
			std::swap(*std::begin(solutions), *ret);
		}
	}

	template<typename Point1, typename Point2>
	unsigned int mapPointToRegion(Point1& reference, Point2& p)
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
		BaseNode(unsigned int region) : m_region(region), m_referenceValid(1), m_size(1) {}
		std::unique_ptr<BaseNode> m_child;
		std::unique_ptr<BaseNode> m_nextSibling;
		unsigned int m_region : 31;
		unsigned int m_referenceValid : 1;
		unsigned int m_size;
	};
	
	struct Node : public BaseNode
	{
		Node(unsigned int region, Solution&& solution)
			: BaseNode(region)
			, m_solution(std::move(solution))
		{}
		Solution m_solution;
	};

	struct LeafNode : public BaseNode
	{
		LeafNode(unsigned int region)
			: BaseNode(region)
		{}
		SolutionsVector m_solutions;
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
		auto s = solutions.begin();
		for (auto k = keyboards.begin(); k != keyboards.end(); ++k, ++s)
		{
			insert(*k, *s);
		}
	}

	NonDominatedSet(const NonDominatedSet& rhs)
	{
		auto solutions = rhs.getResult();
		for (auto&& s : solutions)
		{
			insert(s.m_keyboard, s.m_solution);
		}
	}

	NonDominatedSet(NonDominatedSet&& rhs)
		: m_root(std::move(rhs.m_root))
		, m_distanceToParetoFront(rhs.m_distanceToParetoFront)
		, m_idealPoint(std::move(rhs.m_idealPoint))
	{
	}

	NonDominatedSet& operator=(const NonDominatedSet& rhs) = delete;

	NonDominatedSet& operator=(NonDominatedSet&& rhs)
	{
		m_root = std::move(rhs.m_root);
		m_distanceToParetoFront = rhs.m_distanceToParetoFront;
		m_idealPoint = std::move(rhs.m_idealPoint);
		return *this;
	}

	size_t size() const
	{
		if (m_root)
		{
			return m_root->m_size;
		}
		else
		{
			return 0;
		}
	}

	const std::vector<float> getIdealPoint() const
	{
		return m_idealPoint;
	}

	float getLastParetoDistance() const
	{
		return m_distanceToParetoFront;
	}

	template<typename SolutionType>
	bool insert(const KeyboardType& keyboard, const SolutionType& solution)
	{
		// The algorithm used is based on the two following papers
		// "Scalable Skyline Computation Using Object-based Space Partitioning"
		// "BSkyTree: Scalable Skyline Computation Using A Balanced Pivot Selection"
		m_distanceToParetoFront = 0.0f;
		if (m_idealPoint.empty())
		{
			m_idealPoint.assign(solution.size(), std::numeric_limits<float>::lowest());
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
			auto res = insertToChild(keyboard, solution, 0, InsertMode::Both, m_root);
			inserted = (res == InsertResult::Inserted || res == InsertResult::Duplicate);
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

	SolutionsVector getResult() const
	{
		SolutionsVector res;
		if (m_root)
		{
			getResult(res, *m_root);
		}
		return res;
	}

	const Solution& operator[](size_t index) const
	{
		return getIndex(*m_root, static_cast<unsigned int>(index));
	}

private:
	enum class InsertResult
	{
		Inserted,
		Dominated,
		Duplicate,
		NonDominated,
		Unknown,
	};

	enum class InsertMode
	{
		Dominating = 1,
		Dominated = 2,
		Both = 4,
	};

	Node& getNode(std::unique_ptr<BaseNode>& ptr)
	{
		return static_cast<Node&>(*ptr);
	}

	template<typename SolutionType>
	InsertResult insertToChild(const KeyboardType& keyboard, const SolutionType& solution, unsigned int region, InsertMode mode, std::unique_ptr<BaseNode>& baseNode)
	{
		bool compatible = (baseNode->m_region | region) == (mode == InsertMode::Dominating ? baseNode->m_region : region);
		assert(!baseNode->m_nextSibling || baseNode->m_nextSibling->m_region > baseNode->m_region);
		InsertResult insertRes = mode == InsertMode::Dominating ? InsertResult::Inserted : InsertResult::Unknown;
		if (compatible && (mode != InsertMode::Dominating || baseNode->m_region >= region))
		{
			if (baseNode->m_child == nullptr)
			{
				InsertMode newInsertMode = mode;
				if (mode == InsertMode::Both && baseNode->m_region != region)
				{
					newInsertMode = InsertMode::Dominated;
				}
				insertRes = insertToLeaf(keyboard, solution, newInsertMode, baseNode);
				if (insertRes == InsertResult::Dominated || insertRes == InsertResult::Duplicate)
				{
					assert(mode != InsertMode::Dominating);
					return insertRes;
				}
				if (mode == InsertMode::Dominating)
				{
					insertRes = InsertResult::Inserted;
				}
			}
			else
			{
				if (baseNode->m_referenceValid && isDominated(getNode(baseNode).m_solution.m_solution, solution))
				{
					baseNode->m_referenceValid = false;
					baseNode->m_size--;
				}

				if (compatible)
				{
					unsigned int newRegion = nondominatedset_detail::mapPointToRegion(getNode(baseNode).m_solution.m_solution, solution);
					if (newRegion == (1u << NumObjectives) - 1)
					{
						if (getNode(baseNode).m_solution.m_keyboard == keyboard)
						{
							return InsertResult::Duplicate;
						}
						if (mode == InsertMode::Both || mode == InsertMode::Dominated)
						{
							if (isDominated(solution, getNode(baseNode).m_solution.m_solution))
							{
								m_distanceToParetoFront = nondominatedset_detail::distanceBetweenPoints(solution, getNode(baseNode).m_solution.m_solution);
								return InsertResult::Dominated;
							}
						}
					}
					InsertMode newInsertMode = (mode == InsertMode::Both && baseNode->m_region == region) ? InsertMode::Both : InsertMode::Dominated;
					if (insertRes != InsertResult::Inserted)
					{
						if (baseNode->m_child && baseNode->m_child->m_region <= newRegion)
						{
							unsigned int oldChildSize = baseNode->m_child->m_size;
							insertRes = insertToChild(keyboard, solution, newRegion, newInsertMode, baseNode->m_child);
							baseNode->m_size += baseNode->m_child->m_size - oldChildSize;
							if (insertRes == InsertResult::Dominated || insertRes == InsertResult::Duplicate)
							{
								return insertRes;
							}
						}
						else if (newInsertMode == InsertMode::Both)
						{
							unsigned int oldSize = baseNode->m_child ? baseNode->m_child->m_size : 0;
							auto newNode = std::make_unique<LeafNode>(newRegion);
							newNode->m_solutions.emplace_back(keyboard, std::begin(solution), std::end(solution));
							if (baseNode->m_child)
							{
								newNode->m_size += baseNode->m_child->m_size;
								newNode->m_nextSibling = std::move(baseNode->m_child);
							}
							baseNode->m_child = std::move(newNode);
							baseNode->m_size += baseNode->m_child->m_size - oldSize;
							assert(!baseNode->m_child->m_nextSibling || baseNode->m_child->m_nextSibling->m_region > baseNode->m_child->m_region);
							insertRes = InsertResult::Inserted;
						}
					}
					if (insertRes == InsertResult::Inserted)
					{
						if (newInsertMode == InsertMode::Both && baseNode->m_child->m_nextSibling)
						{
							unsigned int oldChildSize = baseNode->m_child->m_nextSibling->m_size;
							insertToChild(keyboard, solution, newRegion, InsertMode::Dominating, baseNode->m_child->m_nextSibling);
							int diff = baseNode->m_child->m_nextSibling->m_size - oldChildSize;
							baseNode->m_child->m_size += diff;
							baseNode->m_size += diff;
						}
						else if (baseNode->m_child)
						{
							unsigned int oldChildSize = baseNode->m_child->m_size;
							insertToChild(keyboard, solution, newRegion, InsertMode::Dominating, baseNode->m_child);
							baseNode->m_size += baseNode->m_child->m_size - oldChildSize;
						}
					}
				}
			}
		}
		if (insertRes != InsertResult::Inserted)
		{
			if (baseNode->m_nextSibling && baseNode->m_nextSibling->m_region <= region)
			{
				unsigned int oldSize = baseNode->m_nextSibling->m_size;
				insertRes = insertToChild(keyboard, solution, region, mode, baseNode->m_nextSibling);
				baseNode->m_size += baseNode->m_nextSibling->m_size - oldSize;
				if (insertRes == InsertResult::Dominated || insertRes == InsertResult::Duplicate)
				{
					return insertRes;
				}
			}
			else if (mode == InsertMode::Both && baseNode->m_region < region)
			{
				unsigned int oldSize = baseNode->m_nextSibling ? baseNode->m_nextSibling->m_size : 0;
				auto newNode = std::make_unique<LeafNode>(region);
				newNode->m_solutions.emplace_back(keyboard, std::begin(solution), std::end(solution));
				if (baseNode->m_nextSibling)
				{
					newNode->m_size += baseNode->m_nextSibling->m_size;
					newNode->m_nextSibling = std::move(baseNode->m_nextSibling);
				}
				baseNode->m_nextSibling = std::move(newNode);
				baseNode->m_size += baseNode->m_nextSibling->m_size - oldSize;
				assert(baseNode->m_nextSibling->m_region > baseNode->m_region);
				assert(!baseNode->m_nextSibling->m_nextSibling || baseNode->m_nextSibling->m_nextSibling->m_region > baseNode->m_nextSibling->m_region);
				insertRes = InsertResult::Inserted;
			}
		}
		if (insertRes == InsertResult::Inserted)
		{
			if (baseNode->m_nextSibling)
			{
				unsigned int oldSize = baseNode->m_nextSibling->m_size;
				insertToChild(keyboard, solution, region, InsertMode::Dominating, baseNode->m_nextSibling);
				baseNode->m_size += baseNode->m_nextSibling->m_size - oldSize;
			}
		}
		return insertRes;
	}

	template<typename SolutionType>
	InsertResult insertToLeaf(const KeyboardType& keyboard, const SolutionType& solution, InsertMode mode, std::unique_ptr<BaseNode>& node)
	{
		LeafNode& leaf = static_cast<LeafNode&>(*node);
		unsigned int oldOwnSize = static_cast<unsigned int>(leaf.m_solutions.size());
		bool dominated = false;
		auto sItr = leaf.m_solutions.begin();
		auto end = leaf.m_solutions.end();

		bool solutionAssigned = false;

		bool checkIsDominated = mode == InsertMode::Both || mode == InsertMode::Dominated;
		bool checkIsDominating = mode == InsertMode::Both || mode == InsertMode::Dominating;

		while (sItr != end)
		{
			if (mode != InsertMode::Dominating && sItr->m_keyboard == keyboard)
			{
				return InsertResult::Duplicate;
			}
			if (checkIsDominating && isDominated(sItr->m_solution, solution))
			{
				if (mode == InsertMode::Both && !solutionAssigned)
				{
					std::copy(std::begin(solution), std::end(solution), std::begin(sItr->m_solution));
					sItr->m_keyboard = keyboard;
					// The pruning power stays the same
					solutionAssigned = true;
				}
				else
				{
					leaf.m_solutions.erase(std::remove_if(sItr, end, [&solution](auto& s) 
					{ 
						return isDominated(s.m_solution, solution); 
					}), end);
					break;
				}
			}
			else if (checkIsDominated && !solutionAssigned && isDominated(solution, sItr->m_solution))
			{
				m_distanceToParetoFront = nondominatedset_detail::distanceBetweenPoints(solution, sItr->m_solution);
				sItr->m_pruningPower++;
				unsigned int pruningPower = sItr->m_pruningPower;
				while (sItr != leaf.m_solutions.begin())
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
		leaf.m_size += static_cast<unsigned int>(leaf.m_solutions.size()) - oldOwnSize;

		if (mode == InsertMode::Both && !dominated)
		{
			if (!solutionAssigned)
			{
				leaf.m_solutions.emplace_back(keyboard, std::begin(solution), std::end(solution));
				leaf.m_size++;
			}

			if (leaf.m_solutions.size() > MaxLeafSize)
			{
				nondominatedset_detail::selectPivotPoint(leaf.m_solutions);
				const unsigned int numRegions = 1u << NumObjectives;
				std::array<std::unique_ptr<LeafNode>, numRegions> regions;
				auto newNode = std::make_unique<Node>(static_cast<unsigned int>(leaf.m_region), std::move(leaf.m_solutions[0]));
				newNode->m_nextSibling = std::move(leaf.m_nextSibling);
				if (newNode->m_nextSibling)
				{
					newNode->m_size += newNode->m_nextSibling->m_size;
				}
				for (auto itr = leaf.m_solutions.begin() + 1; itr != leaf.m_solutions.end(); ++itr)
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
						regions[i]->m_size = static_cast<unsigned int>(regions[i]->m_solutions.size());
						if (firstChild)
						{
							regions[i]->m_size += firstChild->m_size;
							regions[i]->m_nextSibling = std::move(firstChild);
						}
						firstChild = std::move(regions[i]);
					}
				}
				newNode->m_size += firstChild->m_size;
				newNode->m_child = std::move(firstChild);
				node = std::move(newNode);
				return InsertResult::Inserted;
			}
			return InsertResult::Inserted;
		}
		else if (!dominated)
		{
			return InsertResult::NonDominated;
		}
		else
		{
			return InsertResult::Dominated;
		}
	}

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
			if (n.m_referenceValid)
			{
				res.push_back(n.m_solution);
			}
			getResult(res, *n.m_child);
		}
		if (node.m_nextSibling)
		{
			getResult(res, *node.m_nextSibling);
		}
	}

	const Solution& getIndex(const BaseNode& node, unsigned int index) const
	{
		if (node.m_child)
		{
			if (index == node.m_child->m_size)
			{
				if (node.m_referenceValid)
				{
					return static_cast<const Node&>(node).m_solution;
				}
				else
				{
					return getIndex(*node.m_nextSibling, index - node.m_child->m_size);
				}
			}
			else if (index > node.m_child->m_size)
			{
				return getIndex(*node.m_nextSibling, index - node.m_child->m_size - node.m_referenceValid ? 1 : 0);
			}
			else
			{
				return getIndex(*node.m_child, index);
			}
		}
		else
		{
			auto& leafNode = static_cast<const LeafNode&>(node);
			if (index < leafNode.m_solutions.size())
			{
				return leafNode.m_solutions[index];
			}
			else
			{
				return getIndex(*leafNode.m_nextSibling, index - static_cast<unsigned int>(leafNode.m_solutions.size()));
			}
		}
	}

	float m_distanceToParetoFront;
	std::vector<float> m_idealPoint;
	std::unique_ptr<BaseNode> m_root;
};