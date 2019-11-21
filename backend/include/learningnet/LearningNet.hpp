#pragma once

#include <lemon/list_graph.h>
#include <lemon/concepts/digraph.h>
#include <lemon/lgf_reader.h>
#include <sstream>

namespace learningnet {

//! The condition value representing the "else" branch of a condition node.
const std::string CONDITION_ELSE_BRANCH_KEYWORD{"SONST"};

/**
 * Pseudo-enum-class encapsulating the types a node in a learning net can have.
 */
class NodeType {
	private:
		int m_underlying; //!< this NodeType represented as an int

	public:
		static constexpr int inactive = 0; //!< inactive unit node
		static constexpr int active = 1; //!< active unit node
		static constexpr int completed = 2; //!< completed unit node
		static constexpr int split = 10; //!< split node
		static constexpr int condition = 11; //!< condition node
		static constexpr int test = 12; //!< test node
		static constexpr int join = 20; //!< join node

		/**
		 * Creates a new NodeType.
		 *
		 * @param v the int representation of the node type
		 */
		NodeType(int v) : m_underlying(v) {}

		/**
		 * @return this NodeType as an int
		 */
		operator int() {
			return m_underlying;
		}
};

/**
 * Class representing a learning net.
 */
class LearningNet : public lemon::ListDigraph
{
private:
	lemon::ListDigraph::NodeMap<int> m_type; //!< type of each node

	lemon::ListDigraph::NodeMap<int> m_ref; //!< ref value of each node

	//! condition value or test grade of each edge
	lemon::ListDigraph::ArcMap<std::string> m_condition;

	lemon::ListDigraph::Node m_target; //!< target node

	//! recommended learning path or unit node
	std::vector<lemon::ListDigraph::Node> m_recommended;

	/**
	 * Sets the ref value of a node.
	 *
	 * @param v the node
	 * @param ref the new ref value
	 */
	void setReference(const lemon::ListDigraph::Node &v, int ref) {
		m_ref[v] = ref;
	}

	/**
	 * Turns a vector of nodes into a space-seperated string of node ids.
	 *
	 * @param vs vector of nodes
	 * @return space-separated string of node ids
	 */
	std::string stringify(const std::vector<lemon::ListDigraph::Node> &vs) const
	{
		std::ostringstream oss;
		bool first = true;
		for (auto v : vs) {
			if (first) {
				first = false;
				oss << id(v);
			} else {
				oss << " " << id(v);
			}
		}
		return oss.str();
	}

public:

	/**
	 * Creates a new empty LearningNet.
	 */
	LearningNet()
		: lemon::ListDigraph()
		, m_type{*this}
		, m_ref{*this}
		, m_condition{*this}
		, m_target{lemon::INVALID}
		, m_recommended{std::vector<lemon::ListDigraph::Node>()}
		{}

	/**
	 * Creates a new LearningNet.
	 *
	 * The attribute "recommended" is not read.
	 *
	 * @param network this learning net represented in LGF
	 * @throws lemon::ParserException if the reading of \p networks fails
	 */
	LearningNet(const std::string &network) : LearningNet()
	{
		// Read lemon graph file given as arg.
		std::istringstream networkIss(network);
		lemon::DigraphReader<lemon::ListDigraph>(*this, networkIss)
			.nodeMap("type", m_type)
			.nodeMap("ref", m_ref)
			.arcMap("condition", m_condition)
			.node("target", m_target)
			// Do not read attribute "recommended".
			// It may be set by the Recommender, the old value is not relevant.
			.run();
	};

	/**
	 * Creates a new LearningNet with one unit node for each section id in \p
	 * sections. The successor of each of these unit nodes is a join node with
	 * a number of necessary in-arcs that equals the number of unit nodes. This
	 * join node is the target of the net.
	 *
	 * @param sections section ids
	 * @return newly created LearningNet
	 */
	static LearningNet *create(const std::vector<int> &sections) {
		LearningNet *net = new LearningNet();
		// Connect all units with one join.
		lemon::ListDigraph::Node join = net->addNode();
		net->setType(join, NodeType::join);
		net->setNecessaryInArcs(join, 0);
		net->setTarget(join);

		for (int section : sections) {
			lemon::ListDigraph::Node v = net->addNode();
			net->setType(v, NodeType::inactive);
			net->setSection(v, section);

			net->addArc(v, join);
			net->setType(join, net->getType(join) + 1);
			net->setNecessaryInArcs(join, net->getNecessaryInArcs(join) + 1);
		}
		return net;
	}

	/**
	 * @param v node that is checked for being a source.
	 * @return whether the node \p v has zero inedges.
	 */
	bool isSource(const lemon::ListDigraph::Node &v) const {
		lemon::ListDigraph::InArcIt a(*this, v);
		return a == lemon::INVALID;
	}

	/**
	 * @param v node that is checked for being a dead end.
	 * @return whether the node \p v has zero outedges.
	 */
	bool isDeadEnd(const lemon::ListDigraph::Node &v) const {
		lemon::ListDigraph::OutArcIt a(*this, v);
		return a == lemon::INVALID;
	}

