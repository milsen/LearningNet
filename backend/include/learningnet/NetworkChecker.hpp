#pragma once

#include <learningnet/Module.hpp>
#include <learningnet/LearningNet.hpp>
#include <lemon/connectivity.h> // for dag

namespace learningnet {

using namespace lemon;

class NetworkChecker : public Module
{
private:

	// TDOO remove this unused function
	bool pathsForAllConditions(const LearningNet &net,
			const std::map<int, std::vector<std::string>> &conditionIdToBranches)
	{
		std::vector<lemon::ListDigraph::Node> sources;
		/* lemon::ListDigraph::NodeMap<bool> targetReachable(net, false); */
		/* targetReachable[net.getTarget()] = true; */
		std::map<lemon::ListDigraph::Node, lemon::ListDigraph::OutArcIt> currentConditionCase(net);

		// TODO remove targetReachable or use it by checking whether conditions
		// themselves without any previous

		// Collect nodes with indegree 0.
		for (lemon::ListDigraph::NodeIt v(net); v != lemon::INVALID; ++v) {
			if (net.isSource(v)) {
				sources.push_back(v);
			}
		}

		// Save stack of visited nodes, the last of which are to be marked as
		// targetReachable after target is reached (or simply removed from stack
		// if no path reaches the target).
		std::vector<lemon::ListDigraph::Node> visitedNodes;

		while (!sources.empty()) {
			lemon::ListDigraph::Node v = sources.back();
			sources.pop_back();
			visitedNodes.push_back(v); // remember v for later backtracking in dfs

			if (v == net.getTarget()) {
				// If target is reachable from v, pop all last visited nodes
				// until last condition split from visitedNodes stack and mark
				// them as targetReachable.
				while (!visitedNodes.empty()) {
					lemon::ListDigraph::Node visitedNode = visitedNodes.back();

					// If the node is a condition with at least one case not
					// explored, explore that case and break this loop.
					if (net.isCondition(visitedNode)) {
						lemon::ListDigraph::OutArcIt condCase = currentConditionCase[visitedNode];
						++condCase;
						if (condCase != lemon::INVALID) {
							currentConditionCase[visitedNode] = condCase;
							sources.push_back(net.target(condCase));
							break;
						}
					} else if (net.isJoin(visitedNode)) {
						// decrementActivatedInArcs
					}

					visitedNodes.pop_back();
				}
			} else if (net.isCondition(v)) { // target not yet reachable
				// Condition: only push first condition case, later push other cases
				lemon::ListDigraph::OutArcIt a(net, v);
				currentConditionCase[v] = a;
				sources.push_back(net.target(a));
			} else {
				// Non-Condition: Push all successors (unless it is still a locked join).
				for (lemon::ListDigraph::OutArcIt a(net, v); a != lemon::INVALID; ++a) {
					lemon::ListDigraph::Node u = net.target(a);

					if (net.isJoin(u)) {
						net.incrementActivatedInArcs(u);
					}

					if (!net.isJoin(u) || net.isUnlockedJoin(u)) {
						// Push conditions to front, they'll be used after other
						// nodes such that those nodes are visited less often.
						if (net.isCondition(u)) {
							sources.push_front(u);
						} else {
							sources.push_back(u);
						}
					}
				}
			}
		}

		// If we were able to pop all visitedNodes:
		// Path from source to target exists for every condition case.
		return visitedNodes.empty();
	}

