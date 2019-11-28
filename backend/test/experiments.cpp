#include <catch.hpp>
#include <learningnet/NetworkChecker.hpp>
#include <learningnet/Recommender.hpp>
#include <learningnet/LearningNet.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <chrono>
#include <random>

using namespace learningnet;

const std::string resourcePath = "../test/resources/";
const std::string outputPath = "../test/output/";

/* using PrepareFunc = std::function<void(LearningNet&, ConditionMap &conditionVals, TestMap &testGrades, CostType &costs)>; */
/* using DoFunc = std::function<void(LearningNet&, const ConditionMap &conditionVals, const TestMap &testGrades, const CostType &costs)>; */

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

#if 0
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

	getCosts(net, costs);
}

template<typename Costs>
void prepareNet(LearningNet &net,
	ConditionMap &conditionVals,
	TestMap &testGrades,
	CostType &x)
{
	NodeCosts costs;
	prepareNet(net, conditionVals, testGrades, costs);
}

{
	NodePairCosts costs;
	prepareNet(net, conditionVals, testGrades, costs);
}
#endif


/** Specialized test functions **/
void checkTest(std::ofstream &out, LearningNet &net, bool useCompression) {
	NetworkChecker checker(net, useCompression);
	out << "result," << checker.succeeded() << "\n";
	// TODO more output during compression, amount of learning path searches etc.?
}


#if 0
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
		// TODO output size of vector ?
		out << "size," << actives.size() << "\n";
	} else if (recType == RecType::next) {
		std::vector<lemon::ListDigraph::Node>::const_iterator recommended = rec.recNext(costs);
		// TODO output
	} else if (recType == RecType::path) {
		std::vector<lemon::ListDigraph::Node> learningPath = rec.recPath(costs);
		// TODO output
	} else {
		std::cout << "OH NO! Unknown recType given." << std::endl;
	}
}
#endif


/** General test function **/
/* template<typename Costs> */
void testFile(const std::filesystem::path &filePath,
		const std::string &algName,
		const std::function<void(std::ofstream &out, LearningNet&)> &func)
{
	SECTION(filePath) {
		std::string instanceName = filePath.filename();
		std::string outputFilename = outputPath + algName + "_" + instanceName + ".csv";
		std::ofstream out(outputFilename);

		// Measure time.
		std::chrono::time_point<std::chrono::system_clock>
			start, afterFileRead, afterNetBuilt, end;
		start = std::chrono::system_clock::now();

		// Read file.
		std::ifstream f(filePath);
		CHECK(f);
		std::ostringstream netss;
		netss << f.rdbuf();
		afterFileRead = std::chrono::system_clock::now();

		// Build learning net.
		LearningNet net{netss.str()};
		afterNetBuilt = std::chrono::system_clock::now();

		// Prepare condition/test values and costs for learning net.
		/* prepareFunc(net, conditionVals, testGrades, costs); */

		// Execute function.
		// TODO time includes preparation of cost values and conditions/test
		// values for Recommender
		func(out, net);
		end = std::chrono::system_clock::now();

		// Write times.
		int readMs = getMilliSeconds(start, afterFileRead);
		int buildMs = getMilliSeconds(afterFileRead, afterNetBuilt);
		int doMs = getMilliSeconds(afterNetBuilt, end);
		int totalMs = getMilliSeconds(start, end);
		out << "read time (ms)," << readMs << std::endl;
		out << "build time (ms)," << buildMs << std::endl;
		out << "do time (ms)," << doMs << std::endl;
		out << "total time (ms)," << totalMs << std::endl;
	}
}

/* void for_each_file(const std::string &subdir, */
/* 		const std::string &algName, */
/* 		const PrepareFunc &prepareFunc, */
/* 		const DoFunc &func) */
/* { */

/* } */

void for_each_file(const std::string &subdir,
		const std::string &algName,
		const std::function<void(std::ofstream &out, LearningNet&)> &func)
{
	for (const auto &entry : std::filesystem::directory_iterator(resourcePath + subdir)) {
		testFile(entry.path(), algName, func);
	}
}

#if 0
void for_file(const std::string &subdir, const std::string &filename,
		const std::function<void(LearningNet&)> &func)
{
	testFile(resourcePath + subdir + "/" + filename + ".lgf", func);
}

#endif

#if 0
TEST_CASE("Recommender","[rec]") {
	// dimension costFunction: node / nodepair
	// dimension recType: active / next / path
	// Creates files of the form:
	// Recommender-costFunction-recType_instance-numberconditions-numbertests

	std::map<CostType, std::string> costToString = {
		{CostType::node, "node"},
		{CostType::nodepair, "nodepair"}
	};

	std::map<RecType, std::string> recToString = {
		{RecType::active, "active"},
		{RecType::next, "next"},
		{RecType::path, "path"}
	};


	for (CostType costType : {CostType::node, CostType::nodepair}) {
		std::string costStr = costToString[costType];
		SECTION(costStr) {

			for (RecType recType : {RecType::active, RecType::next, RecType::path}) {
				std::string recTypeStr = recToString[recType];
				SECTION(recTypeStr) {

					std::string algName = "Recommender-" + costStr + "-" + recTypeStr;
					for_each_file("instances", algName, [&](std::ofstream &out, LearningNet &net) {
						ConditionMap conditionVals;
						TestMap testGrades;
						Costs costs;
						prepareNet(net, conditionVals, testGrades, costs);
						recTest(out, net, conditionVals, testGrades, costs, recType);
					});

				}
			}

		}
	}
}
#endif

TEST_CASE("NetworkChecker","[check]") {
	// Creates files of the form:
	// NetworkChecker-comp_instance
	// NetworkChecker-nocomp_instance

	for (bool useCompression : {false, true}) {
		std::string compressionStr = useCompression ? "comp" : "nocomp";
		std::string algName = "NetworkChecker-" + compressionStr;

		SECTION(compressionStr) {
			for_each_file("instances", algName, [&](std::ofstream &out, LearningNet &net) {
				checkTest(out, net, useCompression);
			});
		}
	}
}
