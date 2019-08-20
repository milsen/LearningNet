#pragma once

#include <learningnet/Module.hpp>
#include <learningnet/LearningNet.hpp>
#include <lemon/connectivity.h> // for dag

namespace learningnet {

using namespace lemon;

class NetworkChecker : public Module
{
private:

#if 0
	// TDOO remove this unused function
	void topologicalSort(const LearningNet &net, lemon::ListDigraph::NodeMap<int> &type)
	{
		std::vector<lemon::ListDigraph::Node> sources;
		lemon::ListDigraph::NodeMap<bool> targetReachable(net, false);
		targetReachable[net.getTarget()] = true;
		std::map<lemon::ListDigraph::Node, lemon::ListDigraph::OutArcIt> currentConditionCase(net);

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
			visitedNodes.push_back(v); // remember v for later

			if (targetReachable[v]) {
				// TODO beautify, check == 1 while condition
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
					}
					targetReachable[visitedNode] = true;
					visitedNodes.pop_back();
				}

				// If we were able to pop all visitedNodes:
				// Path from source to target exists for every condition case.
				if (visitedNodes.empty()) {
					return true;
				}
			} else if (net.isCondition(v)) { // target not yet reachable
				// Condition: only push first condition case, later push other cases
				lemon::ListDigraph::OutArcIt a(net, v);
				currentConditionCase[v] = a;
				sources.push_back(net.target(a));
			} else {
				// Push all successors (unless it is still a locked join).
				for (lemon::ListDigraph::OutArcIt a(net, v); a != lemon::INVALID; ++a) {
					lemon::ListDigraph::Node u = net.target(a);

					if (net.isJoin(u)) {
						net.incrementActivatedInArcs(u);
					}

					if (!net.isJoin(u) || net.isUnlockedJoin(u)) {
						sources.push_back(u);
					}
				}
			}

			// If the node has no successors and is not targetReachable...
			lemon::ListDigraph::OutArcIt a(net, v);
			if (a == lemon::INVALID) {
				/* remove from visitedNodes without setting targetReachable */
				/* check until last split or condition...? */
				while (!visitedNodes.empty()) {
					lemon::ListDigraph::Node visitedNode = visitedNodes.back();
					if (visitedNode) {

					}

					visitedNodes.pop_back();
				}
			}
		}

		return false;
	}
#endif

	void compress(LearningNet &net)
	{
		std::vector<lemon::ListDigraph::Node> nodeStack;
		lemon::ListDigraph::NodeMap<bool> visited(net, false);

		// Start depth-first search at every unvisited node.
		for (ListDigraph::NodeIt v(net); v != INVALID; ++v) {
			if (!visited[v]) {
				nodeStack.push_back(v);

				while (!nodeStack.empty()) {
					lemon::ListDigraph::Node v = nodeStack.back();
					nodeStack.pop_back();


					// Add unvisited successors to the stack.
					for (lemon::ListDigraph::OutArcIt out(net, v); out != lemon::INVALID; ++out) {
						lemon::ListDigraph::Node tgt = net.target(out);
						if (!visited[tgt]) {
							nodeStack.push_back(tgt);
						}
					}

					// Contract nodes depending on their type.
					if (net.isUnit(v)) {
						// Remove unit
						if (net.isSource(v)) {
							// A unit without in-edges irrelevant for path-checking.
							lemon::ListDigraph::InArcIt in(net, v);
							lemon::ListDigraph::Node pred = net.source(in);
							if (net.isTarget(v)) {
								net.setTarget(pred);
							}
							net.contract(pred, v);
						} else {
							// Remove source units. Even if they are targets,
							// they can be definitely reached.
							net.erase(v);
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
			}
		}
	}

public:
	NetworkChecker(LearningNet &net) : Module() {
		call(net);
	}

	bool call(LearningNet &net)
	{
		std::map<int, bool> sectionExists;
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
				if (countInArcs(net, v) > 1) {
					failWithError("Condition node has more than one in-arc.");
				}

				// Check that each condition has an else-branch (otherwise it
				// might not always be possible to reach the target).
				bool elseBranchFound = false;
				for (lemon::ListDigraph::OutArcIt out(net, v); out != lemon::INVALID && !elseBranchFound; ++out) {
					if (net.getCondition(out) == CONDITION_ELSE_BRANCH_KEYWORD) {
						elseBranchFound = true;
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
		}

		// TODO when conditions exist, there must exist a path to target
		// => topological sort?
		compress(net);
		/* if (topologicalSort(net)) { */

		/* } */

		return true;
	}
};

}
