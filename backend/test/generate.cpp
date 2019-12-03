#include <catch.hpp>
#include <iomanip>
#include <sstream>
#include <random>
#include <algorithm>
#include <filesystem>
#include <learningnet/LearningNet.hpp>

using namespace learningnet;

const std::string resourcePath = "../test/resources/";
const std::string instancePath = "../test/resources/instances/";

// probabilities of node types
static double pUnitNode = 0.7;
static double pJoin = 0.15; // has no influence, completely depends on DAG
static double pSplit = 0.08;
static double pCondition = 0.02;
static double pTest = 0.06;
static double pConnective = pJoin + pSplit + pCondition + pTest;

enum class SplitTypes : int {
	Splits = 0,
	SplitsAndTests = 1,
	SplitsAndConditionsRare = 2,
	AllSplitLikesRare = 3,
	SplitsAndConditionsMedium = 4,
	AllSplitLikesMedium = 5,
	SplitsAndConditions = 6, // do not test!
	AllSplitLikes = 7, // do not test!
};

static int mediumVal = 12;
static int rareVal = 5;

/**
 * Creates a random directed acyclic graph.
 *
 * @param net is assigned the DAG
 * @param n number of nodes
 * @param p probability of a n edge between every pair of nodes where the second
 * node follows the first in a set random ordering of nodes
 */
void randomDAG(LearningNet &net, int n, double p) {
	std::default_random_engine rng;
	std::uniform_real_distribution<double> realDist(0.0, 1.0);

	std::vector<lemon::ListDigraph::Node> nodes{n};
	// Create nCondition condition nodes.
	for (int i = 0; i < n; i++) {
		nodes[i] = net.addNode();
	}

	// Create edges.
	for (int i = 0; i < n; i++) {
		auto v = nodes[i];

		// Only add edges to later nodes such graph is acyclic.
		for (int j = i+1; j < n; j++) {
			auto w = nodes[j];

			// Create an edge if probablity says so.
			if (realDist(rng) < p) {
				net.addArc(v, w);
			}
		}
	}
}

/**
 * @return a random int between \p low and \p high (inclusively)
 */
int randomNumber(std::default_random_engine &rng, int low, int high)
{
	std::uniform_int_distribution<int> intDist(low, high);
	return intDist(rng);
}

NodeType getSplitType(std::default_random_engine &rng, int outdeg, SplitTypes splitTypes)
{
	std::uniform_real_distribution<double> dist(0.0, pSplit + pCondition + pTest);
	double rand = dist(rng);
	switch (splitTypes) {
		case SplitTypes::Splits:
			return NodeType::split;
		case SplitTypes::SplitsAndTests:
			return rand < pSplit + pCondition ? NodeType::split : NodeType::test;
		case SplitTypes::SplitsAndConditions:
			return rand < pSplit + pTest ? NodeType::split : NodeType::condition;
		case SplitTypes::AllSplitLikes:
			if (rand < pSplit) {
				return NodeType::split;
			} else if (rand < pSplit + pCondition) {
				return NodeType::condition;
			} else {
				return NodeType::test;
			}
		case SplitTypes::SplitsAndConditionsRare:
			return outdeg <= rareVal ?
				getSplitType(rng, 0, SplitTypes::SplitsAndConditions) :
				getSplitType(rng, 0, SplitTypes::Splits);
		case SplitTypes::SplitsAndConditionsMedium:
			return outdeg <= mediumVal ?
				getSplitType(rng, 0, SplitTypes::SplitsAndConditions) :
				getSplitType(rng, 0, SplitTypes::Splits);
		case SplitTypes::AllSplitLikesRare:
			return outdeg <= rareVal ?
				getSplitType(rng, 0, SplitTypes::AllSplitLikes) :
				getSplitType(rng, 0, SplitTypes::SplitsAndTests);
		case SplitTypes::AllSplitLikesMedium:
			return outdeg <= mediumVal ?
				getSplitType(rng, 0, SplitTypes::AllSplitLikes) :
				getSplitType(rng, 0, SplitTypes::SplitsAndTests);
		default:
			std::cout << "Unknown SplitTypes?!" << std::endl;
			return NodeType::split;
	}
}


