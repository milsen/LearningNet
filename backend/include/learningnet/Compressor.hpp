#pragma once

#include <learningnet/LearningNet.hpp>

namespace learningnet {

/**
 * Compresses a LearningNet in linear time.
 */
class Compressor
{
private:
		// Contraction function.
		void contract(LearningNet &net,
			std::vector<lemon::ListDigraph::Node> &succs,
			const lemon::ListDigraph::Node &v,
			const lemon::ListDigraph::Node &w)
		{
			// Add succ to succs.
			for (lemon::ListDigraph::OutArcIt out(net, w); out != lemon::INVALID; ++out) {
				succs.push_back(net.target(out));
			}

			// Transfer target property if needed, then contract w into v.
			if (net.isTarget(w)) {
				net.setTarget(v);
			}
			net.contract(v, w);
		}

		bool hasAtMostNSuccs(const LearningNet &net,
			const lemon::ListDigraph::Node &v,
			int n)
		{
			int count = 0;
			for (lemon::ListDigraph::OutArcIt out(net, v); out != lemon::INVALID; ++out) {
				if (++count > n) {
					return false;
				}
			}
			return true;
		}

		/**
		 * If the entry does exist, increment it, else set it to 1.
		 */
		void nodeMapIncrement(std::map<lemon::ListDigraph::Node, int> m,
				lemon::ListDigraph::Node &v)
		{
			if (m.find(v) == m.end()) {
				m[v] = 1;
			} else {
				m[v]++;
			}
		}

public:
	/**
	 * @return whether the compression already determined that the target can be reached.
	 */
	bool compress(LearningNet &net)
	{
		std::vector<lemon::ListDigraph::Node> succs;
		std::map<lemon::ListDigraph::Node, int> visitedFromHowManyPreds; // for join nodes

		// Collect sources: nodes with indegree 0.
		std::vector<lemon::ListDigraph::Node> initialSources;
		for (lemon::ListDigraph::NodeIt v(net); v != lemon::INVALID; ++v) {
			if ((!net.isJoin(v) && hasAtMostNSuccs(net, v, 0))
			   || net.isUnlockedJoin(v)) {
				initialSources.push_back(v);
			}
		}

		// Remove non-condition sources until all sources are only conditions.
		std::vector<lemon::ListDigraph::Node> sources;
		while (!initialSources.empty()) {
			lemon::ListDigraph::Node v = initialSources.back();
			initialSources.pop_back();

			if (net.isTarget(v)) {
				return true;
			}

			if (!net.isCondition(v)) {
				// TODO joins
				for (lemon::ListDigraph::OutArcIt out(net, v); out != lemon::INVALID; ++out) {
					initialSources.push_back(net.target(out));
				}
				net.erase(v);
			} else {
				sources.push_back(v);
			}
		}

		// Get univisited node v. Try to contract v->w such that w is removed.
		while (!sources.empty()) {
			lemon::ListDigraph::Node v = sources.back();
			sources.pop_back();

			// Add unvisited successors to the stack.
			for (lemon::ListDigraph::OutArcIt out(net, v); out != lemon::INVALID; ++out) {
				succs.push_back(net.target(out));
			}

			while (!succs.empty()) {
				lemon::ListDigraph::Node w = succs.back();
				succs.pop_back();

				// Contract nodes depending on their type.
				// If a node is a unit, just contract it with its predecessor and continue.
				if (net.isUnit(w)) {
					contract(net, succs, v, w);
				} else if (net.isSplit(w)) {
					// Combine adjacent splits.
					if (net.isSplit(v)) {
						contract(net, succs, v, w);
					}
				} else if (net.isJoin(w)) {
					nodeMapIncrement(visitedFromHowManyPreds, w);

					// Combine adjacent 1-joins or *-joins.
					if (net.isJoin(v)) {
						int necArcsV = net.getNecessaryInArcs(v);
						int necArcsSucc = net.getNecessaryInArcs(w);
						if ((necArcsV == 1 && necArcsSucc == 1)
						 || (necArcsV == countInArcs(net, v) &&
							necArcsSucc == countInArcs(net, w))) {
							contract(net, succs, v, w);
						}
					} else if ((net.isSplit(v) || net.isCondition(v))
						&& visitedFromHowManyPreds[w] == countInArcs(net, w)) {
						// Combine splits/conditions with join-successors whose
						// in-arcs all come from the respective split/condition.
						// Only check this after all in-arcs were visited such
						// that it is ensured that they are combined if that is
						// possible at all.
						std::map<lemon::ListDigraph::Node, int> joinToInArcsFromSplit;
						for (lemon::ListDigraph::InArcIt in(net, w); in != lemon::INVALID; ++in) {
							lemon::ListDigraph::Node u = net.source(in);
							if (net.isJoin(u)) {
								nodeMapIncrement(joinToInArcsFromSplit, u);
							}
						}
					}
				}
			}

			// Remove source units / splits with only one in-arc after
			// contracting them with joins.
			// Even if they are targets, they can be definitely reached.
			// If v is not a source, it'll be contracted later with its pred.
			if ((net.isUnit(v) || (net.isSplit(v) && hasAtMostNSuccs(net, v, 1)))
				&& net.isSource(v)) {
				net.erase(v);
			}

			for (lemon::ListDigraph::Node succ : succs) {
				sources.push_back(succ);
			}
		}

		return false;
	}

};

}
