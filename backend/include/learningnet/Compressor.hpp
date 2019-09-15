#pragma once

#include <learningnet/LearningNet.hpp>

namespace learningnet {

enum class TargetReachability {
	Yes,
	No,
	Unknown
};

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
		std::cout << "contract" << net.id(v) << " " << net.id(w) << std::endl;
		// Add succ to succs.
		for (lemon::ListDigraph::OutArcIt out(net, w); out != lemon::INVALID; ++out) {
			succs.push_back(net.target(out));
		}

		// Remember condition branch in case v is a condition.
		lemon::ListDigraph::InArcIt in(net, w);
		std::string branch = net.getConditionBranch(in);

		// Transfer target property if needed, then contract w into v.
		// Conditions may only get the target property from their successor
		// if they are combined with a join. Otherwise they may have
		// branches leading to other successors which would not reach the
		// target.
		// TODO what if conditions only have one edge left?
		if (net.isTarget(w)) { //} && (!net.isCondition(v) || net.isJoin(w))) {
			net.setTarget(v);
		}

		if (net.isUnit(w)) {
			// Just contract w into v but remember the new arc.
			lemon::ListDigraph::OutArcIt out(net, w);
			if (out != lemon::INVALID) {
				lemon::ListDigraph::Node wTgt = net.target(out);
				lemon::ListDigraph::Arc newArc = net.addArc(v, wTgt);

				// Assign condition branch to new arc if needed.
				if (net.isCondition(v)) {
					net.setConditionBranch(newArc, branch);
				}
			}

			net.erase(w);
		} else {
			net.contract(v, w);
		}
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
	void nodeMapIncrement(std::map<lemon::ListDigraph::Node, int> &m,
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
	TargetReachability compress(LearningNet &net)
	{
		std::vector<lemon::ListDigraph::Node> succs;
		std::map<lemon::ListDigraph::Node, int> visitedFromHowManyPreds; // for join nodes
		lemon::ListDigraph::NodeMap<int> indeg{net, 0};

		// Collect sources: nodes with indegree 0.
		std::vector<lemon::ListDigraph::Node> initialSources;
		for (lemon::ListDigraph::NodeIt v(net); v != lemon::INVALID; ++v) {
			indeg[v] = countInArcs(net, v);
			if (indeg[v] == 0) {
				initialSources.push_back(v);
			}
		}

		// Remove non-condition sources until all sources are only conditions.
		std::vector<lemon::ListDigraph::Node> sources;
		while (!initialSources.empty()) {
			lemon::ListDigraph::Node v = initialSources.back();
			initialSources.pop_back();

			if (net.isTarget(v)) {
				return TargetReachability::Yes;
			}

			if (net.isCondition(v)) {
				sources.push_back(v);
			} else {
				for (lemon::ListDigraph::OutArcIt out(net, v); out != lemon::INVALID; ++out) {
					lemon::ListDigraph::Node w = net.target(out);
					indeg[w]--;
					if (indeg[w] == 0) {
						initialSources.push_back(w);
					}
				}
				net.erase(v);
			}
		}

		// For each source v:
		while (!sources.empty()) {
			lemon::ListDigraph::Node v = sources.back();
			sources.pop_back();

			// Add unvisited successors to the stack.
			for (lemon::ListDigraph::OutArcIt out(net, v); out != lemon::INVALID; ++out) {
				succs.push_back(net.target(out));
			}

			// Get successor w of v. Try to contract v->w such that w is removed.
			while (!succs.empty()) {
				lemon::ListDigraph::Node w = succs.back();
				std::cout << net.id(v) << " -> " << net.id(w) << std::endl;
				succs.pop_back();

				// Contract nodes depending on their type.
				// If a node is a unit, just combine it with its predecessor.
				if (net.isUnit(w)) {
					contract(net, succs, v, w);
				} else if (net.isSplit(w) || net.isTest(w)) {
					// Combine adjacent splits.
					if (net.isSplit(v) || net.isTest(w)) {
						contract(net, succs, v, w);
					}
				} else if (net.isJoin(w)) {
					nodeMapIncrement(visitedFromHowManyPreds, w);

					// Combine adjacent 1-joins or *-joins.
					if (net.isJoin(v)) {
						int necArcsV = net.getNecessaryInArcs(v);
						int necArcsSucc = net.getNecessaryInArcs(w);
						if (necArcsV == 1 && necArcsSucc == 1) {
							contract(net, succs, v, w);
						} else if (necArcsV == indeg[v] && necArcsSucc == indeg[w]) {
							contract(net, succs, v, w);
							net.setNecessaryInArcs(v, indeg[v] + indeg[w] - 1);
						}
					} else if ((net.isSplit(v) || net.isCondition(v) || net.isTest(v))
						&& visitedFromHowManyPreds[w] == indeg[w]) {
						// Combine splits/conditions with join-successors whose
						// in-arcs all come from the respective split/condition.
						// Only check this after all in-arcs were visited such
						// that it is ensured that they are combined if that is
						// possible at all.
						bool joinInArcsFromSplit = true;
						for (lemon::ListDigraph::InArcIt in(net, w); in != lemon::INVALID; ++in) {
							if (net.source(in) != v) {
								joinInArcsFromSplit = false;
								break;
							}
						}
						if (joinInArcsFromSplit) {
							/* contract(net, succs, v, w); */
						}
					} else {
						sources.push_back(w);
					}
				} else {
					sources.push_back(w);
				}

			}

			// Remove source units and splits / conditions / test with only one
			// out-arc after contracting them with joins.
			// Even if they are targets, they can be definitely reached.
			// If v is not a source, it'll be contracted later with its pred.
			if (indeg[v] <= 1 && hasAtMostNSuccs(net, v, 1)) {
				if (indeg[v] == 0) {
					net.erase(v);
					if (net.isTarget(v)) {
						return TargetReachability::Yes;
					}
				} else {
					lemon::ListDigraph::InArcIt in(net, v);
					net.contract(net.source(in), v);
				}
			}
		}

		return TargetReachability::Unknown;
	}

};

}
