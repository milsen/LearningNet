#include <catch.hpp>
#include <iomanip>
#include <sstream>
#include <random>
#include <algorithm>
#include <learningnet/LearningNet.hpp>

using namespace learningnet;

const std::string instancePath = "../test/resources/instances/";

// unit node with probability pUnitNode = 0.5
// join node with probability (1 - pUnitNode) / 2 = 0.25
// split node with probability ((1 - pUnitNode) * (3/8)) = 0.1875
// test node with probability ((1 - pUnitNode) * (1/8)) = 0.0625
static double pUnitNode = 0.5;
static double pOtherType = 1.0 - pUnitNode;
static double pJoin = pOtherType / 2.0;
static double pSplitLike = pOtherType / 2.0;
static double pSplit = pSplitLike * 0.75;
static double pTest = pSplitLike * 0.25;

static double thresholdUnit = pUnitNode;
static double thresholdJoin = thresholdUnit + pJoin;
static double thresholdSplit = thresholdJoin + pSplit;
// thresholdSplit + pTest = 1

int randomNumber(std::default_random_engine &rng, int low, int high)
{
	std::uniform_int_distribution<int> intDist(low, high);
	return intDist(rng);
}

void randomDAG(LearningNet &net, int n, int nCondition, double p) {
	std::default_random_engine rng;
	std::uniform_real_distribution<double> realDist(0.0, 1.0);

	std::vector<lemon::ListDigraph::Node> nodes{n};
	// Create nCondition condition nodes.
	for (int i = 0; i < nCondition; i++) {
		nodes[i] = net.addNode();
		net.setType(nodes[i], NodeType::condition);
	}

	// Create other nodes with types according to probabilities.
	for (int i = nCondition; i < n; i++) {
		nodes[i] = net.addNode();

		double rand = realDist(rng);
		if (rand < thresholdUnit) { // unit node
			net.setType(nodes[i], NodeType::inactive);
		} else if (rand < thresholdJoin) { // join node
			net.setType(nodes[i], NodeType::join);
		} else if (rand < thresholdSplit) { // split node
			net.setType(nodes[i], NodeType::split);
		} else { // test node
			net.setType(nodes[i], NodeType::test);
		}
	}

	// Permute nodes.
	std::shuffle(std::begin(nodes), std::end(nodes), rng);

	// Create edges.
	for (int i = 0; i < n; i++) {
		auto v = nodes[i];

		// Only add edges to later nodes such graph is acyclic.
		for (int j = i+1; j < n; j++) {
			auto w = nodes[j];

			// Create an edge if probablity says so and it is possible to add
			// an edge. Only create three outArcs for conditions.
			// TODO may create less than 3 out-arcs for condition nodes.
			if (realDist(rng) < p && (net.isJoin(w) || countInArcs(net, w) <= 1)) {
				net.addArc(v, w);

				if (net.isUnit(v) || net.isJoin(v) ||
					(net.isCondition(v) && countOutArcs(net, v) >= 3)) {
					continue;
				}
			}
		}
	}

	// Set ref attribute and condition attribute according to node type.
	int section{0};
	int condId{0};
	for (auto v : net.nodes()) {
		if (net.isUnit(v)) {
			net.setSection(v, section++);
		} else if (net.isJoin(v)) {
			net.setNecessaryInArcs(v, randomNumber(rng, 0, countInArcs(net, v)));
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

	// Set last node as target since it is the one that most nodes depend on.
	net.setTarget(nodes[n-1]);
}

TEST_CASE("Generate DAG","[dag]") {
	for (int n = 10; n <= 1000; n++) {
		for (double p = 0.25; p < 0.51; p+=0.25) {
			for (int numConds = 0; numConds <= 5; numConds++) {
				std::ostringstream sstream;
				sstream << instancePath << "selfLN-"
					<< std::setfill('0') << std::setw(4) << n << "-"
					<< std::setprecision(2) << std::fixed << p << "-"
					<< numConds << ".lgf";
				std::string instanceFile = sstream.str();

				SECTION(instanceFile) {
					LearningNet net;
					randomDAG(net, n, numConds, p);
					std::ofstream ofs{instanceFile};
					net.write(ofs);
				}
			}
		}
	}
}
