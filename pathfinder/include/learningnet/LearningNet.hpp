#include <lemon/list_graph.h>
#include <lemon/concepts/digraph.h>
#include <lemon/lgf_reader.h>

namespace learningnet {

class Unit {
	private:
		int m_underlying;

	public:
		static constexpr int inactive = 0;
		static constexpr int active = 1;
		static constexpr int completed = 2;
		static constexpr int split = 10;
		static constexpr int join = 20;

		Unit(int v) : m_underlying(v) {}

		operator int() {
			return m_underlying;
		}
};

// TODO c'tor exception
// TODO management of unit types, join units
class LearningNet : public lemon::ListDigraph
{
private:
	lemon::ListDigraph::NodeMap<int> m_section;
	lemon::ListDigraph::NodeMap<int> m_type;
	lemon::ListDigraph::Node m_target;

public:
	LearningNet(const std::string &network)
		: m_section{*this}
		, m_type{*this}
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

	int getType(const lemon::ListDigraph::Node &v) {
		return m_type[v];
	}

	void setType(const lemon::ListDigraph::Node &v, int type) {
		m_type[v] = type;
	}

	int getSection(const lemon::ListDigraph::Node &v) {
		return m_section[v];
	}

	void setSection(const lemon::ListDigraph::Node &v, int section) {
		m_section[v] = section;
	}

	lemon::ListDigraph::Node getTarget() {
		return m_target;
	}

	void setTarget(lemon::ListDigraph::Node tgt) {
		m_target = tgt;
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
