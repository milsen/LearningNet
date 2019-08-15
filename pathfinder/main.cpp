#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <learningnet/NetworkChecker.hpp>
#include <learningnet/ActivitySetter.hpp>

using namespace lemon;
using namespace learningnet;
using namespace rapidjson;

bool hasCorrectArgs(const Document &d, const std::initializer_list<const char *> args)
{
	std::map<std::string, std::function<bool(const Value&)>> typeFunc = {
		{ "action",       std::bind(&Value::IsString, std::placeholders::_1) },
		{ "network",      std::bind(&Value::IsString, std::placeholders::_1) },
		{ "sections",     std::bind(&Value::IsArray, std::placeholders::_1) },
		{ "conditions",   std::bind(&Value::IsObject, std::placeholders::_1) },
		{ "costs",        std::bind(&Value::IsObject, std::placeholders::_1) },
		{ "useNodeCosts", std::bind(&Value::IsBool, std::placeholders::_1) }
	};

	bool result = true;
	for (auto arg : args) {
		result &= d.HasMember(arg) && typeFunc[arg](d[arg]);
	}

	return result;
}

bool streq(const std::string &str1, const std::string &str2)
{
	return str1.compare(str2) == 0;
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

ConditionMap toMap(const Value &oldObj)
{
	ConditionMap idToVal;
	for (auto &m : oldObj.GetObject()) {
		idToVal[std::stoi(m.name.GetString())] = toStringVector(m.value);
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
		if ((streq(action, "check")   && !hasCorrectArgs(d, {"network"}))
		 || (streq(action, "create")  && !hasCorrectArgs(d, {"sections"}))
		 || (streq(action, "active")  && !hasCorrectArgs(d, {"network","sections","conditions"}))
		 || (streq(action, "recnext") && !hasCorrectArgs(d, {"network","costs"}))
		 || (streq(action, "recpath") && !hasCorrectArgs(d, {"network","costs"}))) {
			std::cout << "Not all necessary parameters for the given action found." << std::endl;
			return EXIT_FAILURE;
		}

		// TODO type checks for arrays/objects

		// Execute action.
		if (streq(action, "check")) {
			LearningNet net(d["network"].GetString());
			NetworkChecker checker(net);
			return checker.handleFailure();
		} else if (streq(action, "create")) {
			LearningNet *net = LearningNet::create(toIntVector(d["sections"]));
			net->write();
			delete net;
			return EXIT_SUCCESS;
		} else if (streq(action, "active")) {
			LearningNet net(d["network"].GetString());
			ActivitySetter act(net, toIntVector(d["sections"]), toMap(d["conditions"]));
			net.write();
			return act.handleFailure();
		} else if (streq(action, "recnext")) {
			// TODO just set one item type to 3 (=recommended)
			return EXIT_FAILURE;
		} else if (streq(action, "recpath")) {
			return EXIT_FAILURE;
		} else {
			std::cout << "No known action given." << std::endl;
			return EXIT_FAILURE;
		}
	} catch (Exception &e) {
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
