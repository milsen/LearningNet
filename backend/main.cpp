#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <learningnet/NetworkChecker.hpp>
#include <learningnet/Recommender.hpp>

using namespace learningnet;
using namespace rapidjson;

bool hasCorrectArgs(const Document &d, const std::initializer_list<const char *> &args)
{
	std::map<std::string, std::function<bool(const Value&)>> typeFunc = {
		{ "action",        std::bind(&Value::IsString, std::placeholders::_1) },
		{ "network",       std::bind(&Value::IsString, std::placeholders::_1) },
		{ "recType",       std::bind(&Value::IsString, std::placeholders::_1) },
		{ "sections",      std::bind(&Value::IsArray, std::placeholders::_1) },
		{ "conditions",    std::bind(&Value::IsArray, std::placeholders::_1) },
		{ "testGrades",    std::bind(&Value::IsObject, std::placeholders::_1) },
		{ "nodeCosts",     std::bind(&Value::IsArray, std::placeholders::_1) },
		{ "nodePairCosts", std::bind(&Value::IsArray, std::placeholders::_1) }
	};

	bool result = true;
	for (auto arg : args) {
		result &= d.HasMember(arg) && typeFunc[arg](d[arg]);
	}

	return result;
}

std::vector<int> toIntVector(const Value &oldArr)
{
	std::vector<int> arr;
	for (auto &v : oldArr.GetArray()) {
		arr.push_back(std::stoi(v.GetString()));
	}

	return arr;
}

std::vector<std::string> toStringVector(const Value &oldArr)
{
	std::vector<std::string> arr;
	for (auto &v : oldArr.GetArray()) {
		arr.push_back(v.GetString());
	}

	return arr;
}

ConditionMap toConditionMap(const Value &oldArr)
{
	ConditionMap idToVal;
	for (auto &v : oldArr.GetArray()) {
		idToVal.push_back(toStringVector(v));
	}

	return idToVal;
}

TestMap toTestMap(const Value &oldObj)
{
	TestMap idToVal;
	for (auto &m : oldObj.GetObject()) {
		idToVal[std::stoi(m.name.GetString())] = std::stoi(m.value.GetString());
	}

	return idToVal;
}

NodeCosts toNodeCosts(const LearningNet &net,
	const Value &nodeCostArr)
{
	NodeCosts nodeCosts;

	// Get weight sum.
	double weightSum = 0.0;
	for (auto &val : nodeCostArr.GetArray()) {
		// TODO test whether object and both weight and cost exist
		weightSum += val.GetObject()["weight"].GetDouble();
	}

	for (auto v : net.nodes()) {
		if (net.isUnit(v)) {
			const char *vSection = std::to_string(net.getSection(v)).c_str();
			nodeCosts[v] = 0.0;

			for (auto &val : nodeCostArr.GetArray()) {
				double weight = val.GetObject()["weight"].GetDouble();
				auto &costDict = val.GetObject()["costs"];
				auto itr = costDict.FindMember(vSection);
				if (itr != costDict.MemberEnd()) {
					nodeCosts[v] += (itr->value.GetDouble() * weight) / weightSum;
				}
			}
		}
	}

	return nodeCosts;
}

NodePairCosts toNodePairCosts(const LearningNet &net,
	const Value &nodeCostArr,
	const Value &nodePairCostArr)
{
	NodePairCosts nodePairCosts;

	// Get weight sum.
	double weightSum = 0.0;
	for (auto &val : nodeCostArr.GetArray()) {
		// TODO test whether object and both weight and cost exist
		weightSum += val.GetObject()["weight"].GetDouble();
	}
	for (auto &val : nodePairCostArr.GetArray()) {
		// TODO test whether object and both weight and cost exist
		weightSum += val.GetObject()["weight"].GetDouble();
	}

	// Get weighted costs for each node pair.
	for (auto &v : net.nodes()) {
		if (net.isUnit(v)) {
			const char *srcSection = std::to_string(net.getSection(v)).c_str();

			for (auto &w : net.nodes()) {
				if (net.isUnit(w)) {
					const char *tgtSection = std::to_string(net.getSection(w)).c_str();
					nodePairCosts[v][w] = 0.0;

					for (auto &val : nodeCostArr.GetArray()) {
						double weight = val.GetObject()["weight"].GetDouble();
						auto &costDict = val.GetObject()["costs"];
						auto itr = costDict.FindMember(tgtSection);
						if (itr != costDict.MemberEnd()) {
							nodePairCosts[v][w] +=
								(itr->value.GetDouble() * weight) / weightSum;
						}
					}

					for (auto &val : nodePairCostArr.GetArray()) {
						double weight = val.GetObject()["weight"].GetDouble();
						auto &costDict = val.GetObject()["costs"];
						auto itr = costDict.FindMember(srcSection);
						if (itr != costDict.MemberEnd()) {
							auto itr2 = itr->value.FindMember(tgtSection);
							if (itr2 != itr->value.MemberEnd()) {
								nodePairCosts[v][w] +=
									(itr2->value.GetDouble() * weight) / weightSum;
							}
						}
					}
				}
			}
		}
	}

	return nodePairCosts;
}

