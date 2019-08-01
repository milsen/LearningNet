#pragma once

#include <learningnet/Module.hpp>
#include <learningnet/LearningNet.hpp>

namespace learningnet {

class ActivitySetter : public Module
{
public:
	ActivitySetter(LearningNet &net, const std::string &completed) : Module() {
		call(net, completed);
	}

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
				case NodeType::completed:
				case NodeType::split:
				case NodeType::join:
					// For all outedges (in the completed case: only one).
					for (lemon::ListDigraph::OutArcIt a(net, v); a != lemon::INVALID; ++a) {
						lemon::ListDigraph::Node u = net.target(a);
						// Only join nodes can have multiple inedges.
						// Push those to sources once alle necessary inedges were activated.
						// TODO: changing directly number of howMany of join-nodes
						// in type might not be good
						if (net.isJoin(u) && net.necessaryInArcs(u) > 0) {
							net.setType(u, net.getType(u) - 1);
						}
						if (net.getType(u) <= NodeType::join) {
							sources.push_back(u);
						}
					}
					break;
				default:
					appendError("Input has nodes of unknown type.");
					break;
			}
		}
	}

	void call(LearningNet &net, const std::string &completed)
	{
		std::map<int, lemon::ListDigraph::Node> sectionNode;
		for (lemon::ListDigraph::NodeIt v(net); v != lemon::INVALID; ++v) {
			if (net.isUnit(v)) {
				sectionNode[net.getSection(v)] = v;
			}
		}

		// Completed units are given by string, which is split using vector c'tor.
		std::istringstream completedIss(completed);
		std::vector<std::string> completedSections(
			std::istream_iterator<std::string>{completedIss},
			std::istream_iterator<std::string>()
		);

		// Set type of completed units.
		for (std::string str : completedSections) {
			int completedSection = std::stoi(str);
			auto completedNode = sectionNode.find(completedSection);
			// Only set it for units, not for connectives!
			if (completedNode != sectionNode.end()) {
				net.setType(completedNode->second, NodeType::completed);
			} else {
				appendError("Could not find section " + str + ".");
			}
		}

		call(net);
	}
};

}
