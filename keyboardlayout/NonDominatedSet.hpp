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
		BaseNode(unsigned int region) : m_region(region), m_referenceValid(1) {}
		std::unique_ptr<BaseNode> m_child;
		std::unique_ptr<BaseNode> m_nextSibling;
		unsigned int m_region : 31;
		unsigned int m_referenceValid : 1;
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
		auto res = getResult();
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

	const Solution operator[](size_t index)
	{
		return getResult()[index];
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

#if 0
	template<typename SolutionType>
	InsertResult insertToHelper(unsigned int region, const KeyboardType& keyboard, const SolutionType& solution, InsertMode insertMode, std::unique_ptr<BaseNode>& node)
	{
		if (node && ((insertMode != InsertMode::Dominating && node->m_region <= region) || (insertMode == InsertMode::Dominating && node->m_region > region)))
		{
			if (node->m_child)
			{
				return insertToChild(keyboard, solution, region, insertMode, static_cast<Node&>(*node));
			}
			else
			{
				auto res = insertToLeaf(keyboard, solution, static_cast<LeafNode&>(*node), insertMode);
				if (res.second)
				{ 
					node = std::move(res.second);
				}
				return res.first;
			}
		}
		else if (insertMode == InsertMode::Both)
		{
			auto newNode = std::make_unique<LeafNode>(region);
			newNode->m_solutions.emplace_back(keyboard, std::begin(solution), std::end(solution));
			newNode->m_nextSibling = std::move(node);
			node = std::move(newNode);
			assert(!node->m_nextSibling || node->m_region < node->m_nextSibling->m_region);
			return InsertResult::Inserted;
		}
		return InsertResult::Unknown;
	}
#endif

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
				//KLUDGE patchup
				if (mode == InsertMode::Dominating)
				{
					insertRes = InsertResult::Inserted;
				}
			}
			else
			{
				if (getNode(baseNode).m_referenceValid && isDominated(getNode(baseNode).m_solution.m_solution, solution))
				{
					getNode(baseNode).m_referenceValid = false;
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
								return InsertResult::Dominated;
							}
						}
					}
					InsertMode newInsertMode = (mode == InsertMode::Both && getNode(baseNode).m_region == region) ? InsertMode::Both : InsertMode::Dominated;
					if (insertRes != InsertResult::Inserted)
					{
						if (getNode(baseNode).m_child && getNode(baseNode).m_child->m_region <= newRegion)
						{
							insertRes = insertToChild(keyboard, solution, newRegion, newInsertMode, getNode(baseNode).m_child);
							if (insertRes == InsertResult::Dominated || insertRes == InsertResult::Duplicate)
							{
								return insertRes;
							}
						}
						else if (newInsertMode == InsertMode::Both)
						{
							auto newNode = std::make_unique<LeafNode>(newRegion);
							newNode->m_solutions.emplace_back(keyboard, std::begin(solution), std::end(solution));
							newNode->m_nextSibling = std::move(baseNode->m_child);
							baseNode->m_child = std::move(newNode);
							assert(!baseNode->m_child->m_nextSibling || baseNode->m_child->m_nextSibling->m_region > baseNode->m_child->m_region);
							insertRes = InsertResult::Inserted;
						}
					}
					if (insertRes == InsertResult::Inserted)
					{
						if (newInsertMode == InsertMode::Both && baseNode->m_child->m_nextSibling)
						{
							insertToChild(keyboard, solution, newRegion, InsertMode::Dominating, baseNode->m_child->m_nextSibling);
						}
						else if (baseNode->m_child)
						{
							insertToChild(keyboard, solution, newRegion, InsertMode::Dominating, baseNode->m_child);
						}
					}
				}
			}
		}
		if (insertRes != InsertResult::Inserted)
		{
			if (baseNode->m_nextSibling && baseNode->m_nextSibling->m_region <= region)
			{
				insertRes = insertToChild(keyboard, solution, region, mode, baseNode->m_nextSibling);
				if (insertRes == InsertResult::Dominated || insertRes == InsertResult::Duplicate)
				{
					return insertRes;
				}
			}
			else if (mode == InsertMode::Both && baseNode->m_region < region)
			{
				auto newNode = std::make_unique<LeafNode>(region);
				newNode->m_solutions.emplace_back(keyboard, std::begin(solution), std::end(solution));
				newNode->m_nextSibling = std::move(baseNode->m_nextSibling);
				baseNode->m_nextSibling = std::move(newNode);
				assert(baseNode->m_nextSibling->m_region > baseNode->m_region);
				assert(!baseNode->m_nextSibling->m_nextSibling || baseNode->m_nextSibling->m_nextSibling->m_region > baseNode->m_nextSibling->m_region);
				insertRes = InsertResult::Inserted;
			}
		}
		if (insertRes == InsertResult::Inserted)
		{
			if (baseNode->m_nextSibling)
			{
				insertToChild(keyboard, solution, region, InsertMode::Dominating, baseNode->m_nextSibling);
			}
		}
		return insertRes;
	}

	template<typename SolutionType>
	InsertResult insertToLeaf(const KeyboardType& keyboard, const SolutionType& solution, InsertMode mode, std::unique_ptr<BaseNode>& node)
	{
		LeafNode& leaf = static_cast<LeafNode&>(*node);
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

		if (mode == InsertMode::Both && !dominated)
		{
			if (!solutionAssigned)
			{
				leaf.m_solutions.emplace_back(keyboard, std::begin(solution), std::end(solution));
			}

			if (leaf.m_solutions.size() > MaxLeafSize)
			{
				nondominatedset_detail::selectPivoitPoint(leaf.m_solutions);
				const unsigned int numRegions = 1u << NumObjectives;
				std::array<std::unique_ptr<LeafNode>, numRegions> regions;
				auto newNode = std::make_unique<Node>(static_cast<unsigned int>(leaf.m_region), std::move(leaf.m_solutions[0]));
				newNode->m_nextSibling = std::move(leaf.m_nextSibling);
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
						if (firstChild)
						{
							regions[i]->m_nextSibling = std::move(firstChild);
						}
						firstChild = std::move(regions[i]);
					}
				}
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

	std::vector<float> m_idealPoint;
	std::unique_ptr<BaseNode> m_root;
};