int main(int argc, char *argv[])
{
	try {
		Document d;
		d.Parse(argv[1]);

		if (!hasCorrectArgs(d, {"action"})) {
			std::cout << "No action string given." << std::endl;
			return EXIT_FAILURE;
		}

		// Check for correct parameters.
		std::string action = d["action"].GetString();
		if ((action == "check"     && !hasCorrectArgs(d, {"network"}))
		 || (action == "create"    && !hasCorrectArgs(d, {"sections"}))
		 || (action == "recommend" && !hasCorrectArgs(d, {"recType","network","sections","conditions","testGrades"}))
		 ) {
			// TODO more info
			std::cout << "Not all necessary parameters for the given action found." << std::endl;
			return EXIT_FAILURE;
		}

		// TODO type checks for arrays/objects

		// Execute action.
		if (action == "check") {
			LearningNet net(d["network"].GetString());
			NetworkChecker checker(net);
			return checker.handleFailure();
		} else if (action == "create") {
			LearningNet *net = LearningNet::create(toIntVector(d["sections"]));
			net->write();
			delete net;
			return EXIT_SUCCESS;
		} else if (action == "recommend") {
			std::string recType = d["recType"].GetString();
			bool hasActive = recType == std::string("active");
			bool hasNext   = recType == std::string("next");
			bool hasPath   = recType == std::string("path");
			bool hasNextOrPath = hasNext || hasPath;

			if (!hasActive && !hasNextOrPath) {
				std::cout << "No valid recommendation type (\"active\", "
				          << "\"next\" or \"path\") given even though "
				          << "the action is \"recommend\"." << std::endl;
				return EXIT_FAILURE;
			}

			bool hasNodePairCosts = hasCorrectArgs(d, {"nodePairCosts"}) && !d["nodePairCosts"].Empty();
			bool hasNodeCosts = hasCorrectArgs(d, {"nodeCosts"}) && !d["nodeCosts"].Empty();
			bool hasOnlyNodeCosts = hasNodeCosts && !hasNodePairCosts;

			if (hasNextOrPath && !hasNodeCosts && !hasNodePairCosts) {
				std::cout << "Not node or node pair costs given even though "
				          << "the recommendation type \"" << recType
				          << "\" requires it." << std::endl;
				return EXIT_FAILURE;
			}

			// Create LearningNet, set given sections as completed and pass the
			// net to the Recommender, which will set the appropriate unit nodes
			// as active. Active nodes are set for every recommendation type.
			LearningNet net(d["network"].GetString());
			net.setCompleted(toIntVector(d["sections"]));
			Recommender rec(net,
				toConditionMap(d["conditions"]),
				toTestMap(d["testGrades"])
			);

			if (hasPath) {
				// Set path with heuristically lowest costs as path attribute.
				std::vector<lemon::ListDigraph::Node> recPath = hasOnlyNodeCosts ?
					rec.recPath(toNodeCosts(net, d["nodeCosts"])) :
					rec.recPath(toNodePairCosts(net, d["nodeCosts"], d["nodePairCosts"]));

				net.setRecommended(recPath);
			} else if (hasNext) {
				// Set node with lowest costs as recommended path attribute.
				lemon::ListDigraph::Node recNode = hasOnlyNodeCosts ?
					rec.recNext(toNodeCosts(net, d["nodeCosts"])) :
					rec.recNext(toNodePairCosts(net, d["nodeCosts"], d["nodePairCosts"]));
				std::vector<lemon::ListDigraph::Node> recPath = {recNode};
				net.setRecommended(recPath);
			}

			net.write(std::cout, rec.getVisited());
			return rec.handleFailure();
		} else {
			std::cout << "No known action given." << std::endl;
			return EXIT_FAILURE;
		}
	} catch (Exception &e) {
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
