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

	/**
	 * @param net learning net containing node \p v.
	 * @param v node whose successors should be counted
	 * @param n number of successors to check for
	 * @return Whether \p v has at most \p n successors in \p net.
	 */
	bool hasAtMostNOutArcs(const LearningNet &net,
		const lemon::ListDigraph::Node &v,
		int n)
	{
		int count = 0;
		for (auto out : net.outArcs(v)) {
			(void) out;
			if (++count > n) {
				return false;
			}
		}
		return true;
	}

	bool hasOnlyOnePred(const LearningNet &net,
		const lemon::ListDigraph::Node &v,
		const lemon::ListDigraph::Node &pred)
	{
		bool result = true;
		for (auto in : net.inArcs(v)) {
			if (net.source(in) != pred) {
				result = false;
				break;
			}
		}
		return result;
	}

	bool hasOnlyOneSucc(const LearningNet &net,
		const lemon::ListDigraph::Node &v,
		const lemon::ListDigraph::Node &succ)
	{
		bool result = true;
		for (auto out : net.outArcs(v)) {
			if (net.target(out) != succ) {
				result = false;
				break;
			}
		}
		return result;
	}

	/**
	 * If the entry does exist, increment it, else set it to 1.
	 * @param m mapping of nodes to ints
	 * @param v node that is used as an entry of \p m
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

	// Contraction function.
	TargetReachability contract(LearningNet &net,
		std::vector<lemon::ListDigraph::Node> &succs,
		const lemon::ListDigraph::Node &v,
		const lemon::ListDigraph::Node &w)
	{
		// Push succs to succs.
		for (auto out : net.outArcs(w)) {
			succs.push_back(net.target(out));
		}

		// Remember condition branch in case v is a condition.
		lemon::ListDigraph::InArcIt in(net, w);
		std::string branch = net.getConditionBranch(in);

		// Transfer target property from w to v if w is the target.
		if (net.isTarget(w)) {
			// TODO Instead of hasAtMostNOutArcs use hasAtMostNSuccs (there is a
			// difference if multiple arcs lead to the same succ).
			// Is this actually guaranteed if contraction is done?
			if (!net.isCondition(v) || hasOnlyOneSucc(net, v, w)) {
				net.setTarget(v);
			} else {
				// If a target node is contracted into a condition node while
				// the condition node has other successors, then branches to
				// those other successors cannot reach the target.
				return TargetReachability::No;
			}
		}

		// Contract w into v.
		if (hasAtMostNOutArcs(net, w, 1)) {
			// Just contract w into v but remember the new arc.
			lemon::ListDigraph::OutArcIt out(net, w);
			if (out != lemon::INVALID) {
				lemon::ListDigraph::Node wTgt = net.target(out);
				lemon::ListDigraph::Arc newArc = net.addArc(v, wTgt);

				// Assign condition branch to new arc if needed.
				if (net.isCondition(v)) {
					net.setConditionBranch(newArc, branch);
				}

				net.erase(w);
			} else {
				// If w has no out-edge, just erase it.
				// If v is a condition with multiple succs, then do not erase w,
				// since w is proof that the condition v does not lead to a
				// target for every branch.
				if (!net.isCondition(v) || hasOnlyOneSucc(net, v, w)) {
					net.erase(w);
					if (net.isCondition(v)) {
						net.setType(v, NodeType::split);
					}
				}
			}
		} else {
			// TODO assert v is not a condition here
			net.contract(v, w);
		}

		return TargetReachability::Unknown;
	}

	/**
	 * Preprocesses the given learning net by removing non-condition source
	 * nodes (nodes with indegree 0) until all sources are only conditions.
	 * The preprocessing is based on a topological sort that deletes all visited
	 * nodes (necessary in-arcs of join nodes are adjusted as well).
	 * Returns immediately if the target node is found.
	 *
	 * @param net learning net to be preprocessed
	 * @param indeg indegree of every node after preprocessing
	 * @param sources is assigned a list of condition nodes with indegree 0
	 * @return whether the target was found during topological sorting
	 */
	bool preprocess(LearningNet &net,
		lemon::ListDigraph::NodeMap<int> &indeg,
		std::vector<lemon::ListDigraph::Node> &sources)
	{
		// Collect sources: nodes with indegree 0.
		std::vector<lemon::ListDigraph::Node> initialSources;
		for (auto v : net.nodes()) {
			indeg[v] = countInArcs(net, v);
			if (indeg[v] == 0 || net.isCompletedJoin(v)) {
				initialSources.push_back(v);
			}
		}

		while (!initialSources.empty()) {
			lemon::ListDigraph::Node v = initialSources.back();
			initialSources.pop_back();

			// If target can be reached, directly return.
			if (net.isTarget(v)) {
				return true;
			}

			if (net.isCondition(v)) {
				sources.push_back(v);
			} else {
				for (auto out : net.outArcs(v)) {
					lemon::ListDigraph::Node w = net.target(out);
					indeg[w]--;

					// Reduce necessary in-arcs for joins.
					if (net.isJoin(w)) {
						net.setNecessaryInArcs(w, std::max(net.getNecessaryInArcs(w) - 1, 0));
					}

					if (indeg[w] == 0) {
						initialSources.push_back(w);
					}
				}
				net.erase(v);
			}
		}

		return false;
	}


