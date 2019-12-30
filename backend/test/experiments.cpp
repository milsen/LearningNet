#include <catch.hpp>
#include <learningnet/NetworkChecker.hpp>
#include <learningnet/Recommender.hpp>
#include <learningnet/Compressor.hpp>
#include <learningnet/LearningNet.hpp>
#include <experimental/filesystem>
#include <iostream>
#include <fstream>
#include <chrono>
#include <random>

using namespace learningnet;

const std::string resourcePath = "../test/resources/";
const std::string instancePath = "../test/resources/instances/";
const std::string outputPath = "../test/output/";

using DoFunc = std::function<
	std::chrono::time_point<std::chrono::system_clock>(
		std::ofstream &out, LearningNet&
	)
>;

enum class RecType {
	active,
	next,
	path
};

enum class CostType {
	node,
	nodepair
};


/** Helpers **/
int getMilliSeconds(
		const std::chrono::time_point<std::chrono::system_clock> &start,
		const std::chrono::time_point<std::chrono::system_clock> &end)
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

void getCosts(LearningNet &net, NodeCosts &costs) {
	std::uniform_real_distribution<double> unif(0, 100);
	std::default_random_engine rand;
	for (auto v : net.nodes()) {
		if (net.isUnit(v)) {
			costs[v] = unif(rand);
		}
	}
}

void getCosts(LearningNet &net, NodePairCosts &costs) {
	std::uniform_real_distribution<double> unif(0, 100);
	std::default_random_engine rand;
	for (auto v : net.nodes()) {
		if (net.isUnit(v)) {
			for (auto w : net.nodes()) {
				if (net.isUnit(w)) {
					costs[v][w] = unif(rand);
				}
			}
		}
	}
}

template<typename Costs>
void prepareNet(LearningNet &net,
	ConditionMap &conditionVals,
	TestMap &testGrades,
	Costs &costs)
{
	for (auto v : net.nodes()) {
		if (net.isCondition(v)) {
			lemon::ListDigraph::OutArcIt a(net, v);
			std::vector<std::string> branches = {net.getConditionBranch(a)};
			unsigned int conditionId = net.getConditionId(v);
			if (conditionId >= conditionVals.size()) {
				conditionVals.resize(conditionId + 1);
			}
			conditionVals[conditionId] = branches;
		}
		if (net.isTest(v)) {
			lemon::ListDigraph::OutArcIt a(net, v);
			testGrades[net.getTestId(v)] = std::stoi(net.getConditionBranch(a));
		}
	}

	getCosts(net, costs);
}

long long getConditionBranches(const LearningNet &net)
{
	std::map<int, std::vector<std::string>> conditionIdToBranches;
	for (auto v : net.nodes()) {
		if (net.isCondition(v)) {
			int conditionId = net.getConditionId(v);

			for (auto out : net.outArcs(v)) {
				if (conditionIdToBranches.find(conditionId) ==
					conditionIdToBranches.end()) {
					std::vector<std::string> branches;
					conditionIdToBranches[conditionId] = branches;
				}
				conditionIdToBranches[conditionId].push_back(
					net.getConditionBranch(out)
				);
			}
		}
	}

	// Make collected condition branches unique.
	for (auto idToBranches : conditionIdToBranches) {
		std::vector<std::string> branches = std::get<1>(idToBranches);
		auto last = std::unique(branches.begin(), branches.end());
		branches.erase(last, branches.end());
		conditionIdToBranches[std::get<0>(idToBranches)] = branches;
	}

	long long result = 1;
	for (auto idToBranches : conditionIdToBranches) {
		result *= conditionIdToBranches[std::get<0>(idToBranches)].size();
	}

	return result;
}

void instanceTests(std::ofstream &out, const LearningNet &net, const std::string &prefix = "") {
	out << prefix << "condition combinations," << getConditionBranches(net) << std::endl;
}



/** Specialized test functions **/
void checkTest(std::ofstream &out, LearningNet &net, bool useCompression) {
	NetworkChecker checker(net, useCompression);

	auto oldTime = checker.m_startTime;
	for (auto p : checker.m_timePoints) {
		auto newTime = std::get<1>(p);
		int time = getMilliSeconds(oldTime, newTime);
		out << std::get<0>(p) << " (ms)," << time << "\n";
		oldTime = newTime;
	}
	out << "result," << checker.succeeded() << "\n";
	std::string error = checker.getError();
	std::replace(error.begin(), error.end(), '\n', '_');
	out << "error, " << error << "\n";
}


