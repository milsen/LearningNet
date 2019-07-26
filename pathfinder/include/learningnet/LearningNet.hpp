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

class LearningNet
{
private:
	lemon::ListDigraph m_graph;
	lemon::ListDigraph::NodeMap<int> m_section;
	lemon::ListDigraph::NodeMap<int> m_type;
	lemon::ListDigraph::Node m_target;

public:
	LearningNet(const std::string &network)
		: m_section{m_graph}
		, m_type{m_graph}
	{
		// Read lemon graph file given as arg.
		std::istringstream networkIss(network);
		lemon::DigraphReader<lemon::ListDigraph>(m_graph, networkIss)
			.nodeMap("type", m_type)
			.nodeMap("section", m_section)
			.node("target", m_target)
			.run();

	};

	virtual ~LearningNet ();

};

}
