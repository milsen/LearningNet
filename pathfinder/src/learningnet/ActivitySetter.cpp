#include <learningnet/ActivitySetter.hpp>

namespace learningnet {

void ActivitySetter::call(LearningNet &net)
{
	// sources = actives
	std::vector<ListDigraph::Node> sources;

	// Collect nodes with indegree 0.
	for (ListDigraph::NodeIt v(net); v != INVALID; ++v) {
		if (countInArcs(net, v) == 0) {
			sources.push_back(v);
		}
	}

	while (!sources.empty()) {
		ListDigraph::Node v = sources.back();
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
				for (ListDigraph::OutArcIt a(net, v); a != INVALID; ++a) {
					ListDigraph::Node u = net.target(a);
					// Only join nodes can have multiple inedges.
					// Push those to sources once alle necessary inedges were activated.
					// TODO: changing directly number of howMany of join-nodes
					// in type might not be good
					if (net.isJoin(u)) {
						net.setType(u, net.getType(u) - 1);
					}
					if (net.getType(u) <= NodeType::join) {
						sources.push_back(u);
					}
				}
				break;
			default:
				appendError("Input has nodes of unknown type.";
				break;
		}
	}
}

void ActivitySetter::call(LearningNet &net, const std::string &completed)
{
	std::map<int, ListDigraph::Node> sectionNode;
	for (ListDigraph::NodeIt v(net); v != INVALID; ++v) {
		sectionNode[net.getSection(v)] = v;
	}

	// Set type of completed units to 2.
	// Completed units are given by agv[2], string is split using vector c'tor.
	std::istringstream completedIss(completed);
	std::vector<std::string> completedSections(
		std::istream_iterator<std::string>{completedIss},
		std::istream_iterator<std::string>()
	);
	for (std::string str : completedSections) {
		int completedSection = std::stoi(str);
		net.setType(sectionNode[completedSection], 2);
	}

	setActivity(net);
}

}
