#pragma once

#include <learningnet/Module.hpp>
#include <learningnet/LearningNet.hpp>

namespace learningnet {

class ActivitySetter : public Module
{
public:
	ActivitySetter(LearningNet &net, const std::vector<int> &completed) : Module() {
		call(net, completed);
	}

	// TDOO remove this unused function
	void topologicalSort(const lemon::ListDigraph &g, lemon::ListDigraph::NodeMap<int> &type)
	{
		std::vector<lemon::ListDigraph::Node> sources;
		lemon::ListDigraph::NodeMap<int> indeg(g);

		// Get indegree for each node.
		for (lemon::ListDigraph::ArcIt a(g); a != lemon::INVALID; ++a) {
			indeg[g.target(a)]++;
		}

		// Collect nodes with indegree 0.
		for (lemon::ListDigraph::NodeIt v(g); v != lemon::INVALID; ++v) {
			if (indeg[v] == 0) {
				sources.push_back(v);
			}
		}

		int count = 0;
		while (!sources.empty()) {
			lemon::ListDigraph::Node v = sources.back();
			sources.pop_back();
			type[v] = count++;

			// For all outedges:
			for (lemon::ListDigraph::OutArcIt a(g, v); a != lemon::INVALID; ++a) {
				lemon::ListDigraph::Node u = g.target(a);
				if (--indeg[u] == 0) {
					sources.push_back(u);
				}
			}
		}
	}

	void call(LearningNet &net)
	{
		// sources = actives
		std::vector<lemon::ListDigraph::Node> sources;

		// Collect nodes with indegree 0.
		for (lemon::ListDigraph::NodeIt v(net); v != lemon::INVALID; ++v) {
			if (net.isJoin(v)) {
				net.resetActivatedInArcs(v);
			}

			if (((net.isUnit(v) || net.isSplit(v)) && countInArcs(net, v) == 0)
			   || net.isUnlockedJoin(v)) {
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
					if (!net.isUnit(v) && !net.isSplit(v) && !net.isJoin(v)) {
						appendError("Input has nodes of unknown type.");
						break;
					}

					// For all outedges (in the completed case: only one).
					for (lemon::ListDigraph::OutArcIt a(net, v); a != lemon::INVALID; ++a) {
						lemon::ListDigraph::Node u = net.target(a);
						// Only join nodes can have multiple inedges.
						// Push those to sources once alle necessary inedges were activated.
						// TODO: changing directly number of howMany of join-nodes
						// in type might not be good
						if (net.isJoin(u)) {
							net.incrementActivatedInArcs(u);
						}
						if (!net.isJoin(u) || net.isUnlockedJoin(u)) {
							sources.push_back(u);
						}
					}
					break;
			}
		}
	}

	void call(LearningNet &net, const std::vector<int> &completed)
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

		call(net);
	}
};

}
