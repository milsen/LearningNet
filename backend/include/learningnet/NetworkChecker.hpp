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

	bool m_useCompression;

	template<typename ArcItType>
	bool hasAtMostOneArc(const LearningNet &net,
			const lemon::ListDigraph::Node &v)
	{
		int count = 0;
		for (ArcItType a(net, v); a != lemon::INVALID; ++a) {
			if (++count > 1) {
				return false;
			}
		}
		return true;
	}


	bool targetReachableByTopSort(LearningNet &net,
			const std::map<int, std::string> &branchCombination)
	{
		bool targetReachable = false;

		// Collect sources: nodes with indegree 0.
		std::deque<lemon::ListDigraph::Node> sources;
		for (auto v : net.nodes()) {
			if (net.isJoin(v)) {
				net.resetActivatedInArcs(v);
			}

			if (net.isSource(v)) {
				sources.push_back(v);
			}
		}

		// Function to push an arc's target to sources.
		auto exploreArc = [&](const lemon::ListDigraph::OutArcIt &a) {
			lemon::ListDigraph::Node u = net.target(a);

			if (net.isJoin(u)) {
				net.incrementActivatedInArcs(u);
			}

			if (!net.isJoin(u) ||
				net.getActivatedInArcs(u) == net.getNecessaryInArcs(u)) {
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
				for (auto a : net.outArcs(v)) {
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
			} else if (net.isTest(v)) {
				// For a test one of the branches with the highest grade should
				// lead to the target.
				std::vector<lemon::ListDigraph::OutArcIt> highestGradeBranches;
				int maxGrade = -1;
				for (auto a : net.outArcs(v)) {
					int branchGrade = std::stoi(net.getConditionBranch(a));
					if (branchGrade >= maxGrade) {
						if (branchGrade > maxGrade) {
							maxGrade = branchGrade;
							highestGradeBranches.clear();
						}
						highestGradeBranches.push_back(a);
					}
				}

				for (auto a : highestGradeBranches) {
					exploreArc(a);
				}
			} else {
				// Non-Condition/non-test: Push all successors
				// (unless it is a locked join).
				for (auto a : net.outArcs(v)) {
					exploreArc(a);
				}
			}
		}

		return targetReachable;
	}

	void pathsForAllConditions(LearningNet &net,
			std::map<int, std::vector<std::string>> &conditionIdToBranches)
	{
		// Start with first branch for every condition.
		// conditionId -> (iterator in conditionIdToBranches->second)
		std::map<int, std::vector<std::string>::size_type> branchIndices;
		for (auto branches : conditionIdToBranches) {
			branchIndices[std::get<0>(branches)] = 0;
		}

		// Get first and last condition id in map.
		int firstId = std::get<0>(*(conditionIdToBranches.cbegin()));
		int lastId = std::get<0>(*(conditionIdToBranches.crbegin()));

		while (branchIndices[lastId] < conditionIdToBranches[lastId].size()) {
			std::map<int, std::string> branchCombination;
			// For each condition id: Dereference branch iterator to get string.
			for (auto branches : conditionIdToBranches) {
				int conditionId = std::get<0>(branches);
				int branchId = branchIndices[conditionId];
				branchCombination[conditionId] =
					conditionIdToBranches[conditionId][branchId];
			}

			if (!targetReachableByTopSort(net, branchCombination)) {
				failWithError("No path to target for condition branches:");
				for (auto branches : conditionIdToBranches) {
					int conditionId = std::get<0>(branches);
					appendError(std::to_string(conditionId) + ": " +
							branchCombination[conditionId]);
				}
				return;
			}

			// Get next combination of condition branches:
			// Increment the branch of the current condition.
			branchIndices[firstId]++;

			// Work similar to an odometer:
			// If the iterator reaches the end of the branches for this
			// condition, reset them and increment the branches for next one.
			// Repeat this process until one iterator does not reach the end for
			// the respective condition anymore.
			auto id = conditionIdToBranches.begin();
			int currentId = std::get<0>(*id);
			while (
				currentId != lastId &&
				branchIndices[currentId] == conditionIdToBranches[currentId].size())
			{
				branchIndices[currentId] = 0;
				id++;
				currentId = std::get<0>(*id);
				branchIndices[currentId]++;
			}
		}
	}


public:
	NetworkChecker(LearningNet &net, bool useCompression = true)
		: Module()
		, m_useCompression{useCompression}
	{
		call(net);
	}

	void call(LearningNet &net)
	{
		int conditionCount = 0;
		int testCount = 0;

		std::map<int, bool> sectionExists;
		for (auto v : net.nodes()) {
			// Check number of in-/out-arcs for each node-type.
			if (net.isUnit(v)) {
				int section = net.getSection(v);
				if (!hasAtMostOneArc<lemon::ListDigraph::OutArcIt>(net, v) ||
				    !hasAtMostOneArc<lemon::ListDigraph::InArcIt>(net, v)) {
					failWithError("Unit node of section " + std::to_string(section)
						+ " has more than one in-arc or out-arc.");
				}

				// Each section only occurs once.
				if (sectionExists[section]) {
					failWithError("Section " + std::to_string(section) + " used multiple times.");
					return;
				}
				sectionExists[section] = true;

			} else if (net.isJoin(v)) {
				if (net.isSource(v)) {
					failWithError("Join node has no in-arc.");
				}
				if (!hasAtMostOneArc<lemon::ListDigraph::OutArcIt>(net, v)) {
					failWithError("Join node has more than one out-arc.");
				}

				// Necessary inarcs of a join are less than actual InArcs.
				int necessaryInArcs = net.getNecessaryInArcs(v);
				int actualInArcs = countInArcs(net, v);
				if (necessaryInArcs == 0) {
					failWithError("Join node has necessary in-arcs set to 0.");
				}
				if (necessaryInArcs > actualInArcs) {
					failWithError("Join node has " + std::to_string(necessaryInArcs) +
						" necessary in-arcs but only " + std::to_string(actualInArcs) +
						" actual in-arcs.");
				}

			} else if (net.isSplit(v)) {
				if (!hasAtMostOneArc<lemon::ListDigraph::InArcIt>(net, v)) {
					failWithError("Split node has more than one in-arc.");
				}

			} else if (net.isCondition(v)) {
				conditionCount++;
				if (!hasAtMostOneArc<lemon::ListDigraph::InArcIt>(net, v)) {
					failWithError("Condition node has more than one in-arc.");
				}

				// Check that each condition has an else-branch (otherwise it
				// might not always be possible to reach the target).
				bool elseBranchFound = false;
				for (auto out : net.outArcs(v)) {
					std::string conditionBranch = net.getConditionBranch(out);
					if (conditionBranch == CONDITION_ELSE_BRANCH_KEYWORD) {
						elseBranchFound = true;
						break;
					}
				}
				if (!elseBranchFound) {
					failWithError("Condition has no else branch.");
				}

			} else if (net.isTest(v)) {
				testCount++;
				if (!hasAtMostOneArc<lemon::ListDigraph::InArcIt>(net, v)) {
					failWithError("Test node has more than one in-arc.");
				}
			} else {
				failWithError("Node of unknown type detected.");
			}
		}

		// If something went wrong already, return.
		if (!succeeded()) {
			return;
		}

		if (!dag(net)) {
			// Fail if the network is not acylic.
			failWithError("Given network is not acyclic.");
			return;
		} else if (conditionCount == 0 && testCount == 0) {
			// If there are no conditions/tests, the net is valid if acyclic.
			return;
		}

		// If compression should be used, compress the network.
		if (m_useCompression) {
			Compressor comp{net};
			if (comp.getResult() == TargetReachability::Yes) {
				return;
			}
			if (comp.getResult() == TargetReachability::No) {
				failWithError(comp.getError());
				return;
			}
		}

		// If there are no conditions but tests, run learning path search once.
		if (conditionCount == 0) {
			if (!targetReachableByTopSort(net, {})) {
				failWithError("The target cannot be reached when getting the "
					"highest grade in every test.");
			}
			return;
		}

		// Conditions exist, there must exist a path to target for each branch.
		// Collect all used branch values for each condition id, including
		// "else"-branches. These represent that none of the possible branches
		// for the given condition id is applicable for the user.
		std::map<int, std::vector<std::string>> conditionIdToBranches;
		for (auto v : net.nodes()) {
			if (net.isCondition(v)) {
				int conditionId = net.getConditionId(v);

				for (auto out : net.outArcs(v)) {
					if (conditionIdToBranches.find(conditionId) ==
						conditionIdToBranches.end()) {
						std::vector<std::string> branches;
						conditionIdToBranches[conditionId] = branches;
					}
					conditionIdToBranches[conditionId].push_back(
						net.getConditionBranch(out)
					);
				}
			}
		}

		// Make collected condition branches unique.
		for (auto idToBranches : conditionIdToBranches) {
			std::vector<std::string> branches = std::get<1>(idToBranches);
			auto last = std::unique(branches.begin(), branches.end());
			branches.erase(last, branches.end());
			conditionIdToBranches[std::get<0>(idToBranches)] = branches;
		}

		pathsForAllConditions(net, conditionIdToBranches);
	}
};

}
