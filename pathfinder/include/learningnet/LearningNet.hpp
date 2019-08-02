#pragma once

#include <lemon/list_graph.h>
#include <lemon/concepts/digraph.h>
#include <lemon/lgf_reader.h>

namespace learningnet {

class NodeType {
	private:
		int m_underlying;

	public:
		static constexpr int inactive = 0;
		static constexpr int active = 1;
		static constexpr int completed = 2;
		static constexpr int split = 10;
		static constexpr int join = 20;

		NodeType(int v) : m_underlying(v) {}

		operator int() {
			return m_underlying;
		}
};


// TODO management of unit types, join units
class LearningNet : public lemon::ListDigraph
{
private:
	lemon::ListDigraph::NodeMap<int> m_type;
	lemon::ListDigraph::NodeMap<int> m_ref;
	lemon::ListDigraph::Node m_target;

	void setReference(const lemon::ListDigraph::Node &v, int ref) {
		m_ref[v] = ref;
	}

public:
	LearningNet()
		: lemon::ListDigraph()
		, m_type{*this}
		, m_ref{*this}
		{}

	/**
	 * @throws lemon ParserException
	 */
	LearningNet(const std::string &network) : LearningNet()
	{
		// Read lemon graph file given as arg.
		std::istringstream networkIss(network);
		lemon::DigraphReader<lemon::ListDigraph>(*this, networkIss)
			.nodeMap("type", m_type)
			.nodeMap("ref", m_ref)
			.node("target", m_target)
			.run();
	};

	virtual ~LearningNet ();

	static LearningNet *create(const std::vector<std::string> &sections) {
		LearningNet *net = new LearningNet();
		// Connect all units with one join.
		lemon::ListDigraph::Node join = net->addNode();
		net->setType(join, NodeType::join);
		net->setNecessaryInArcs(join, 0);
		net->setTarget(join);

		for (std::string section : sections) {
			lemon::ListDigraph::Node v = net->addNode();
			net->setType(v, NodeType::inactive);
			net->setSection(v, std::stoi(section));

			net->addArc(v, join);
			net->setType(join, net->getType(join) + 1);
			net->setNecessaryInArcs(join, net->getNecessaryInArcs(join) + 1);
		}
		return net;
	}

	static LearningNet *create(const std::string &sectionStr) {
		std::istringstream sectionsIss(sectionStr);
		std::vector<std::string> sections(
			std::istream_iterator<std::string>{sectionsIss},
			std::istream_iterator<std::string>()
		);
		return create(sections);
	}

	// Type Checkers, Getter and Setter
	// @{
	bool isUnit(const lemon::ListDigraph::Node &v) const {
		return m_type[v] < NodeType::split;
	}

	bool isSplit(const lemon::ListDigraph::Node &v) const {
		return m_type[v] >= NodeType::split && m_type[v] < NodeType::join;
	}

	bool isJoin(const lemon::ListDigraph::Node &v) const {
		return m_type[v] >= NodeType::join;
	}

	int getType(const lemon::ListDigraph::Node &v) const {
		return m_type[v];
	}

	void setType(const lemon::ListDigraph::Node &v, int type) {
		m_type[v] = type;
	}

	// @}
	// Reference Getters and Setters
	// @{
	int getSection(const lemon::ListDigraph::Node &v) const {
		return isUnit(v) ? m_ref[v] : -1;
	}

	void setSection(const lemon::ListDigraph::Node &v, int section) {
		if (isUnit(v)) {
			setReference(v, section);
		} else {
			throw "setSection() on node which is not a unit.";
		}
	}

	int getNecessaryInArcs(const lemon::ListDigraph::Node &v) const {
		return isJoin(v) ? m_ref[v] : countInArcs(*this, v);
	}

	void setNecessaryInArcs(const lemon::ListDigraph::Node &v, int inArcs) {
		if (isJoin(v)) {
			setReference(v, inArcs);
		} else {
			throw "setNecessaryInArcs() on node which is not a join.";
		}
	}

	int getConditionId(const lemon::ListDigraph::Node &v) const {
		return isSplit(v) ? m_ref[v] : 0;
	}

	void setConditionId(const lemon::ListDigraph::Node &v, int conditionId) {
		if (isSplit(v)) {
			setReference(v, conditionId);
		} else {
			throw "setConditionId() on node which is not a split.";
		}
	}

	// @}
	// Target Getter and Setter
	// @{
	lemon::ListDigraph::Node getTarget() const {
		return m_target;
	}

	void setTarget(const lemon::ListDigraph::Node &tgt) {
		m_target = tgt;
	}

	// @}
	// Helper Functions for Join nodes
	// @{
	void resetActivatedInArcs(const lemon::ListDigraph::Node &v) {
		if (isJoin(v)) {
			m_type[v] = NodeType::join;
		} else {
			throw "resetActivatedInArcs() on node which is not a join.";
		}
	}

	void incrementActivatedInArcs(const lemon::ListDigraph::Node &v) {
		if (isJoin(v)) {
			m_type[v]++;
		} else {
			throw "incrementActivatedInArcs() on node which is not a join.";
		}
	}

	int getActivatedInArcs(const lemon::ListDigraph::Node &v) const {
		if (isJoin(v)) {
			return m_type[v] - NodeType::join;
		} else {
			throw "getActivatedInArcs() on node which is not a join.";
		}
	}

	bool isUnlockedJoin(const lemon::ListDigraph::Node &v) const {
		return isJoin(v) && getActivatedInArcs(v) >= getNecessaryInArcs(v);
	}

	// @}
	void write(std::ostream &out = std::cout) {
		// Write lemon graph file to cout.
		lemon::DigraphWriter<lemon::ListDigraph>(*this, out)
			.nodeMap("type", m_type)
			.nodeMap("ref", m_ref)
			.node("target", m_target)
			.run();
	}

};

}
