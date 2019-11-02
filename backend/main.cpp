#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <learningnet/NetworkChecker.hpp>
#include <learningnet/Recommender.hpp>

using namespace learningnet;
using namespace rapidjson;

class DataReader : public Module
{
private:
	Document m_d; //!< The JSON document to be processed.

	LearningNet *m_net; //!< The learning net read from the document.

	/**
	 * Called from the constructor:
	 * Checks whether the correct parameters were set and initializes #m_net if
	 * the actions "check" or "recommend" were chosen.
	 *
	 * @param action string given in #m_d under the key "action"
	 */
	void initialize(const std::string &action) {
		// Check for correct parameters.
		if (action == "check") {
			checkArgs({"network"});
		} else if (action == "create") {
			checkArgs({"sections"});
		} else if (action == "recommend") {
			checkArgs({"recType","network","sections","conditions","testGrades"});
		} else {
			failWithError("Given action is unknown.");
		}

		// If the wrong parameters were given, we should not continue.
		if (!succeeded()) {
			return;
		}

		if (action == "check") {
			// Set the net which to check.
			m_net = new LearningNet(m_d["network"].GetString());
		} else if (action == "recommend") {
			// Check recType and nodeCosts/nodePairCosts.
			std::string recType = m_d["recType"].GetString();
			bool hasActive = recType == "active";
			bool hasNext   = recType == "next";
			bool hasPath   = recType == "path";
			bool hasNextOrPath = hasNext || hasPath;

			if (!hasActive && !hasNextOrPath) {
				failWithError("No valid recommendation type "
					"(\"active\", \"next\" or \"path\") given even "
					"though the action is \"recommend\".");
			}

			bool hasNodeCosts = m_d.HasMember("nodeCosts");
			bool hasNodePairCosts = m_d.HasMember("nodePairCosts");
			if (hasNextOrPath && !hasNodeCosts && !hasNodePairCosts) {
				failWithError("No node or node pair costs given even though "
					"the recommendation type \"" + recType + "\" requires it.");
			}
			if (hasNodeCosts) {
				checkArgs({"nodeCosts"});
			}
			if (hasNodePairCosts) {
				checkArgs({"nodePairCosts"});
			}

			// Initialize the net for which to get a recommendation.
			m_net = new LearningNet(m_d["network"].GetString());
			std::vector<int> notFound = m_net->setCompleted(getSections());
			for (auto section : notFound) {
				appendError("Setting completed units: Could not find section " +
					std::to_string(section) + ".");
			}
		}
	}

	/**
	 * Checks whether #m_d has the keys \p args and whether the corresponding
	 * values have the correct type and content.
	 *
	 * If that is not the case, the error is noted via Module::failWithError().
	 *
	 * @param args initializer list of key strings to check for
	 */
	void checkArgs(const std::initializer_list<const char *> &args)
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

