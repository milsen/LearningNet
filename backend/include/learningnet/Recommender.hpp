#pragma once
#include <learningnet/Module.hpp>
#include <learningnet/LearningNet.hpp>
#include <lemon/pairing_heap.h>
#include <algorithm>
#include <map>

namespace learningnet {

using NodeCosts = std::map<lemon::ListDigraph::Node, double>;
using NodePairCosts = std::map<lemon::ListDigraph::Node, NodeCosts>;
using ConditionMap = std::vector<std::vector<std::string>>;
using TestMap = std::map<int, int>;

class Recommender : public Module
{

private:
	LearningNet &m_net;

	const ConditionMap m_conditionVals;

	const TestMap m_testGrades;

	std::vector<lemon::ListDigraph::Node> m_firstActives;

	lemon::ListDigraph::NodeMap<int> m_nodeTypeBackup;

	/**
	 * Get sources of #m_net, i.e. nodes with indegree 0 or joins with 0
	 * necessary in-arcs.
	 * @return sources of #m_net
	 */
	std::vector<lemon::ListDigraph::Node> getSources()
	{
		std::vector<lemon::ListDigraph::Node> sources;
		for (auto v : m_net.nodes()) {
			if (m_net.isSource(v)) {
				sources.push_back(v);
			}
		}

		return sources;
	}

	/**
	 * Start a learning path search at \p sources over already completed nodes.
	 * Stop at inactive nodes, collecting them and setting their type to active.
	 * Return the found new active nodes.
	 *
	 * @param sources list of nodes at which the search for active nodes starts
	 * @return newly found active nodes
	 */
	std::vector<lemon::ListDigraph::Node> getNewActives(
		std::vector<lemon::ListDigraph::Node> &sources)
	{
		std::vector<lemon::ListDigraph::Node> actives;
		while (!sources.empty()) {
			lemon::ListDigraph::Node v = sources.back();
			sources.pop_back();
			switch (m_net.getType(v)) {
				case NodeType::inactive:
					actives.push_back(v);
					m_net.setType(v, NodeType::active);
					break;
				case NodeType::active:
					// TODO remove this?
					appendError("Input has active units set already. Why?");
					break;
				default:
					if (m_net.isUnknown(v)) {
						appendError("Input has nodes of unknown type.");
						break;
					}

					// Function to push an arc's target to sources.
					auto exploreArc = [&](const lemon::ListDigraph::OutArcIt &a) {
						lemon::ListDigraph::Node u = m_net.target(a);

						// Push join nodes only if all necessary in-edges are
						// activated. All other nodes only have one in-edge and
						// can be pushed directly when explored.
						if (m_net.isJoin(u)) {
							m_net.incrementActivatedInArcs(u);
						}

						// Once the activated in-arcs of a join reach the number
						// of its necessary in-arcs, push them. Do not push them
						// again if the join is visited another time.
						if (!m_net.isJoin(u) ||
							m_net.getActivatedInArcs(u) == m_net.getNecessaryInArcs(u)) {
							sources.push_back(u);
						}
					};

					// For a condition, only explore out-edges corresponding to set user-values.
					if (m_net.isCondition(v)) {
						// Get user values for this condition.
						std::vector<std::string> vals = m_conditionVals[m_net.getConditionId(v)];
						if (vals.empty()) {
							vals.push_back(CONDITION_ELSE_BRANCH_KEYWORD);
						}

						for (auto a : m_net.outArcs(v)) {
							if (std::find(vals.begin(), vals.end(), m_net.getConditionBranch(a)) != vals.end()) {
								exploreArc(a);
							}
						}
					} else if (m_net.isTest(v)) {
						// Get the branch with the highest grade that is still
						// below the actual grade of the user.
						// If the user has no grade, assume that he will reach
						// the best grade for this test.
						auto gradeIt = m_testGrades.find(m_net.getTestId(v));
						bool hasGrade = gradeIt != m_testGrades.end();
						int grade = hasGrade ? std::get<1>(*gradeIt) : 0;

						int maxBranchGrade = -1;
						lemon::ListDigraph::OutArcIt correctBranch;
						for (auto a : m_net.outArcs(v)) {
							int branchGrade = stoi(m_net.getConditionBranch(a));
							if ((!hasGrade || branchGrade <= grade) && branchGrade > maxBranchGrade) {
								maxBranchGrade = branchGrade;
								correctBranch = a;
							}
						}

						if (maxBranchGrade > -1) {
							exploreArc(correctBranch);
						}
					} else {
						// Else explore all out-edges (for completed units: only one).
						for (auto a : m_net.outArcs(v)) {
							exploreArc(a);
						}
					}

					break;
			}
		}

		return actives;
	}

	/**
	 * Reset the types of nodes of #m_net to the state they were in after the
	 * first call to #getNewActives().
	 */
	void resetNodeTypes() {
		for (auto v : m_net.nodes()) {
			m_net.setType(v, m_nodeTypeBackup[v]);
		}
	}

public:
	Recommender(LearningNet &net,
		const ConditionMap &conditionVals,
		const TestMap &testGrades)
	: Module()
	, m_net{net}
	, m_conditionVals{conditionVals}
	, m_testGrades{testGrades}
	, m_nodeTypeBackup{net}
	{
		std::vector<lemon::ListDigraph::Node> sources = getSources();
		m_firstActives = getNewActives(sources);

		// Remember types of nodes after first getNewActives() call.
		for (auto v : m_net.nodes()) {
			m_nodeTypeBackup[v] = m_net.getType(v);
		}
	}

