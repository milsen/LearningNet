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
	lemon::ListDigraph::NodeMap<int> m_section;
	lemon::ListDigraph::NodeMap<int> m_type;
	lemon::ListDigraph::Node m_target;

public:
	LearningNet()
		: lemon::ListDigraph()
		, m_section{*this}
		, m_type{*this}
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
			.nodeMap("section", m_section)
			.node("target", m_target)
			.run();
	};

	virtual ~LearningNet ();

	static LearningNet *create(const std::vector<std::string> &sections) {
		LearningNet *net = new LearningNet();
		// Connect all units with one join.
		lemon::ListDigraph::Node join = net->addNode();
		net->setSection(join, 0);
		net->setType(join, NodeType::join);
		net->setTarget(join);

		for (std::string section : sections) {
			lemon::ListDigraph::Node v = net->addNode();
			net->setType(v, NodeType::inactive);
			net->setSection(v, std::stoi(section));

			net->addArc(v, join);
			net->setType(join, net->getType(join) + 1);
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

	int getType(const lemon::ListDigraph::Node &v) const {
		return m_type[v];
	}

	void setType(const lemon::ListDigraph::Node &v, int type) {
		m_type[v] = type;
	}

	int getSection(const lemon::ListDigraph::Node &v) const {
		return m_section[v];
	}

	void setSection(const lemon::ListDigraph::Node &v, int section) {
		m_section[v] = section;
	}

	lemon::ListDigraph::Node getTarget() const {
		return m_target;
	}

	void setTarget(lemon::ListDigraph::Node tgt) {
		m_target = tgt;
	}

	bool isUnit(lemon::ListDigraph::Node v) const {
		return m_type[v] < NodeType::split;
	}

	bool isSplit(lemon::ListDigraph::Node v) const {
		return m_type[v] >= NodeType::split && m_type[v] < NodeType::join;
	}

	bool isJoin(lemon::ListDigraph::Node v) const {
		return m_type[v] >= NodeType::join;
	}

	int necessaryInArcs(lemon::ListDigraph::Node v) const {
		return m_type[v] >= NodeType::join ?
			m_type[v] - NodeType::join :
			countInArcs(*this, v);
	}

	void write(std::ostream &out = std::cout) {
		// Write lemon graph file to cout.
		lemon::DigraphWriter<lemon::ListDigraph>(*this, out)
			.nodeMap("type", m_type)
			.nodeMap("section", m_section)
			.node("target", m_target)
			.run();
	}

};

}
