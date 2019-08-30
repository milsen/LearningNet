#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <learningnet/NetworkChecker.hpp>
#include <learningnet/ActivitySetter.hpp>
#include <learningnet/Recommender.hpp>

using namespace learningnet;
using namespace rapidjson;

bool hasCorrectArgs(const Document &d, const std::initializer_list<const char *> &args)
{
	std::map<std::string, std::function<bool(const Value&)>> typeFunc = {
		{ "action",       std::bind(&Value::IsString, std::placeholders::_1) },
		{ "network",      std::bind(&Value::IsString, std::placeholders::_1) },
		{ "recTypes",     std::bind(&Value::IsArray, std::placeholders::_1) },
		{ "sections",     std::bind(&Value::IsArray, std::placeholders::_1) },
		{ "conditions",   std::bind(&Value::IsArray, std::placeholders::_1) },
		{ "test_grades",  std::bind(&Value::IsObject, std::placeholders::_1) },
		{ "nodeCosts",    std::bind(&Value::IsObject, std::placeholders::_1) },
		{ "edgeCosts",    std::bind(&Value::IsObject, std::placeholders::_1) },
		{ "useNodeCosts", std::bind(&Value::IsBool, std::placeholders::_1) }
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

lemon::ListDigraph::NodeMap toNodeMap(const Value &oldObj)
{
	lemon::ListDigraph::NodeMap nodeMap;
	for (auto &m : oldObj.GetObject()) {
		nodeMap[std::stoi(m.name.GetString())] = m.value.GetInt();
	}

	return idToVal;
}

void output(const Document &d)
{
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);

    std::cout << buffer.GetString() << std::endl;
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

		std::string action = d["action"].GetString();

		// Check for correct parameters.
		if ((action == "check"     && !hasCorrectArgs(d, {"network"}))
		 || (action == "create"    && !hasCorrectArgs(d, {"sections"}))
		 || (action == "recommend" && !hasCorrectArgs(d, {"recTypes","network","sections","conditions","test_grades"}))
		 ) {
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
			std::vector<std::string> recTypes = toStringVector(d["recTypes"]);
			auto isRecType = [&recTypes](const std::string &s) {
				return std::find(recTypes.begin(), recTypes.end(), s) != recTypes.end();
			};
			bool hasActive = isRecType("active");
			bool hasNext   = isRecType("next");
			bool hasPath   = isRecType("path");

			if (!hasActive && !hasNext && !hasPath) {
				std::cout << "No valid recommendation type (\"active\", "
					      << "\"next\" or \"path\") given even though "
				          << "the action is \"recommend\"." << std::endl;
				return EXIT_FAILURE;
			}

			if ((hasNext || hasPath) && !hasCorrectArgs(d, {"nodeCosts"})
			 && !hasCorrectArgs(d, {"edgeCosts"})) {
				std::cout << "Not node or edge costs given even though the "
				          << "recommendation types require it." << std::endl;
				return EXIT_FAILURE;
			}

			// Activity must be found out even if it is not output (i.e. the
			// recType active is not given).
			LearningNet net(d["network"].GetString());
			ActivitySetter act(net,
				toIntVector(d["sections"]),
				toConditionMap(d["conditions"]),
				toTestMap(d["test_grades"])
			);
			// TODO

			if (hasNext || hasPath) {
				Recommender rec{net}; //with set activity
				if (hasNext) {
					lemon::ListDigraph::Node recNode =
						rec.recNext(nodeCosts, edgeCosts);

					// TODO just set one item type to 3 (=recommended)
				}
				if (hasPath) {
					std::vector<lemon::ListDigraph::Node> recPath =
						rec.recPath(nodeCosts, edgeCosts);
					// TODO output path somehow using output()?
				}
			}

			if (hasActive) {
				net.write();
			}
			return act.handleFailure();
		} else {
			std::cout << "No known action given." << std::endl;
			return EXIT_FAILURE;
		}
	} catch (Exception &e) {
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
