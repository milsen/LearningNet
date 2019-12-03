#pragma once

#include <learningnet/Compressor.hpp>
#include <learningnet/Module.hpp>
#include <lemon/connectivity.h> // for dag
#include <deque>
#include <chrono>

namespace learningnet {

using namespace lemon;

/**
 * Checks whether a given directed graph is a valid learning net, i.e. that it
 * has the basic necessary properties, is acyclic, and offers a learning path
 * for every learner.
 */
class NetworkChecker : public Module
{
private:

	//! Whether the graph is compressed before searching learning paths.
	bool m_useCompression;

	std::chrono::time_point<std::chrono::system_clock> m_startTime;

	/**
	 * @tparam ArcItType InArcIt or OutArcIt
	 * @param net the learning net containing \p v
	 * @param v the node
	 * @return whether \p v has at most one incoming or outgoing edge (depending
	 * on ArcItType)
	 */
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

	/**
	 * Executes a learning path search in \p net for a given value for each
	 * condition id.
	 *
	 * @param net the learning net
	 * @param branchCombination mapping from condition ids to condition values
	 * @return whether there exists a learning path in \p net for the condition
	 * values given by \p branchCombination
	 */
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
				for (auto a : net.outArcs(v)) {
					if (net.getConditionBranch(a) == MAX_GRADE) {
						exploreArc(a);
					}
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

	/**
	 * Executes a learning path search in \p net for each combination of
	 * condition values as given by a set of values for each condition id.
	 *
	 * @param net the learning net
	 * @param conditionIdToBranches mapping from condition ids to condition values
	 * @return whether there exists a learning path in \p net for each
	 * combination of condition values given by \p conditionIdToBranches
	 */
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

			auto now = std::chrono::system_clock::now();
			int timeSoFar = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_startTime).count();
			if (timeSoFar >= 600000) {
				failWithError("Timeout of 10 min reached.");
				return;
			}
		}
	}

	/**
	 * Verifies that the given directed graph has the basic properties of a
	 * learning net. Otherwise this NetworkChecker fails with an appropriate
	 * error message.
	 *
	 * @param net the (supposed) learning net
	 * @param conditionsExist is assigned whether \p net has condition nodes
	 * @param testsExist is assigned whether \p net has test nodes
	 */
	void basicChecks(const LearningNet &net,
			bool &conditionsExist,
			bool &testsExist)
	{
		conditionsExist = false;
		testsExist = false;

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
				conditionsExist = true;
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
				testsExist = true;
				if (!hasAtMostOneArc<lemon::ListDigraph::InArcIt>(net, v)) {
					failWithError("Test node has more than one in-arc.");
				}
			} else {
				failWithError("Node of unknown type detected.");
			}
		}
	}

	/**
	 * Collect all used condition values for each condition id and return them.
	 *
	 * This includes "else"-branches (these represent that none of the possible
	 * branches for the given condition id is applicable for the user).
	 *
	 * @param net learning net in which to collect condition branches
	 * @return map from condition ids to arrays of strings (condition values)
	 */
	std::map<int, std::vector<std::string>> getConditionBranches(
			const LearningNet &net) const
	{
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

		return conditionIdToBranches;
	}

	/**
	 * Calls this NetworkChecker for a learning net.
	 * If the check fails, this NetworkChecker fails with an appropriate error
	 * message.
	 *
	 * @param net the learning net
	 */
	void call(LearningNet &net)
	{
		bool conditionsExist = false;
		bool testsExist = false;
		basicChecks(net, conditionsExist, testsExist);

		// If something went wrong already, return.
		if (!succeeded()) {
			return;
		}

		if (!dag(net)) {
			// Fail if the network is not acylic.
			failWithError("Given network is not acyclic.");
			return;
		} else if (!conditionsExist && !testsExist) {
			// If there are no conditions/tests, the net is valid if acyclic.
			return;
		}

		// For test grades, set highest test grades to MAX_GRADE, others to 0.
		// This later simplifies checking whether a test grade is the highest.
		for (auto v : net.nodes()) {
			if (net.isTest(v)) {
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

					net.setConditionBranch(a, "0");
				}

				for (auto a : highestGradeBranches) {
					net.setConditionBranch(a, MAX_GRADE);
				}
			}
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

			// Update whether conditions and tests exist.
			conditionsExist = false;
			testsExist = false;
			for (auto v : net.nodes()) {
				if (net.isCondition(v)) {
					conditionsExist = true;
				}
				if (net.isTest(v)) {
					testsExist = true;
				}
				if (conditionsExist && testsExist) {
					break;
				}
			}
		}

		if (!conditionsExist) {
			if (testsExist) {
				// If there are no conditions but tests, run learning path
				// search once.
				if (!targetReachableByTopSort(net, {})) {
					failWithError("The target cannot be reached when getting "
						"the highest grade in every test.");
				}
			}
			// If there are no conditions or tests after compression, the graph
			// is a learning net. Return without failing.
			return;
		}

		// Conditions exist, there must exist a path to target for each branch.
		std::map<int, std::vector<std::string>> conditionBranches =
			getConditionBranches(net);
		pathsForAllConditions(net, conditionBranches);
	}

public:
	/**
	 * Creates a NetworkChecker and checks the given directed graph.
	 *
	 * @param net the learning net to check
	 * @param useCompression whether the graph should be compressed before
	 * searching learning paths
	 */
	NetworkChecker(LearningNet &net, bool useCompression = true)
		: Module()
		, m_useCompression{useCompression}
		, m_startTime{std::chrono::system_clock::now()}
	{
		call(net);
	}
};

}
