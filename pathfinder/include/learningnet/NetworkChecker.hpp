#pragma once

#include <learningnet/Module.hpp>
#include <learningnet/LearningNet.hpp>
#include <lemon/connectivity.h> // for dag

namespace learningnet {

using namespace lemon;

class NetworkChecker : public Module
{
public:
	NetworkChecker(LearningNet &net) : Module() {
		call(net);
	}

	bool call(const LearningNet &net)
	{
		// TODO when conditions exist, there must exist a path to target
		// => topological sort?

		std::map<int, bool> sectionExists;
		for (ListDigraph::NodeIt v(net); v != INVALID; ++v) {
			if (net.isUnit(v)) {
				// Each section only occurs once.
				int section = net.getSection(v);
				if (sectionExists[section]) {
					failWithError("Section " + std::to_string(section) + " used multiple times.");
					return false;
				}
				sectionExists[section] = true;
			}

			// Necessary inarcs of a join are less than actual InArcs.
			int necessaryInArcs = net.getNecessaryInArcs(v);
			int actualInArcs = countInArcs(net, v);
			if (net.isJoin(v) && necessaryInArcs > actualInArcs) {
				failWithError("Join node has " + std::to_string(necessaryInArcs) +
					" necessary in-arcs but only " + std::to_string(actualInArcs) +
					" actual in-arcs.");
				return false;
			}
		}

		if (!dag(net)) {
			failWithError("Given network is not acyclic.");
			return false;
		}

		return true;
	}

};

}
