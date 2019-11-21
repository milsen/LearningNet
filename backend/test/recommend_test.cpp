#include <catch.hpp>
#include "resources.hpp"
#include <random>
#include <learningnet/Recommender.hpp>

using namespace learningnet;

double getCost(const NodeCosts &costs,
		const lemon::ListDigraph::Node &prev,
		const lemon::ListDigraph::Node &v)
{
	return costs.at(v);
}

double getCost(const NodePairCosts &costs,
		const lemon::ListDigraph::Node &prev,
		const lemon::ListDigraph::Node &v)
{
	return costs.at(prev).at(v);
}

template<typename CostType>
void checkNet(LearningNet &net,
	const ConditionMap &conditionVals,
	const TestMap &testGrades,
	const CostType &costs)
{
	Recommender rec(net, conditionVals, testGrades);
	std::vector<lemon::ListDigraph::Node>::const_iterator recommended =
		rec.recNext(costs);

	for (auto v : rec.recActive()) {
		CHECK(net.getType(v) == NodeType::active);
	}

	std::vector<lemon::ListDigraph::Node> learningPath = rec.recPath(costs);

	// Check source.
	bool hasConnectiveSources = false;
	for (auto v : net.nodes()) {
		if (net.isSource(v) && !net.isUnit(v)) {
			hasConnectiveSources = true;
			break;
		}
	}
	if (!hasConnectiveSources) {
		CHECK(net.isSource(learningPath.front()));
	}

	// Check target.
	lemon::ListDigraph::Node last = learningPath.back();
	lemon::ListDigraph::Node tgt = net.getTarget();
	if (net.isUnit(tgt)) {
		CHECK(net.isTarget(last));
	}

	// The following check only works since recPath() uses a local greedy search.
	if (!learningPath.empty()) {
		CHECK(learningPath.front() == *recommended);
	}

	// Check whether iterative recommendation of active nodes could lead to the
	// learning path.
	lemon::ListDigraph::Node prev = lemon::INVALID;
	for (auto v : learningPath) {
		CHECK(net.isUnit(v));
		Recommender newRec(net, conditionVals, testGrades); // active nodes are set
		CHECK(net.getType(v) == NodeType::active);
		net.setType(v, NodeType::completed);

		// The following check only works since recPath() uses a local greedy search.
		// Ensure that v has the lowest cost out of all active nodes.
		for (auto activeW : newRec.recActive()) {
			CHECK(getCost(costs, prev, v) <= getCost(costs, prev, activeW));
		}
		prev = v;
	}
}

void checkNet(LearningNet &net) {
	std::uniform_real_distribution<double> unif(0, 100);
	std::default_random_engine rand;

	ConditionMap conditionVals;
	TestMap testGrades;
	for (auto v : net.nodes()) {
		if (net.isCondition(v)) {
			lemon::ListDigraph::OutArcIt a(net, v);
			std::vector<std::string> branches = {net.getConditionBranch(a)};
			unsigned int conditionId = net.getConditionId(v);
			if (conditionId > conditionVals.size()) {
				conditionVals.resize(conditionId + 1);
			}
			conditionVals[conditionId] = branches;
		}
		if (net.isTest(v)) {
			lemon::ListDigraph::OutArcIt a(net, v);
			testGrades[net.getTestId(v)] = std::stoi(net.getConditionBranch(a));
		}
	}

	SECTION("with node costs") {
		NodeCosts costs;
		for (auto v : net.nodes()) {
			if (net.isUnit(v)) {
				costs[v] = unif(rand);
			}
		}
		checkNet(net, conditionVals, testGrades, costs);
	}

	SECTION("with node pair costs") {
		NodePairCosts costs;
		for (auto v : net.nodes()) {
			if (net.isUnit(v)) {
				for (auto w : net.nodes()) {
					if (net.isUnit(w)) {
						costs[v][w] = unif(rand);
					}
				}
			}
		}
		checkNet(net, conditionVals, testGrades, costs);
	}
}

TEST_CASE("Recommender","[rec]") {
	for_each_file("valid", [&](LearningNet &net) {
		checkNet(net);
	});
}