		for (auto arg : args) {
			std::string argStr = arg;
			if (!m_d.HasMember(arg)) {
				failWithError("Input has no member \"" + argStr + "\".");

			} else if (!typeFunc[arg](m_d[arg])) {
				failWithError("Member \"" + argStr +
						"\" does not have the correct type.");

			} else if (argStr == "nodeCosts" || argStr == "nodePairCosts") {
				if (m_d[arg].Empty()) {
					failWithError("Member \"" + argStr + "\" is empty.");
				}
				for (auto &val : m_d[arg].GetArray()) {
					auto obj = val.GetObject();
					if (!obj.HasMember("costs")) {
						failWithError("Entry of member \"" + argStr
								+ "\" has no member \"costs\".");
					}
					if (!obj.HasMember("weight")) {
						failWithError("Entry of member \"" + argStr
								+ "\" has no member \"weight\".");
					}
				}
			}
		}
	}

	/**
	 * @param oldArr array to convert
	 * @return vector of ints corresponding to \p oldArr
	 */
	std::vector<int> toIntVector(const Value &oldArr) const
	{
		std::vector<int> arr;
		for (auto &v : oldArr.GetArray()) {
			arr.push_back(std::stoi(v.GetString()));
		}

		return arr;
	}

	/**
	 * @param oldArr array to convert
	 * @return vector of strings corresponding to \p oldArr
	 */
	std::vector<std::string> toStringVector(const Value &oldArr) const
	{
		std::vector<std::string> arr;
		for (auto &v : oldArr.GetArray()) {
			arr.push_back(v.GetString());
		}

		return arr;
	}

	/**
	 * @param oldArr array to convert
	 * @return ConditionMap corresponding to \p oldArr
	 */
	ConditionMap toConditionMap(const Value &oldArr) const
	{
		ConditionMap idToVal;
		for (auto &v : oldArr.GetArray()) {
			idToVal.push_back(toStringVector(v));
		}

		return idToVal;
	}

	/**
	 * @param oldObj object to convert
	 * @return TestMap corresponding to \p oldObj
	 */
	TestMap toTestMap(const Value &oldObj) const
	{
		TestMap idToVal;
		for (auto &m : oldObj.GetObject()) {
			idToVal[std::stoi(m.name.GetString())] =
				std::stoi(m.value.GetString());
		}

		return idToVal;
	}

	/**
	 * @param nodeCostArr array of weights & cost function values for each node
	 * @return weighted sum of all costs given for each node
	 */
	NodeCosts toNodeCosts(const Value &nodeCostArr) const
	{
		NodeCosts nodeCosts;

		// Get weight sum.
		double weightSum = 0.0;
		for (auto &val : nodeCostArr.GetArray()) {
			weightSum += val.GetObject()["weight"].GetDouble();
		}

		for (auto v : m_net->nodes()) {
			if (m_net->isUnit(v)) {
				const char *vSection =
					std::to_string(m_net->getSection(v)).c_str();
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

	/**
	 * @param nodePairCostArr array of weights & cost function values for each
	 * pair of nodes
	 * @return weighted sum of all costs given for each pair of nodes
	 */
	NodePairCosts toNodePairCosts(
		const Value &nodeCostArr,
		const Value &nodePairCostArr) const
	{
		NodePairCosts nodePairCosts;

		// Get weight sum.
		double weightSum = 0.0;
		for (auto &val : nodeCostArr.GetArray()) {
			weightSum += val.GetObject()["weight"].GetDouble();
		}
		for (auto &val : nodePairCostArr.GetArray()) {
			weightSum += val.GetObject()["weight"].GetDouble();
		}

		// Get weighted costs for each node pair.
		for (auto &v : m_net->nodes()) {
			if (m_net->isUnit(v)) {
				const char *srcSection =
					std::to_string(m_net->getSection(v)).c_str();

				for (auto &w : m_net->nodes()) {
					if (m_net->isUnit(w)) {
						const char *tgtSection =
							std::to_string(m_net->getSection(w)).c_str();
						nodePairCosts[v][w] = 0.0;

						for (auto &val : nodeCostArr.GetArray()) {
							auto obj = val.GetObject();
							double weight = obj["weight"].GetDouble();
							auto &costDict = obj["costs"];
							auto itr = costDict.FindMember(tgtSection);
							if (itr != costDict.MemberEnd()) {
								nodePairCosts[v][w] +=
									(itr->value.GetDouble() * weight) / weightSum;
							}
						}

						for (auto &val : nodePairCostArr.GetArray()) {
							auto obj = val.GetObject();
							double weight = obj["weight"].GetDouble();
							auto &costDict = obj["costs"];
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

public:

	/**
	 * Reads the data \p data, initializes an underlying JSON Document #m_d and
	 * checks whether the correct keys and values were given.
	 * If the actions "check" or "recommend" were chosen, #m_net is also set.
	 *
	 * @param data JSON data to read
	 */
	DataReader(char *data) {
		// Parse document.
		m_d.Parse(data);

		// Only initialize if action given.
		checkArgs({"action"});
		if (succeeded()) {
			initialize(m_d["action"].GetString());
		}
	}

	std::string getAction() const {
		return m_d["action"].GetString();
	}

	LearningNet *getNet() const {
		return m_net;
	}

	std::vector<int> getSections() const {
		return toIntVector(m_d["sections"]);
	}

	std::string getRecType() const {
		return m_d["recType"].GetString();
	}

	ConditionMap getConditionValues() const {
		return toConditionMap(m_d["conditions"]);
	}

	TestMap getTestGrades() const {
		return toTestMap(m_d["testGrades"]);
	}

	NodeCosts getNodeCosts() const {
		return toNodeCosts(m_d["nodeCosts"]);
	}

	NodePairCosts getNodePairCosts() const {
		return toNodePairCosts(m_d["nodeCosts"], m_d["nodePairCosts"]);
	}

	bool hasOnlyNodeCosts() const {
		return m_d.HasMember("nodeCosts") && !m_d.HasMember("nodePairCosts");
	}
};


int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cout << "No argument given to executable." << std::endl;
		return EXIT_FAILURE;
	}

	try {
		DataReader reader{argv[1]};

		if (!reader.succeeded()) {
			return reader.handleFailure();
		}

		std::string action = reader.getAction();
		// Execute action.
		if (action == "check") {
			LearningNet *net = reader.getNet();
			NetworkChecker checker(*net);
			delete net;
			return checker.handleFailure();
		} else if (action == "create") {
			LearningNet *net = LearningNet::create(reader.getSections());
			net->write();
			delete net;
			return EXIT_SUCCESS;
		} else if (action == "recommend") {
			// Get LearningNet, set given sections as completed and pass the
			// net to the Recommender, which will set the appropriate unit nodes
			// as active. Active nodes are set for every recommendation type.
			LearningNet *net = reader.getNet();
			Recommender rec(*net,
				reader.getConditionValues(),
				reader.getTestGrades()
			);

			if (reader.getRecType() == "path") {
				// Set path with heuristically lowest costs as path attribute.
				std::vector<lemon::ListDigraph::Node> recPath =
					reader.hasOnlyNodeCosts() ?
					rec.recPath(reader.getNodeCosts()) :
					rec.recPath(reader.getNodePairCosts());

				net->setRecommended(recPath);
			} else if (reader.getRecType() == "next") {
				// Set node with lowest costs as recommended path attribute.
				lemon::ListDigraph::Node recNode =
					reader.hasOnlyNodeCosts() ?
					rec.recNext(reader.getNodeCosts()) :
					rec.recNext(reader.getNodePairCosts());

				std::vector<lemon::ListDigraph::Node> recPath = {recNode};
				net->setRecommended(recPath);
			}

			net->write(std::cout, rec.getVisited());
			delete net;
			return rec.handleFailure();
		}
	} catch (Exception &e) {
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
