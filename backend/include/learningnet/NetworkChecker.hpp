#pragma once

#include <learningnet/Module.hpp>
#include <learningnet/LearningNet.hpp>
#include <lemon/connectivity.h> // for dag
#include <deque>

namespace learningnet {

using namespace lemon;

class NetworkChecker : public Module
{
private:

	bool targetReachableByTopSort(LearningNet &net,
			const std::map<int, std::string> &branchCombination)
	{
		bool targetReachable = false;

		std::deque<lemon::ListDigraph::Node> sources;

		// Collect nodes with indegree 0.
		for (lemon::ListDigraph::NodeIt v(net); v != lemon::INVALID; ++v) {
			if (net.isSource(v)) {
				sources.push_back(v);
			}
			if (net.isJoin(v)) {
				net.resetActivatedInArcs(v);
			}
		}

		// Function to push an arc's target to sources.
		auto exploreArc = [&](const lemon::ListDigraph::OutArcIt &a) {
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
		};

		while (!sources.empty()) {
			lemon::ListDigraph::Node v = sources.back();
			sources.pop_back();

			if (v == net.getTarget()) {
				targetReachable = true;
				break;
			} else if (net.isCondition(v)) { // target not yet reachable
				// Condition: only visit branch given by branchCombination.
				std::string branch = branchCombination.at(net.getConditionId(v));
				bool explored = false;
				lemon::ListDigraph::OutArcIt elseBranch;
				for (lemon::ListDigraph::OutArcIt a(net, v); a != lemon::INVALID; ++a) {
					if (net.getConditionBranch(a) == branch) {
						explored = true;
						exploreArc(a);
					} else if (net.getConditionBranch(a) == CONDITION_ELSE_BRANCH_KEYWORD) {
						 elseBranch = a;
					}
				}

				// If no other branch was visited, the else-branch is explored.
				if (!explored) {
					exploreArc(elseBranch);
				}
			} else {
				// Non-Condition: Push all successors (unless it is still a locked join).
				for (lemon::ListDigraph::OutArcIt a(net, v); a != lemon::INVALID; ++a) {
					exploreArc(a);
				}
			}
		}

		return targetReachable;
	}

	bool pathsForAllConditions(LearningNet &net,
			std::map<int, std::vector<std::string>> &conditionIdToBranches)
	{
		// Start with first branch for every condition.
		// conditionId -> (iterator in conditionIdToBranches->second)
		std::map<int, std::vector<std::string>::iterator> branchIterators;
		for (auto branches : conditionIdToBranches) {
			branchIterators[std::get<0>(branches)] =
				std::get<1>(branches).begin();
		}
		auto conditionIt = conditionIdToBranches.begin();

		while (conditionIt != conditionIdToBranches.end()) {
			// dereference iterators to get strings
			std::map<int, std::string> branchCombination;
			for (auto branches : conditionIdToBranches) {
				int conditionId = std::get<0>(branches);
				branchCombination[conditionId] = *branchIterators[conditionId];
			}

			if (!targetReachableByTopSort(net, branchCombination)) {
				failWithError("No path to target for condition branches:");
				for (auto branches : conditionIdToBranches) {
					int conditionId = std::get<0>(branches);
					appendError(std::to_string(conditionId) + ": " +
							branchCombination[conditionId]);
				}
				return false;
			}

			// TODO really combination
			// Get next combination of condition branches:
			// Increment the branch of the current condition.
			//
			//
			/* while (it[0] != v[0].end()) { */
				// process the pointed-to elements

				// the following increments the "odometer" by 1
				/* ++it[K-1]; */
				/* for (int i = K-1; (i > 0) && (it[i] == v[i].end()); --i) { */
				/* 	it[i] = v[i].begin(); */
				/* 	++it[i-1]; */
				/* } */
			/* } */

			//
			//
			auto &branchIt = branchIterators[std::get<0>(*conditionIt)];
			auto branches = std::get<1>(*conditionIt);
			branchIt++;
			// If the iterator reach the end of the branches for this condition,
			// reset branches for all conditions until the current one,
			// increment branch for next condition.
			if (branchIt == branches.end()) {
				conditionIt++;
				auto prevConditionIt = conditionIdToBranches.begin();
				while (std::get<0>(*prevConditionIt) != std::get<0>(*conditionIt)) {
					branchIterators[std::get<0>(*prevConditionIt)] =
						std::get<1>(*prevConditionIt).begin();
					prevConditionIt++;
				}
				branchIterators[std::get<0>(*conditionIt)]++;
			}
		}

		return true;
	}

#if 0
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
#endif

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
				for (lemon::ListDigraph::OutArcIt out(net, v); out != lemon::INVALID; ++out) {
					std::string conditionBranch = net.getConditionBranch(out);
					if (conditionBranch == CONDITION_ELSE_BRANCH_KEYWORD) {
						elseBranchFound = true;
					} else {
						int conditionId = net.getConditionId(v);
						if (conditionIdToBranches.find(conditionId) == conditionIdToBranches.end()) {
							std::vector<std::string> branches;
							conditionIdToBranches[conditionId] = branches;
						}
						conditionIdToBranches[conditionId].push_back(conditionBranch);
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
		/* compress(net); */
		return pathsForAllConditions(net, conditionIdToBranches);
	}
};

}