	void compress(LearningNet &net)
	{
		std::vector<lemon::ListDigraph::Node> nodeStack;
		lemon::ListDigraph::NodeMap<bool> visited(net, false);
		lemon::ListDigraph::NodeMap<int> visitedFromHowManyPreds(net, 0);

		// sources
		std::vector<lemon::ListDigraph::Node> sources;

		// Collect nodes with indegree 0.
		for (lemon::ListDigraph::NodeIt v(net); v != lemon::INVALID; ++v) {
			if (net.isJoin(v)) {
				net.resetActivatedInArcs(v);
			}

			if ((!net.isJoin(v) && countInArcs(net, v) == 0)
			   || net.isUnlockedJoin(v)) {
				sources.push_back(v);
			}
		}


		// Start at sources...
		// Start breadth-first search at every source.
		while (!sources.empty()) {
			lemon::ListDigraph::Node v = sources.front();
			nodeStack.pop_front();
			visited[v] = true;

			// Contract nodes depending on their type.
			// If a node is a unit, just remove it and continue.
			if (net.isUnit(v)) {
				if (net.isSource(v)) {
					// Remove source units. Even if they are targets,
					// they can be definitely reached.
					net.erase(v);
				} else {
					// Else contract them with their predecessor.
					lemon::ListDigraph::InArcIt in(net, v);
					lemon::ListDigraph::Node pred = net.source(in);
					if (net.isTarget(v)) {
						net.setTarget(pred);
					}
					net.contract(pred, v);
				}
				continue;
			} else


			// Add unvisited successors to the stack.
			for (lemon::ListDigraph::OutArcIt out(net, v); out != lemon::INVALID; ++out) {
				lemon::ListDigraph::Node tgt = net.target(out);
				if (!visited[tgt]) {
					nodeStack.push_back(tgt);
				}
			}

			} else if (net.isSplit(v)) {
				// Combine splits.
				for (lemon::ListDigraph::OutArcIt out(net, v); out != lemon::INVALID; ++out) {
					lemon::ListDigraph::Node succ = net.target(out);
					if (net.isSplit(succ)) {
						if (net.isTarget(succ)) {
							net.setTarget(v);
						}
						net.contract(v, succ);
					}
				}

			} else if (net.isJoin(v)) {
				// Combine 1-joins or *-joins.
				lemon::ListDigraph::OutArcIt out(net, v);
				lemon::ListDigraph::Node succ = net.target(out);
				if (out != lemon::INVALID && net.isJoin(succ)) {
					int necArcsV = net.getNecessaryInArcs(v);
					int necArcsSucc = net.getNecessaryInArcs(succ);
					if ((necArcsV == 1 && necArcsSucc == 1)
						|| (necArcsV == countInArcs(net, v) &&
							necArcsSucc == countInArcs(net, succ))) {
						if (net.isTarget(succ)) {
							net.setTarget(v);
						}
						net.contract(v, succ);
					}
				}
			}
	}

public:
	NetworkChecker(LearningNet &net) : Module() {
		call(net);
	}

	bool call(LearningNet &net)
	{
		int conditionCount = 0;

		std::map<int, bool> sectionExists;
		std::map<int, std::vector<std::string>> conditionIdToBranches;
		for (ListDigraph::NodeIt v(net); v != INVALID; ++v) {
			// Check number of in-/out-arcs for each node-type.
			if (net.isUnit(v)) {
				int section = net.getSection(v);
				if (countInArcs(net, v) > 1 || countOutArcs(net, v) > 1) {
					failWithError("Unit node of section " + std::to_string(section)
						+ " has more than one in-arc or out-arc.");
				}

				// Each section only occurs once.
				if (sectionExists[section]) {
					failWithError("Section " + std::to_string(section) + " used multiple times.");
					return false;
				}
				sectionExists[section] = true;

			} else if (net.isJoin(v)) {
				if (countOutArcs(net, v) > 1) {
					failWithError("Join node has more than one out-arc.");
				}

				// Necessary inarcs of a join are less than actual InArcs.
				int necessaryInArcs = net.getNecessaryInArcs(v);
				int actualInArcs = countInArcs(net, v);
				if (necessaryInArcs > actualInArcs) {
					failWithError("Join node has " + std::to_string(necessaryInArcs) +
						" necessary in-arcs but only " + std::to_string(actualInArcs) +
						" actual in-arcs.");
					return false;
				}

			} else if (net.isSplit(v)) {
				if (countInArcs(net, v) > 1) {
					failWithError("Split node has more than one in-arc.");
				}

			} else if (net.isCondition(v)) {
				conditionCount++;
				if (countInArcs(net, v) > 1) {
					failWithError("Condition node has more than one in-arc.");
				}

				// Check that each condition has an else-branch (otherwise it
				// might not always be possible to reach the target).
				bool elseBranchFound = false;
				for (lemon::ListDigraph::OutArcIt out(net, v); out != lemon::INVALID && !elseBranchFound; ++out) {
					std::string conditionBranch = net.getConditionBranch(out);
					if (conditionBranch == CONDITION_ELSE_BRANCH_KEYWORD) {
						elseBranchFound = true;
					} else {
						conditionIdToBranches[net.getConditionId(v)].push_back(conditionBranch);
					}
				}
				if (!elseBranchFound) {
					failWithError("Condition has no else branch.");
				}

			}
		}

		if (!dag(net)) {
			failWithError("Given network is not acyclic.");
			return false;
		} else if (conditionCount == 0) {
			// If there are no conditions, then the network is already valid if
			// it's acyclic.
			return true;
		}

		// TODO when conditions exist, there must exist a path to target
		// => topological sort?
		compress(net);
		return pathsForAllConditions(net, conditionIdToBranches);
	}
};

}
