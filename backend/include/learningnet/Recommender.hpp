#pragma once
#include <learningnet/Module.hpp>
#include <learningnet/LearningNet.hpp>
#include <lemon/pairing_heap.h>
#include <algorithm>
#include <map>

namespace learningnet {

//! Representation of costs for each unit node.
using NodeCosts = std::map<lemon::ListDigraph::Node, double>;

//! Representation of costs for each pair of unit nodes.
using NodePairCosts = std::map<lemon::ListDigraph::Node, NodeCosts>;

//! Mapping of condition ids (vector indices) to vectors of condition values.
using ConditionMap = std::vector<std::vector<std::string>>;

//! Mapping of test ids to test grades.
using TestMap = std::map<int, int>;

/**
 * Computes active nodes and recommends learning paths (or unit nodes) for a
 * given learning net with accompanying completed sections, condition values and
 * test grades representing a specific learner.
 */
class Recommender : public Module
{

private:
	LearningNet &m_net; //!< learning net

	const ConditionMap m_conditionVals; //!< condition values of this learner

	const TestMap m_testGrades; //!< test grades of this learner

	//! active nodes as computed in constructor
	std::vector<lemon::ListDigraph::Node> m_firstActives;

	//! edges visited during first learning path search in constructor
	lemon::ListDigraph::ArcMap<bool> m_firstVisited;

	//! backup of node types after first learning path search in constructor
	lemon::ListDigraph::NodeMap<int> m_nodeTypeBackup;

	//! whether the target was found during the recommendation of a learning path
	bool m_targetFound;

	//! whether the target was already found after the first learning path
	//! search in the constructor
	bool m_targetFoundBackup;

	/**
	 * Get sources of #m_net, i.e. nodes with indegree 0.
	 * Side-effect: The activated in-arcs of each join node are reset.
	 *
	 * @return sources of #m_net
	 */
	std::vector<lemon::ListDigraph::Node> getSources()
	{
		std::vector<lemon::ListDigraph::Node> sources;
		for (auto v : m_net.nodes()) {
			if (m_net.isSource(v)) {
				sources.push_back(v);
			}

			if (m_net.isJoin(v)) {
				m_net.resetActivatedInArcs(v);
			}
		}

		return sources;
	}

	/**
	 * Start a learning path search at \p sources, skipping over already
	 * completed nodes.
	 * Stop at inactive nodes, collecting them and setting their type to active.
	 * Return the found new active nodes.
	 *
	 * @param sources list of nodes at which the search for active nodes starts
	 * @param visited is assigned true for each arc that is visited
	 * (assumes that visited is initialized with false for each arc in #m_net)
	 * @return newly found active nodes
	 */
	std::vector<lemon::ListDigraph::Node> getNewActives(
		std::vector<lemon::ListDigraph::Node> &sources,
		lemon::ListDigraph::ArcMap<bool> *visited = nullptr)
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
						if (visited) {
							(*visited)[a] = true;
						}
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

					if (m_net.isTarget(v)) {
						m_targetFound = true;
					}

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
	 * Reset the state of this Recommender, i.e. set #m_targetFound and types of
	 * nodes of #m_net to the state they were in after the first call to
	 * #getNewActives().
	 */
	void reset() {
		for (auto v : m_net.nodes()) {
			m_net.setType(v, m_nodeTypeBackup[v]);
		}
		m_targetFound = m_targetFoundBackup;
	}

public:
	/**
	 * Constructs a Recommender and immediately starts a learning path search
	 * resulting in active nodes.
	 *
	 * All other methods refer to the state after the search for the first
	 * actives, i.e. #recActive() returns the first active, #recNext() returns
	 * one of the first actives and #recPath() returns a learning path starting
	 * at one of the first actives.
	 *
	 * @param net learning net to be used as a basis for recommendation
	 * @param conditionVals mapping of condition ids to vectors of condition
	 * branches (that correspond to properties of the user)
	 * @param testGrades mapping of test ids to user grades
	 */
	Recommender(LearningNet &net,
		const ConditionMap &conditionVals,
		const TestMap &testGrades)
	: Module()
	, m_net{net}
	, m_conditionVals{conditionVals}
	, m_testGrades{testGrades}
	, m_firstVisited{net, false}
	, m_nodeTypeBackup{net}
	, m_targetFound{false}
	, m_targetFoundBackup{false}
	{
		std::vector<lemon::ListDigraph::Node> sources = getSources();
		m_firstActives = getNewActives(sources, &m_firstVisited);

		// Remember types of nodes after first getNewActives() call.
		for (auto v : m_net.nodes()) {
			m_nodeTypeBackup[v] = m_net.getType(v);
		}
		m_targetFoundBackup = m_targetFound;
	}

