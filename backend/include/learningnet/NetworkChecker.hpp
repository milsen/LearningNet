#pragma once

#include <learningnet/Compressor.hpp>
#include <learningnet/Module.hpp>
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
		Compressor comp;
		return comp.compress(net) || pathsForAllConditions(net, conditionIdToBranches);
	}
};

}