public:

	/**
	 * @return whether the compression already determined that the target can be
	 * reached.
	 */
	TargetReachability compress(LearningNet &net)
	{
		std::map<lemon::ListDigraph::Node, int> visitedFromHowManyPreds; // for join nodes
		std::vector<lemon::ListDigraph::Node> succs;

		// Remove non-condition sources until all sources are only conditions.
		lemon::ListDigraph::NodeMap<int> indeg{net, 0};
		std::vector<lemon::ListDigraph::Node> sources;
		if (preprocess(net, indeg, sources)) {
			return TargetReachability::Yes;
		}

		// For each source v:
		while (!sources.empty()) {
			lemon::ListDigraph::Node v = sources.back();
			sources.pop_back();

			// Add unvisited successors to the stack.
			for (auto out : net.outArcs(v)) {
				succs.push_back(net.target(out));
			}

			// Get successor w of v. Try to contract v->w such that w is removed.
			while (!succs.empty()) {
				lemon::ListDigraph::Node w = succs.back();
				succs.pop_back();
				TargetReachability afterContraction = TargetReachability::Unknown;

				// Contract nodes depending on their type.
				// If a node is a unit, just combine it with its predecessor.
				if (indeg[w] <= 1 && hasAtMostNOutArcs(net, w, 1)) {
					afterContraction = contract(net, succs, v, w);
				} else if (net.isSplit(w) || net.isTest(w)) {
					// Combine adjacent splits.
					if (net.isSplit(v) || net.isTest(w)) {
						afterContraction = contract(net, succs, v, w);
					}
				} else if (net.isJoin(w)) {
					nodeMapIncrement(visitedFromHowManyPreds, w);

					// Combine adjacent 1-joins or *-joins.
					if (net.isJoin(v)) {
						int necArcsV = net.getNecessaryInArcs(v);
						int necArcsSucc = net.getNecessaryInArcs(w);
						if (necArcsV == 1 && necArcsSucc == 1) {
							indeg[v] += indeg[w] - 1;
							afterContraction = contract(net, succs, v, w);
						} else if (necArcsV == indeg[v] && necArcsSucc == indeg[w]) {
							indeg[v] += indeg[w] - 1;
							afterContraction = contract(net, succs, v, w);
							net.setNecessaryInArcs(v, indeg[v] + indeg[w] - 1);
						}
					} else if ((net.isSplit(v) || net.isCondition(v) || net.isTest(v))
						&& visitedFromHowManyPreds[w] == indeg[w]) {
						// Combine splits/conditions with join-successors whose
						// in-arcs all come from the respective split/condition.
						// Only check this after all in-arcs were visited such
						// that it is ensured that they are combined if that is
						// possible at all.
						if (hasOnlyOnePred(net, w, v)) {
							contract(net, succs, v, w);
						}
					} else { // w is a join that cannot be contracted.
						sources.push_back(w);
					}
				} else { // w is a condition with more than one succ.
					// TODO assert w is a condition
					sources.push_back(w);
				}

				// If we found out during contraction whether the target is
				// reachable via all conditions, return this result.
				if (afterContraction != TargetReachability::Unknown) {
					return afterContraction;
				}
			}

			// Remove splits / conditions / test nodes with only one in-arc
			// after contracting them with joins.
			if (indeg[v] <= 1 && hasAtMostNOutArcs(net, v, 1)) {
				// TODO assert v is not a unit node because unit nodes would not
				// be v at any point

				// If v is a source, delete it.
				// Return if it was the target (do not delete the target in this
				// case such that the LearningNet is still valid).
				if (indeg[v] == 0) {
					if (net.isTarget(v)) {
						return TargetReachability::Yes;
					} else {
						net.erase(v);
					}
				} else { // indeg[v] == 1
					// Contract v into its predecessor.
					lemon::ListDigraph::InArcIt in(net, v);
					net.contract(net.source(in), v);
				}
			}
		}

		return TargetReachability::Unknown;
	}

};

}