	/**
	 * @return a map assigning to each arc whether it was visited during the
	 * learning path search to find the first active nodes
	 */
	lemon::ListDigraph::ArcMap<bool> *getVisited()
	{
		return &m_firstVisited;
	}

	/**
	 * @return first active nodes as calculated in the constructor
	 */
	std::vector<lemon::ListDigraph::Node> recActive()
	{
		return m_firstActives;
	}

	/**
	 * Returns the active node with minimum cost.
	 * @param nodeCosts cost value for each unit node
	 * @param actives active nodes from which the recommended node is chosen
	 * @return iterator of actives at best active node according to \p nodeCosts
	 */
	std::vector<lemon::ListDigraph::Node>::const_iterator recNext(
		const NodeCosts &nodeCosts,
		const std::vector<lemon::ListDigraph::Node> &actives)
	{
		double minCost = std::numeric_limits<double>::max();
		std::vector<lemon::ListDigraph::Node>::const_iterator recommended = actives.end();
		for (auto it = actives.begin(); it != actives.end(); ++it) {
			double cost = nodeCosts.at(*it);
			if (cost < minCost) {
				recommended = it;
				minCost = cost;
			}
		}
		return recommended;
	}

	/**
	 * Overload of recNext to work with the output of #recActive().
	 */
	std::vector<lemon::ListDigraph::Node>::const_iterator recNext(
			const NodeCosts &nodeCosts)
	{
		return recNext(nodeCosts, m_firstActives);
	}

	/**
	 * @param nodePairCosts cost value for each pair of unit nodes
	 * @param actives active nodes from which the recommended node is chosen
	 * @param prev previously completed node
	 * @return pointer to best active node according to \p nodePairCosts
	 */
	std::vector<lemon::ListDigraph::Node>::const_iterator recNext(
		const NodePairCosts &nodePairCosts,
		const std::vector<lemon::ListDigraph::Node> &actives,
		const lemon::ListDigraph::Node &prev = lemon::INVALID)
	{
		std::vector<lemon::ListDigraph::Node>::const_iterator recommended = actives.end();
		double minCost = std::numeric_limits<double>::max();
		if (prev == lemon::INVALID) {
			// If there is no previously completed node, use the cost sum over
			// all node pairs starting at an active node as a guideline.
			for (auto it = actives.begin(); it != actives.end(); ++it) {
				double costSumForV = 0.0;
				for (auto pair : nodePairCosts.at(*it)) {
					costSumForV += pair.second;
				}
				if (costSumForV < minCost) {
					recommended = it;
					minCost = costSumForV;
				}
			}
		} else {
			for (auto it = actives.begin(); it != actives.end(); ++it) {
				double cost = (nodePairCosts.at(prev)).at(*it);
				if (cost < minCost) {
					recommended = it;
					minCost = cost;
				}
			}
		}
		return recommended;
	}

	/**
	 * Overload of recNext to work with the output of #recActive().
	 */
	std::vector<lemon::ListDigraph::Node>::const_iterator recNext(
		const NodePairCosts &nodePairCosts,
		const lemon::ListDigraph::Node &prev = lemon::INVALID)
	{
		return recNext(nodePairCosts, m_firstActives, prev);
	}

	/**
	 * @param nodeCosts cost value for each unit node
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

		while (!heap.empty() && !m_targetFound) {
			lemon::ListDigraph::Node bestActive = heap.top();
			heap.pop();

			// Update result and whether target was found.
			result.push_back(bestActive);
			if (!m_targetFound) {
				// Search new actives on the basis of the new best active node.
				m_net.setType(bestActive, NodeType::completed);
				std::vector<lemon::ListDigraph::Node> newSources = {bestActive};
				for (auto v : getNewActives(newSources)) {
					heap.push(v, nodeCosts.at(v));
				}
			}
		}

		reset();
		return result;
	}

	/**
	 * @param nodePairCosts cost value for each pair of unit nodes
	 * @param lastCompleted the last completed unit node (if one exists)
	 * @return heuristically best learning path according to \p nodePairCosts
	 */
	std::vector<lemon::ListDigraph::Node> recPath(
		const NodePairCosts &nodePairCosts,
		const lemon::ListDigraph::Node &lastCompleted = lemon::INVALID)
	{
		std::vector<lemon::ListDigraph::Node> result;
		std::vector<lemon::ListDigraph::Node> actives = m_firstActives;

		lemon::ListDigraph::Node bestActive = lastCompleted;
		while (!actives.empty() && !m_targetFound) {
			auto bestIt = recNext(nodePairCosts, actives, bestActive);
			if (bestIt == actives.end()) {
				// This should not happen since !actives.empty() at the
				// beginning of the while loop.
				break;
			}
			bestActive = *bestIt;
			actives.erase(bestIt);

			// Update result and whether target was found.
			result.push_back(bestActive);
			if (!m_targetFound) {
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

		reset();
		return result;
	}
};

}