	/**
	 * @return active nodes as calculated in the constructor
	 */
	std::vector<lemon::ListDigraph::Node> recActive()
	{
		return m_firstActives;
	}

	/**
	 * Returns the active node with minimum cost.
	 * @param nodeCosts
	 * @param actives
	 * @return best active node according to \p nodeCosts
	 */
	lemon::ListDigraph::Node recNext(const NodeCosts &nodeCosts,
		const std::vector<lemon::ListDigraph::Node> &actives)
	{
		double minCost = std::numeric_limits<double>::max();
		lemon::ListDigraph::Node recommended = lemon::INVALID;
		for (auto v : actives) {
			double cost = nodeCosts.at(v);
			if (cost < minCost) {
				recommended = v;
				minCost = cost;
			}
		}
		return recommended;
	}

	/**
	 * Overload of recNext to work with #m_firstActives.
	 */
	lemon::ListDigraph::Node recNext(const NodeCosts &nodeCosts)
	{
		return recNext(nodeCosts, m_firstActives);
	}

	/**
	 * @param nodePairCosts
	 * @param actives
	 * @param prev previously completed node
	 * @return best active node according to \p nodePairCosts
	 */
	lemon::ListDigraph::Node recNext(const NodePairCosts &nodePairCosts,
		const std::vector<lemon::ListDigraph::Node> &actives,
		const lemon::ListDigraph::Node &prev = lemon::INVALID)
	{
		lemon::ListDigraph::Node recommended = lemon::INVALID;
		double minCost = std::numeric_limits<double>::max();
		if (prev == lemon::INVALID) {
			// If there is no previously completed node, use the cost sum over
			// all node pairs starting at an active node as a guideline.
			for (auto v : actives) {
				double costSumForV = 0.0;
				for (auto pair : nodePairCosts.at(v)) {
					costSumForV += pair.second;
				}
				if (costSumForV < minCost) {
					recommended = v;
					minCost = costSumForV;
				}
			}
		} else {
			for (auto v : actives) {
				double cost = (nodePairCosts.at(prev)).at(v);
				if (cost < minCost) {
					recommended = v;
					minCost = cost;
				}
			}
		}
		return recommended;
	}

	/**
	 * Overload of recNext to work with #m_firstActives.
	 */
	lemon::ListDigraph::Node recNext(const NodePairCosts &nodePairCosts,
		const lemon::ListDigraph::Node &prev = lemon::INVALID)
	{
		return recNext(nodePairCosts, m_firstActives, prev);
	}

	/**
	 * @param nodeCosts
	 * @return heuristically best learning path according to \p nodeCosts
	 */
	std::vector<lemon::ListDigraph::Node> recPath(const NodeCosts &nodeCosts)
	{
		using PHeap = lemon::PairingHeap<double, lemon::ListDigraph::NodeMap<int>>;
		std::vector<lemon::ListDigraph::Node> result;

		// Heap contains currently active nodes, map needed for heap internals.
		lemon::ListDigraph::NodeMap<int> heapMap(m_net, PHeap::PRE_HEAP);
		PHeap heap{heapMap};

		for (auto v : m_firstActives) {
			heap.push(v, nodeCosts.at(v));
		}

		bool targetFound = false;
		while (!heap.empty() && !targetFound) {
			lemon::ListDigraph::Node bestActive = heap.top();
			heap.pop();

			// Update result and whether target was found.
			result.push_back(bestActive);
			if (m_net.isTarget(bestActive)) {
				targetFound = true;
			} else {
				// Search new actives on the basis of the new best active node.
				m_net.setType(bestActive, NodeType::completed);
				std::vector<lemon::ListDigraph::Node> newSources = {bestActive};
				for (auto v : getNewActives(newSources)) {
					heap.push(v, nodeCosts.at(v));
				}
			}
		}

		resetNodeTypes();
		return result;
	}

	/**
	 * @param nodePairCosts
	 * @return heuristically best learning path according to \p nodePairCosts
	 */
	std::vector<lemon::ListDigraph::Node> recPath(
		const NodePairCosts &nodePairCosts)
	{
		std::vector<lemon::ListDigraph::Node> result;
		std::vector<lemon::ListDigraph::Node> actives = m_firstActives;

		bool targetFound = false;
		lemon::ListDigraph::Node prevBestActive = lemon::INVALID;
		while (!actives.empty() && !targetFound) {
			lemon::ListDigraph::Node bestActive =
				recNext(nodePairCosts, actives, prevBestActive);
			// TODO improve performance, e.g. by giving back iterator and
			// deleting directly
			auto position = std::find(actives.begin(), actives.end(), bestActive);
			if (position != actives.end()) {
				actives.erase(position);
			}

			// Update result and whether target was found.
			result.push_back(bestActive);
			if (m_net.isTarget(bestActive)) {
				targetFound = true;
			} else {
				// Search new actives on the basis of the new best active node.
				m_net.setType(bestActive, NodeType::completed);
				std::vector<lemon::ListDigraph::Node> newSources = {bestActive};
				std::vector<lemon::ListDigraph::Node> newActives =
					getNewActives(newSources);

				// Concat newActives with actives.
				actives.insert(
					actives.end(),
					std::make_move_iterator(newActives.begin()),
					std::make_move_iterator(newActives.end())
				);
			}
		}

		resetNodeTypes();
		return result;
	}
};

}