template<typename Costs>
void recTest(std::ofstream &out,
	LearningNet &net,
	const ConditionMap &conditionVals,
	const TestMap &testGrades,
	const Costs &costs,
	RecType recType)
{
	Recommender rec(net, conditionVals, testGrades);

	if (recType == RecType::active) {
		std::vector<lemon::ListDigraph::Node> actives = rec.recActive();
		out << "size," << actives.size() << "\n";
	} else if (recType == RecType::next) {
		/* std::vector<lemon::ListDigraph::Node>::const_iterator recommended = */ rec.recNext(costs);
	} else if (recType == RecType::path) {
		std::vector<lemon::ListDigraph::Node> learningPath = rec.recPath(costs);
		out << "size," << learningPath.size() << "\n";
	} else {
		std::cout << "OH NO! Unknown recType given." << std::endl;
	}
	std::string error = rec.getError();
	std::replace(error.begin(), error.end(), '\n', '_');
	out << "error, " << error << "\n";
	// TODO more output during path computation?
}


/** General test function **/
void testFile(const std::experimental::filesystem::path &filePath,
		const std::string &algName,
		const DoFunc &func)
{
	SECTION(filePath) {
		std::string instanceName = filePath.filename();
		std::string outputFilename = outputPath + algName + "_" + instanceName + ".csv";
		std::ofstream out(outputFilename);

		// Read file.
		std::ifstream f(filePath);
		CHECK(f);
		std::ostringstream netss;
		netss << f.rdbuf();

		// Build learning net.
		LearningNet net{netss.str()};

		// Execute function.
		// Preparation of condition/test values and costs for learning net is
		// done by this function until the returned time point.
		func(out, net);
	}
}

void for_each_file(const std::string &subdir,
		const std::string &algName,
		const DoFunc &func)
{
	for (const auto &entry : std::experimental::filesystem::directory_iterator(instancePath + subdir)) {
		testFile(entry.path(), algName, func);
	}
}


template<typename Costs>
void recTest(const std::string &costStr, Costs &costs, const std::string &instanceType)
{
	const std::map<RecType, std::string> recToString = {
		{RecType::active, "active"},
		{RecType::next, "next"},
		{RecType::path, "path"}
	};

	SECTION(costStr) {
		for (RecType recType : {RecType::active, RecType::next, RecType::path}) {
			std::string recTypeStr = recToString.at(recType);
			SECTION(recTypeStr) {

				std::string algName = "Recommender-" + costStr + "-" + recTypeStr;
				for_each_file(instanceType, algName, [&](std::ofstream &out, LearningNet &net) {
					std::ostringstream oss;
					net.write(oss);
					LearningNet netCopy{oss.str()};
					NetworkChecker checker{netCopy};
					bool valid = checker.succeeded();
					out << "valid," << valid << std::endl;

					auto afterPrepare = std::chrono::system_clock::now();
					if (valid) {
						ConditionMap conditionVals;
						TestMap testGrades;
						Costs costs;
						prepareNet(net, conditionVals, testGrades, costs);

						instanceTests(out, net);

						afterPrepare = std::chrono::system_clock::now();
						recTest(out, net, conditionVals, testGrades, costs, recType);
					}
					return afterPrepare;
				});

			}
		}
	}
}

void recTest(const std::string &instanceType) {
	// dimension costFunction: node / nodepair
	// dimension recType: active / next / path
	// Creates files of the form:
	// Recommender-costFunction-recType_instance-numberconditions-numbertests

	NodeCosts ncosts;
	recTest("node", ncosts, instanceType);

	NodePairCosts npcosts;
	recTest("nodepair", npcosts, instanceType);
}

void checkTest(const std::string &instanceType) {
	// Creates files of the form:
	// NetworkChecker-comp_instance
	// NetworkChecker-nocomp_instance

	for (bool useCompression : {false}) {
		std::string compressionStr = useCompression ? "comp" : "nocomp";
		std::string algName = "NetworkChecker-" + compressionStr;

		SECTION(compressionStr) {
			for_each_file(instanceType, algName, [&](std::ofstream &out, LearningNet &net) {
				instanceTests(out, net);
				auto afterPrepare = std::chrono::system_clock::now();
				Compressor comp{net};
				instanceTests(out, net, "end ");
				return afterPrepare;
			});
		}
	}
}


TEST_CASE("check selfLN", "[selfLN][check]") {
	checkTest("selfLN");
}

TEST_CASE("rec selfLN", "[selfLN][rec]") {
	recTest("selfLN");
}

TEST_CASE("check randomDag", "[randomDag][check]") {
	checkTest("randomDag");
}

TEST_CASE("rec randomDag", "[randomDag][rec]") {
	recTest("randomDag");
}

TEST_CASE("check north", "[north][check]") {
	checkTest("north");
}

TEST_CASE("rec north", "[north][rec]") {
	recTest("north");
}

TEST_CASE("check DAGmar", "[DAGmar][check]") {
	checkTest("DAGmar");
}

TEST_CASE("rec DAGmar", "[DAGmar][rec]") {
	recTest("DAGmar");
}