	// Type Checkers, Getter and Setter
	// @{

	/**
	 * @param v the node
	 * @return whether \p v is a unit node
	 */
	bool isUnit(const lemon::ListDigraph::Node &v) const {
		return m_type[v] < NodeType::split;
	}

	/**
	 * @param v the node
	 * @return whether \p v is a split node
	 */
	bool isSplit(const lemon::ListDigraph::Node &v) const {
		return m_type[v] == NodeType::split;
	}

	/**
	 * @param v the node
	 * @return whether \p v is a condition node
	 */
	bool isCondition(const lemon::ListDigraph::Node &v) const {
		return m_type[v] == NodeType::condition;
	}

	/**
	 * @param v the node
	 * @return whether \p v is a test node
	 */
	bool isTest(const lemon::ListDigraph::Node &v) const {
		return m_type[v] >= NodeType::test && m_type[v] < NodeType::join;
	}

	/**
	 * @param v the node
	 * @return whether \p v is a split, condition or test node, i.e. a node with
	 * possibly multiple outgoing edges
	 */
	bool isSplitLike(const lemon::ListDigraph::Node &v) const {
		return m_type[v] >= NodeType::split && m_type[v] < NodeType::join;
	}

	/**
	 * @param v the node
	 * @return whether \p v is a join node
	 */
	bool isJoin(const lemon::ListDigraph::Node &v) const {
		return m_type[v] >= NodeType::join;
	}

	/**
	 * @param v the node
	 * @return whether the type of \p v is unknown
	 */
	bool isUnknown(const lemon::ListDigraph::Node &v) const {
		return m_type[v] < NodeType::inactive
		   || (m_type[v] > NodeType::completed && m_type[v] < NodeType::split)
		   || (m_type[v] > NodeType::test && m_type[v] < NodeType::join);
	}

	/**
	 * @param v the node
	 * @return the type of \p v
	 */
	int getType(const lemon::ListDigraph::Node &v) const {
		return m_type[v];
	}

	/**
	 * Sets the type of a given node.
	 *
	 * @param v the node
	 * @param type the new type of \p v (preferrably given as a NodeType
	 * implicitly casted to int)
	 */
	void setType(const lemon::ListDigraph::Node &v, int type) {
		m_type[v] = type;
	}

	// @}
	// Reference Getters and Setters
	// @{

	/**
	 * @param v the node
	 * @return the section id of \p v if \p v is a unit node, -1 otherwise
	 */
	int getSection(const lemon::ListDigraph::Node &v) const {
		return isUnit(v) ? m_ref[v] : -1;
	}

	/**
	 * Sets the section id of a unit node.
	 *
	 * @param v the unit node
	 * @param section the new section id
	 * @throws exception if \p v is not a unit node
	 */
	void setSection(const lemon::ListDigraph::Node &v, int section) {
		if (isUnit(v)) {
			setReference(v, section);
		} else {
			throw "setSection() on node which is not a unit.";
		}
	}

	/**
	 * @param v the node
	 * @return the number of ingoing edges coming from reachable nodes in order
	 * for \p v to be reachable if \p v is a join node, -1 otherwise
	 */
	int getNecessaryInArcs(const lemon::ListDigraph::Node &v) const {
		return isJoin(v) ? m_ref[v] : -1;
	}

	/**
	 * Sets the necessary in-arcs of a join node.
	 *
	 * @param v the join node
	 * @param inArcs the new number of necessary in-arcs
	 * @throws exception if \p v is not a join node
	 */
	void setNecessaryInArcs(const lemon::ListDigraph::Node &v, int inArcs) {
		if (isJoin(v)) {
			setReference(v, inArcs);
		} else {
			throw "setNecessaryInArcs() on node which is not a join.";
		}
	}

	/**
	 * @param v the node
	 * @return the condition id of \p v if \p v is a condition node, -1
	 * otherwise
	 */
	int getConditionId(const lemon::ListDigraph::Node &v) const {
		return isCondition(v) ? m_ref[v] : -1;
	}

	/**
	 * Sets the condition id of a condition node.
	 *
	 * @param v the condition node
	 * @param conditionId the new condition id
	 * @throws exception if \p v is not a condition node
	 */
	void setConditionId(const lemon::ListDigraph::Node &v, int conditionId) {
		if (isCondition(v)) {
			setReference(v, conditionId);
		} else {
			throw "setConditionId() on node which is not a condition.";
		}
	}

	/**
	 * @param v the node
	 * @return the test id of \p v if \p v is a test node, -1 otherwise
	 */
	int getTestId(const lemon::ListDigraph::Node &v) const {
		return isTest(v) ? m_ref[v] : -1;
	}

	/**
	 * Sets the test id of a test node.
	 *
	 * @param v the test node
	 * @param testId the new test id
	 * @throws exception if \p v is not a test node
	 */
	void setTestId(const lemon::ListDigraph::Node &v, int testId) {
		if (isTest(v)) {
			setReference(v, testId);
		} else {
			throw "setTestId() on node which is not a test.";
		}
	}


