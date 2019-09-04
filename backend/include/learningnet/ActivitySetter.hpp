#pragma once

#include <learningnet/Module.hpp>
#include <learningnet/LearningNet.hpp>
#include <map>

namespace learningnet {

using ConditionMap = std::vector<std::vector<std::string>>;

class ActivitySetter : public Module
{
public:
	ActivitySetter(
		LearningNet &net,
		const std::vector<int> &completed,
		const ConditionMap &conditionVals)
	: Module() {
		call(net, completed, conditionVals);
	}

	void call(LearningNet &net, const ConditionMap &conditionVals)
	{
		// sources = actives
		std::vector<lemon::ListDigraph::Node> sources;

		// Collect nodes with indegree 0.
		for (lemon::ListDigraph::NodeIt v(net); v != lemon::INVALID; ++v) {
			if (net.isJoin(v)) {
				net.resetActivatedInArcs(v);
			}

			if (countInArcs(net, v) == 0) {
				sources.push_back(v);
			}
		}

		while (!sources.empty()) {
			lemon::ListDigraph::Node v = sources.back();
			sources.pop_back();
			switch (net.getType(v)) {
				case NodeType::inactive:
					net.setType(v, NodeType::active);
					break;
				case NodeType::active:
					appendError("Input has active units set already. Why?");
					break;
				default:
					if (net.isUnknown(v)) {
						appendError("Input has nodes of unknown type.");
						break;
					}

					// Function to push an arc's target to sources.
					auto exploreArc = [&](const lemon::ListDigraph::OutArcIt &a) {
						lemon::ListDigraph::Node u = net.target(a);
						// Push join nodes only if all necessary in-edges are
						// activated. All other nodes only have one in-edge and
						// can be pushed directly when explored.
						if (net.isJoin(u)) {
							net.incrementActivatedInArcs(u);
						}
						if (!net.isJoin(u) || net.isUnlockedJoin(u)) {
							sources.push_back(u);
						}
					};

					// For a condition, only explore out-edges corresponding to set user-values.
					if (net.isCondition(v)) {
						// Get user values for this condition.
						std::vector<std::string> vals = conditionVals[net.getConditionId(v)];
						if (vals.empty()) {
							vals.push_back(CONDITION_ELSE_BRANCH_KEYWORD);
						}

						for (lemon::ListDigraph::OutArcIt a(net, v); a != lemon::INVALID; ++a) {
							if (std::find(vals.begin(), vals.end(), net.getConditionBranch(a)) != vals.end()) {
								exploreArc(a);
							}
						}
					} else {
						// Else explore all out-edges (for completed units: only one).
						for (lemon::ListDigraph::OutArcIt a(net, v); a != lemon::INVALID; ++a) {
							exploreArc(a);
						}
					}

					break;
			}
		}
	}

	void call(LearningNet &net, const std::vector<int> &completed, const ConditionMap &conditionVals)
	{
		std::map<int, lemon::ListDigraph::Node> sectionNode;
		for (lemon::ListDigraph::NodeIt v(net); v != lemon::INVALID; ++v) {
			if (net.isUnit(v)) {
				sectionNode[net.getSection(v)] = v;
			}
		}

		// Set type of completed units.
		for (int completedSection : completed) {
			auto completedNode = sectionNode.find(completedSection);
			// Only set it for units, not for connectives!
			if (completedNode != sectionNode.end()) {
				net.setType(completedNode->second, NodeType::completed);
			} else {
				appendError("Could not find section " + std::to_string(completedSection) + ".");
			}
		}

		call(net, conditionVals);
	}
};

}