void learningNetFromDAG(LearningNet &net, SplitTypes splitTypes) {
	std::default_random_engine rng;

	// Set node types depending on in- and outdegress.
	for (auto v : net.nodes()) {
		int indeg = countInArcs(net, v);
		int outdeg = countOutArcs(net, v);

		if (indeg > 1 && outdeg > 1) {
			// Both in-arcs and out-arcs: Split into join node and split-like.
			auto w = net.split(v);
			net.setType(v, NodeType::join);
			net.setType(w, getSplitType(rng, outdeg, splitTypes));
		} else if (indeg > 1) {
			// More than one in-arcs but at most one out-arc: join.
			net.setType(v, NodeType::join);
		} else if (outdeg > 1) {
			// More than one out-arcs but at most one in-arc: split-like.
			net.setType(v, getSplitType(rng, outdeg, splitTypes));
		} else {
			// At most one in-arc and at most one out-arc: Unit node.
			net.setType(v, NodeType::inactive);
		}
	}

	// Get arcs and shuffle them.
	std::vector<lemon::ListDigraph::Arc> arcs;
	for (auto a : net.arcs()) {
		arcs.push_back(a);
	}
	std::shuffle(std::begin(arcs), std::end(arcs), rng);

	// The nodes so far are mostly connectives.
	// Insert unit nodes by splitting arcs.
	int nUnitNodes = (countNodes(net) * pUnitNode) / pConnective;
	for (int i = 0; i < nUnitNodes; ++i) {
		auto randArc = arcs[randomNumber(rng, 0, arcs.size()-1)];
		auto newNode = net.split(randArc);
		net.setType(newNode, NodeType::inactive);
		lemon::ListDigraph::OutArcIt out(net, newNode);
		arcs.push_back(out);
		std::shuffle(std::begin(arcs), std::end(arcs), rng);
	}

	// Set ref attribute and condition attribute according to node type.
	int section{0};
	int condId{0};
	for (auto v : net.nodes()) {
		if (net.isUnit(v)) {
			net.setSection(v, section++);
		} else if (net.isJoin(v)) {
			net.setNecessaryInArcs(v, randomNumber(rng, 1, countInArcs(net, v)));
		} else if (net.isCondition(v)) {
			// No condition id used twice, although that may occur in practive.
			net.setConditionId(v, condId++);
			int elseBranch = randomNumber(rng, 0, countOutArcs(net, v)-1);
			int count{0};
			for (auto a : net.outArcs(v)) {
				net.setConditionBranch(a, count++ == elseBranch ?
					CONDITION_ELSE_BRANCH_KEYWORD :
					std::to_string(randomNumber(rng, 0, 100))
				);
			}
		} else if (net.isTest(v)) {
			net.setTestId(v, randomNumber(rng, 0, 1000));
			for (auto a : net.outArcs(v)) {
				net.setConditionBranch(a, std::to_string(randomNumber(rng, 0, 100)));
			}
		}
	}

	// Set target to one without out-arcs if possible
	std::vector<lemon::ListDigraph::Node> noOutArcs;
	for (auto v : net.nodes()) {
		if (countOutArcs(net, v) == 0) {
			noOutArcs.push_back(v);
		}
	}
	std::shuffle(std::begin(noOutArcs), std::end(noOutArcs), rng);

	if (noOutArcs.empty()) {
		std::vector<lemon::ListDigraph::Node> nodes;
		for (auto v : net.nodes()) {
			nodes.push_back(v);
		}
		std::shuffle(std::begin(nodes), std::end(nodes), rng);

		net.setTarget(nodes[0]);
	} else {
		net.setTarget(noOutArcs[0]);
	}
}

void forAllSplitTypeConfigs(std::function<void(SplitTypes splitTypes)> func) {
	for (SplitTypes splitTypes : {
			SplitTypes::Splits,
			SplitTypes::SplitsAndTests,
			SplitTypes::SplitsAndConditionsRare,
			SplitTypes::AllSplitLikesRare,
			SplitTypes::SplitsAndConditionsMedium,
			SplitTypes::AllSplitLikesMedium}) {
		// Do not test AllSplitLikes and SplitsAndConditions directly, they
		// create conditions with too many branches!
		func(splitTypes);
	}
}

TEST_CASE("Generate selfLN","[selfLN]") {
	forAllSplitTypeConfigs([](SplitTypes splitTypes) {
		for (int n = 5; n <= 200; n++) {
			for (double p = 0.20; p < 0.81; p+=0.2) {
				std::ostringstream sstream;
				sstream << instancePath << "selfLN" << "-"
					<< std::setfill('0') << std::setw(4) << n << "-"
					<< std::setprecision(1) << std::fixed << p << "-"
					<< static_cast<int>(splitTypes) //split types
					<< ".lgf";
				std::string instanceFile = sstream.str();

				SECTION(instanceFile) {
					LearningNet net;
					randomDAG(net, n, p);
					learningNetFromDAG(net, splitTypes);
					std::ofstream ofs{instanceFile};
					net.write(ofs);
				}
			}
		}
	});
}

TEST_CASE("Generate randomDag","[randomDag]") {
	forAllSplitTypeConfigs([](SplitTypes splitTypes) {
		for (const auto &entry : std::filesystem::directory_iterator(resourcePath + "random-dag-lgf")) {
			std::ostringstream sstream;
			sstream << instancePath << "randomDag" << "-"
				<< entry.path().filename() << "-"
				<< static_cast<int>(splitTypes) //split types
				<< ".lgf";
			std::string instanceFile = sstream.str();

			SECTION(instanceFile) {
				// Read graph.
				lemon::ListDigraph g;
				lemon::DigraphReader<lemon::ListDigraph>(g, entry.path()).run();

				// Create net
				LearningNet net;
				lemon::ListDigraph::NodeMap<lemon::ListDigraph::Node> nodeMap{g};
				for (auto v : g.nodes()) {
					nodeMap[v] = net.addNode();
				}
				for (auto a : g.arcs()) {
					net.addArc(nodeMap[g.source(a)], nodeMap[g.target(a)]);
				}
				learningNetFromDAG(net, splitTypes);

				// Write to file.
				std::ofstream ofs{instanceFile};
				net.write(ofs);
			}
		}
	});
}
