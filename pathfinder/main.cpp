#include <lemon/arg_parser.h>
#include <lemon/list_graph.h>
#include <lemon/concepts/digraph.h>
#include <lemon/lgf_reader.h>
#include <unordered_map>
#include <lemon/connectivity.h> // for dag
#include <learningnet/LearningNet.hpp>

using namespace lemon;
using namespace learningnet;

using Graph = ListDigraph;

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

bool setActivity(const std::string &network, const std::string &completed)
{
	// Read lemon graph file given as arg.
	std::istringstream networkIss(network);
	ListDigraph g;
	ListDigraph::NodeMap<int> section(g);
	ListDigraph::NodeMap<int> type(g);
	ListDigraph::Node tgt;

	try {
		DigraphReader<ListDigraph>(g, networkIss)
			.nodeMap("type", type)
			.nodeMap("section", section)
			.node("target", tgt)
			.run();
	} catch (Exception &e) {
		std::cerr << e.what() << std::endl;
		return false;
	}

	std::map<int, ListDigraph::Node> sectionNode;
	for (ListDigraph::NodeIt v(g); v != INVALID; ++v) {
		sectionNode[section[v]] = v;
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
		type[sectionNode[completedSection]] = 2;
	}

	setActivity(g, type);

	// Write lemon graph file to cout.
	DigraphWriter<ListDigraph>(g, std::cout)
		.nodeMap("type", type)
		.nodeMap("section", section)
		.node("target", tgt)
        .run();

	return true;
}

bool checkNetwork(const std::string &network)
{
	// Read lemon graph file given as arg.
	std::istringstream networkIss(network);
	ListDigraph g;
	ListDigraph::NodeMap<int> section(g);
	ListDigraph::NodeMap<int> type(g);
	ListDigraph::Node tgt;

	try {
		DigraphReader<ListDigraph>(g, networkIss)
			.nodeMap("type", type)
			.nodeMap("section", section)
			.node("target", tgt)
			.run();
	} catch (Exception &e) {
		std::cerr << e.what() << std::endl;
		return false;
	}

	// TODO when conditions exist, there must exist a path to target
	return dag(g);
}

/* createNetwork() */
/* { */

/* } */

int main(int argc, char *argv[])
{
	ArgParser ap(argc, argv);
	std::string network;
	std::string sections;
	std::string costs;

	// Set option that can be parsed.
	ap
		// possible actions (one and only one must be used)
		.boolOption("check", "Check whether the input is a correct learning net. Param: network")
		.boolOption("create", "Create a learning net without edges. Param: sections")
		.boolOption("active", "Output active units based on completed ones. Param: network, sections")
		.boolOption("recnext", "Output the recommended next unit. Param: network, costs")
		.boolOption("recpath", "Output the sequence of recommended units. Param: network, costs")
		.optionGroup("action", "check")
		.optionGroup("action", "create")
		.optionGroup("action", "active")
		.optionGroup("action", "recnext")
		.optionGroup("action", "recpath")
		.mandatoryGroup("action")
		.onlyOneGroup("action")

		// possible arguments
		.refOption("network", "Network as space-separated string.", network)
		.refOption("costs", "Costs as space-separated string.", costs) // TODO will not work
		.refOption("sections", "Relevant sections as space-separated string.", sections)

		// whether costs are for nodes or edges
		.boolOption("edgecosts", "Use costs for edges.")
		.boolOption("nodecosts", "Use costs for nodes.")
		.optionGroup("costtype", "edgecosts")
		.optionGroup("costtype", "nodecosts")
		.onlyOneGroup("costtype");

	// Throw an exception when problems occurs.
	// The default behavior is to exit(1) on these cases, but this makes
	// Valgrind falsely warn about memory leaks.
	ap.throwOnProblems();

	// Perform the parsing process
	// (in case of any error it terminates the program)
	// The try {} construct is necessary only if the ap.trowOnProblems()
	// setting is in use.
	try {
		ap.parse();
	} catch (ArgParserException &) {
		return EXIT_FAILURE;
	}

	// Check for correct parameters.
	if ((ap.given("check")   && !ap.given("network"))
	 || (ap.given("create")  && !ap.given("sections"))
	 || (ap.given("active")  && !(ap.given("network") && ap.given("sections")))
	 || (ap.given("recnext") && !(ap.given("network") && ap.given("costs")))
	 || (ap.given("recpath") && !(ap.given("network") && ap.given("costs")))) {
		std::cerr << "Not all necessary parameters for the given action found." << std::endl;
		return EXIT_FAILURE;
	}

	if (ap.given("network")) {
		LearningNet net(network);
	}

	try {
		// Execute action.
		if (ap.given("check")) {
			return checkNetwork(network);
		} else if (ap.given("create")) {
			/* return createNetwork(sections); */
			return EXIT_FAILURE;
		} else if (ap.given("active")) {
			return setActivity(network, sections);
		} else if (ap.given("recnext")) {
			return EXIT_FAILURE;
		} else if (ap.given("recpath")) {
			return EXIT_FAILURE;
		} else {
			std::cerr << "No action given." << std::endl;
			return EXIT_FAILURE;
		}
	} catch (Exception &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