	// @}
	// Target Getter and Setter
	// @{

	/**
	 * @return the target node of this learning net
	 */
	lemon::ListDigraph::Node getTarget() const {
		return m_target;
	}

	/**
	 * Sets the target node of this learning net.
	 *
	 * @param tgt the new target node of this learning net
	 */
	void setTarget(const lemon::ListDigraph::Node &tgt) {
		m_target = tgt;
	}

	/**
	 * @param v node that is checked for being the target.
	 * @return whether the node \p v is the target.
	 */
	bool isTarget(const lemon::ListDigraph::Node &v) const {
		return v == m_target;
	}

	// @}
	// Recommended Learning Path Getter and Setter
	// @{

	/**
	 * @return the recommend learning path (or node) associated with this
	 * learning net
	 */
	std::vector<lemon::ListDigraph::Node> getRecommended() const {
		return m_recommended;
	}

	/**
	 * Sets the recommended learning path (or node) associated with this
	 * learning net.
	 *
	 * @param rec the new recommende learning path (or node)
	 */
	void setRecommended(const std::vector<lemon::ListDigraph::Node> &rec) {
		m_recommended = rec;
	}

	// @}
	// Condition Branch Getter and Setter
	// @{

	/**
	 * @param a the edge
	 * @return the condition value or test grade associated with \p a
	 */
	std::string getConditionBranch(const lemon::ListDigraph::Arc &a) const {
		return m_condition[a];
	}

	/**
	 * Sets the condition value or test grade of an edge.
	 *
	 * @param a the edge
	 * @param branch the new condition value or test grade
	 */
	void setConditionBranch(const lemon::ListDigraph::Arc &a,
			const std::string &branch) {
		m_condition[a] = branch;
	}

	// @}
	// Helper Functions for Join nodes
	// @{

	/**
	 * Resets the activated in-arcs of a join node to 0.
	 *
	 * @param v the join node
	 * @throws exception if \p v is not a join node
	 */
	void resetActivatedInArcs(const lemon::ListDigraph::Node &v) {
		if (isJoin(v)) {
			m_type[v] = NodeType::join;
		} else {
			throw "resetActivatedInArcs() on node which is not a join.";
		}
	}

	/**
	 * Increments the activated in-arcs of a join node by 1.
	 *
	 * @param v the join node
	 * @throws exception if \p v is not a join node
	 */
	void incrementActivatedInArcs(const lemon::ListDigraph::Node &v) {
		if (isJoin(v)) {
			m_type[v]++;
		} else {
			throw "incrementActivatedInArcs() on node which is not a join.";
		}
	}

	/**
	 * @param v the join node
	 * @throws exception if \p v is not a join node
	 * @return activated in-arcs of \p v
	 */
	int getActivatedInArcs(const lemon::ListDigraph::Node &v) const {
		if (isJoin(v)) {
			return m_type[v] - NodeType::join;
		} else {
			throw "getActivatedInArcs() on node which is not a join.";
		}
	}

	/**
	 * @param v the node
	 * @return whether \p v is join node with at least as many activated in-arcs
	 * as necessary ones
	 */
	bool isCompletedJoin(const lemon::ListDigraph::Node &v) const {
		return isJoin(v) && getActivatedInArcs(v) >= getNecessaryInArcs(v);
	}

	// @}

	/**
	 * Writes this LearningNet in LGF to the stream \p out.
	 *
	 * @param out the stream to which the learning net is written
	 * @param visited optional, mapping from edges to whether that edge was
	 * visited during a learning path search, also written to \p out if given
	 */
	void write(std::ostream &out = std::cout,
			const lemon::ListDigraph::ArcMap<bool> *visited = nullptr) const {
		// Write lemon graph file to cout.
		lemon::DigraphWriter<lemon::ListDigraph> writer{*this, out};
		writer.nodeMap("type", m_type)
		      .nodeMap("ref", m_ref)
		      .arcMap("condition", m_condition);

		// Write out visited arcs if given.
		if (visited) {
			writer.arcMap("visited", *visited);
		}

		writer.node("target", m_target)
		      .attribute("recommended", stringify(m_recommended))
		      .run();
	}

	/**
	 * Set the type of the unit nodes given by \p completed to completed.
	 * @param completed section numbers of completed unit nodes
	 * @return section numbers of completed units for which no unit node could
	 * be found
	 */
	std::vector<int> setCompleted(const std::vector<int> &completed)
	{
		// Get map: sectionId -> node.
		std::map<int, lemon::ListDigraph::Node> sectionNode;
		for (auto v : nodes()) {
			if (isUnit(v)) {
				sectionNode[getSection(v)] = v;
			}
		}

		// Set type of completed units.
		std::vector<int> couldNotBeSet;
		for (int completedSection : completed) {
			auto completedNode = sectionNode.find(completedSection);
			if (completedNode != sectionNode.end()) {
				// Only set it for units, not for connectives!
				setType(completedNode->second, NodeType::completed);
			} else {
				couldNotBeSet.push_back(completedSection);
			}
		}

		return couldNotBeSet;
	}

};

}
