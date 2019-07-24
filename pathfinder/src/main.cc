#include <lemon/list_graph.h>
#include <lemon/concepts/digraph.h>
#include <lemon/lgf_reader.h>
#include <unordered_map>

using namespace lemon;

using Graph = ListDigraph;

class Unit {
	private:
		int m_underlying;

	public:
		static constexpr int inactive = 0;
		static constexpr int active = 1;
		static constexpr int completed = 2;
		static constexpr int split = 10;
		static constexpr int join = 20;

		Unit(int v) : m_underlying(v) {}

		operator int() {
			return m_underlying;
		}
};

void topologicalSort(const ListDigraph &g, ListDigraph::NodeMap<int> &type)
{
	std::vector<ListDigraph::Node> sources;
	ListDigraph::NodeMap<int> indeg(g);

	// Get indegree for each node.
	for (ListDigraph::ArcIt a(g); a != INVALID; ++a) {
		indeg[g.target(a)]++;
	}

	// Collect nodes with indegree 0.
	for (ListDigraph::NodeIt v(g); v != INVALID; ++v) {
		if (indeg[v] == 0) {
			sources.push_back(v);
		}
	}

	int count = 0;
	while (!sources.empty()) {
		ListDigraph::Node v = sources.back();
		sources.pop_back();
		type[v] = count++;

		// For all outedges:
		for (ListDigraph::OutArcIt a(g, v); a != INVALID; ++a) {
			ListDigraph::Node u = g.target(a);
			if (--indeg[u] == 0) {
				sources.push_back(u);
			}
		}
	}
}

void setActivity(const ListDigraph &g, ListDigraph::NodeMap<int> &type)
{
	// sources = actives
	std::vector<ListDigraph::Node> sources;

	// Collect nodes with indegree 0.
	for (ListDigraph::NodeIt v(g); v != INVALID; ++v) {
		if (countInArcs(g, v) == 0) {
			sources.push_back(v);
		}
	}

	while (!sources.empty()) {
		ListDigraph::Node v = sources.back();
		sources.pop_back();
		switch (type[v]) {
			case Unit::inactive:
				type[v] = Unit::active;
				break;
			case Unit::active:
				std::cerr << "Input has active units set already. Why?" << std::endl;
				break;
			case Unit::completed:
			case Unit::split:
			case Unit::join:
				// For all outedges (in the completed case: only one).
				for (ListDigraph::OutArcIt a(g, v); a != INVALID; ++a) {
					ListDigraph::Node u = g.target(a);
					// Only join nodes can have multiple inedges.
					// Push those to sources once alle necessary inedges were activated.
					// TODO: changing directly number of howMany of join-nodes
					// in type might not be good
					if (type[u] <= Unit::join || --type[u] == Unit::join) {
						sources.push_back(u);
					}
				}
				break;
			default:
				std::cerr << "Input has nodes of unknown type." << std::endl;
				break;
		}
	}
}

int main(int argc, char *argv[])
{
	// Read lemon graph file given as arg.
	std::istringstream networkIss(argv[1]);
	ListDigraph g;
	ListDigraph::NodeMap<int> section(g);
	ListDigraph::NodeMap<int> type(g);
	/* ListDigraph::NodeMap<double> vCost(g); */
	/* ListDigraph::ArcMap<double> eCost(g); */
	ListDigraph::Node tgt;

	DigraphReader<ListDigraph>(g, networkIss)
		.nodeMap("type", type)
		.nodeMap("section", section)
		/* .nodeMap("cost", vCost) */
		/* .arcMap("cost", eCost) */
		.node("target", tgt)
		.run();

	std::map<int, ListDigraph::Node> sectionNode;
	for (ListDigraph::NodeIt v(g); v != INVALID; ++v) {
		sectionNode[section[v]] = v;
	}

	// Set type of completed units to 2.
	// Completed units are given by agv[2], string is split using vector c'tor.
	std::istringstream completedIss(argv[2]);
	std::vector<std::string> completedSections(
		std::istream_iterator<std::string>{completedIss},
		std::istream_iterator<std::string>()
	);
	for (std::string str : completedSections) {
		int completedSection = std::stoi(str);
		type[sectionNode[completedSection]] = 2;
	}

	setActivity(g, type);

	// TODO give out only active nodes, only type? use skipArcs
	// next line: next recommended item
	// next line: recommended path as list of nodes
	// Write lemon graph file to cout.
	DigraphWriter<ListDigraph>(g, std::cout)
		.nodeMap("type", type)
		.nodeMap("section", section)
		/* .skipArcs() */
		.node("target", tgt)
        .run();

	return EXIT_SUCCESS;
}
