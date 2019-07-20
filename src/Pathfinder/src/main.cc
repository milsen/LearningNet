#include <lemon/list_graph.h>
#include <lemon/concepts/digraph.h>
#include <lemon/lgf_reader.h>

using namespace lemon;

int main(int argc, char *argv[])
{
	// Read lemon graph file given as arg.
	std::istringstream iss(argv[1]);
	ListDigraph g;
	ListDigraph::NodeMap<int> type(g);
	ListDigraph::NodeMap<double> vCost(g);
	ListDigraph::ArcMap<double> eCost(g);
	ListDigraph::Node tgt;

	DigraphReader<ListDigraph>(g, iss)
		.nodeMap("type", type)
		.nodeMap("cost", vCost)
		.arcMap("cost", eCost)
		.node("target", tgt)
		.run();

	// Write lemon graph file to cout.
	DigraphWriter<ListDigraph>(g, std::cout)
		.nodeMap("type", type)
		.nodeMap("cost", vCost)
		.arcMap("cost", eCost)
		.node("target", tgt)
        .run();
}
