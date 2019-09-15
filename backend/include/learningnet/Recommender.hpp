#pragma once

#include <learningnet/Module.hpp>
#include <learningnet/LearningNet.hpp>

namespace learningnet {

class Recommender : public Module
{
public:
	Recommender() : Module() { }

	std::vector<lemon::ListDigraph::Node> call(const LearningNet &net,
		const lemon::ListDigraph::NodeMap<double> &nodeCosts,
		const lemon::ListDigraph::ArcMap<double> &edgeCosts,
		bool nextNotPath = true)
	{
		return nextNotPath ?
			std::vector<lemon::ListDigraph::Node>(recNext(net, nodeCosts, edgeCosts)) :
			recPath(net, nodeCosts, edgeCosts);

	}

	/**
	 * Assumes that active nodes are already marked as such via ActivitySetter.
	 */
	lemon::ListDigraph::Node recNext(const LearningNet &net,
		const lemon::ListDigraph::NodeMap<double> &nodeCosts,
		const lemon::ListDigraph::ArcMap<double> &edgeCosts)
	{
		/* out of all actives */
		return ;
	}

	/**
	 * Assumes that active nodes are already marked as such via ActivitySetter.
	 */
	std::vector<lemon::ListDigraph::Node> recPath(const LearningNet &net,
		const lemon::ListDigraph::NodeMap<double> &nodeCosts,
		const lemon::ListDigraph::ArcMap<double> &edgeCosts)
	{

	}
};

}
