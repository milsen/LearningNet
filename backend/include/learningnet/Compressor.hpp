#pragma once

#define LN_DEBUG_COMPRESSOR

#include <learningnet/LearningNet.hpp>
#include <learningnet/Module.hpp>

namespace learningnet {

//! Placeholder for highest test grades out of all out-arcs of a test node.
const std::string MAX_GRADE{"MAX_GRADE"};

/**
 * Conveys whether it was determined during the compression that the target is
 * reachable by every learner.
 */
enum class TargetReachability {
	Yes,    //!< Target is reachable.
	No,     //!< Target is not reachable.
	Unknown //!< It is unknown whether the target is reachable.
};

/**
 * Compresses a LearningNet in linear time.
 * The size of the net is decreased while making sure that a learning path can
 * be found for the same condition values and test grades as before.
 * If it can be determined that the target can be definitely (not) reached by
 * every learner, the compression is stopped and this result is stored.
 */
class Compressor : public Module
{
private:
	//! LearningNet that is compressed.
	LearningNet &m_net;

	//! Current sources, i.e. nodes at which the main loop is started.
	std::vector<lemon::ListDigraph::Node> m_sources;

	//! Successors of the currently looked at node.
	std::vector<lemon::ListDigraph::Node> m_succs;

	//! Indegree for each node.
	lemon::ListDigraph::NodeMap<int> m_indeg;

	//! For join nodes: From how many predecessors they were already visited.
	std::map<lemon::ListDigraph::Node, int> m_visitedFromHowManyPreds;

	//! The reachability of the target as detected during compression.
	TargetReachability m_targetReached;

	void xassert(bool asserted, const char *msg) {
#ifdef LN_DEBUG_COMPRESSOR
		if (!asserted) {
			std::cout << "assertion failed: " << msg << std::endl;
		}
#else
		(void) asserted;
		(void) msg;
		return;
#endif
	}

	/**
	 * @param v node whose successors should be counted
	 * @param n number of successors to check for
	 * @return whether \p v has at most \p n successors in #m_net
	 */
	bool hasAtMostNOutArcs(const lemon::ListDigraph::Node &v, int n) const
	{
		int count = 0;
		for (auto out : m_net.outArcs(v)) {
			(void) out;
			if (++count > n) {
				return false;
			}
		}
		return true;
	}

	/**
	 * @pre \p v must be a test node
	 * @param v node whose out-arcs with MAX_GRADE should be counted
	 * @return number of \p v's out-arcs with MAX_GRADE
	 */
	int numberOfOutArcsWithMaxGrade(const lemon::ListDigraph::Node &v) const
	{
		int count = 0;
		for (auto out : m_net.outArcs(v)) {
			if (m_net.getConditionBranch(out) == MAX_GRADE) {
				count++;
			}
		}
		return count;
	}

	/**
	 * @param v node with predecessor \p pred
	 * @param pred predecessor of \p v
	 * @return whether the only predecessor of \p v is \p pred
	 */
	bool hasOnlyOnePred(
		const lemon::ListDigraph::Node &v,
		const lemon::ListDigraph::Node &pred) const
	{
		bool result = true;
		for (auto in : m_net.inArcs(v)) {
			if (m_net.source(in) != pred) {
				result = false;
				break;
			}
		}
		return result;
	}

	/**
	 * @param v node with successor \p succ
	 * @param succ successor of \p v
	 * @return whether the only successor of \p v is \p succ
	 */
	bool hasOnlyOneSucc(
		const lemon::ListDigraph::Node &v,
		const lemon::ListDigraph::Node &succ) const
	{
		bool result = true;
		for (auto out : m_net.outArcs(v)) {
			if (m_net.target(out) != succ) {
				result = false;
				break;
			}
		}
		return result;
	}

	/**
	 * @param v node whose number of successors is checked
	 * @return whether \p v has only one successor
	 */
	bool hasAtMostOneSucc(const lemon::ListDigraph::Node &v) const
	{
		lemon::ListDigraph::OutArcIt out(m_net, v);
		return out == lemon::INVALID || hasOnlyOneSucc(v, m_net.target(out));
	}

	/**
	 * If the entry does exist, increment it, else set it to 1.
	 * @param m mapping of nodes to ints
	 * @param v node that is used as an entry of \p m
	 */
	void nodeMapIncrement(std::map<lemon::ListDigraph::Node, int> &m,
			const lemon::ListDigraph::Node &v) const
	{
		if (m.find(v) == m.end()) {
			m[v] = 1;
		} else {
			m[v]++;
		}
	}

	/**
	 * Sets #m_error, #m_failed and #m_targetReached to inform that the
	 * contraction of \p w into \p v is not possible.
	 *
	 * @pre \p w is a target node who is supposed to be contracted into the
	 * condition or test node \p v but the transfer of the target property is
	 * not possible
	 *
	 * @param v node with successor \p w
	 * @param w successor of \p v
	 */
	void failContraction(const lemon::ListDigraph::Node &v,
			const lemon::ListDigraph::Node &w)
	{
		// v is a condition node with more than w as a successor or
		// a test node with a not-highest grade leading to w.
		if (m_net.isCondition(v)) {
			// If a target node is contracted into a condition node
			// while the condition node has other successors, then
			// branches to those successors cannot reach the target.
			lemon::ListDigraph::InArcIt in(m_net, w);
			std::string thisBranch = m_net.getConditionBranch(in);
			std::string condId = std::to_string(m_net.getConditionId(v));

			failWithError("No path to target for condition branches:");
			for (auto out : m_net.outArcs(v)) {
				std::string thatBranch = m_net.getConditionBranch(out);
				if (thatBranch != thisBranch) {
					appendError(condId + ": " + thatBranch);
				}
			}
		}
		if (m_net.isTest(v)) {
			// If a target node is contracted into a test node over the
			// edge for a lower grade, the branches for the highest
			// grade cannot reach the target.
			failWithError("No path to target for the highest grade of "
				"the test " + std::to_string(m_net.getTestId(v)));
		}

		m_targetReached = TargetReachability::No;
	}

	/**
	 * Returns whether \p v and \p w can be double-contracted.
	 *
	 * A double contraction is allowed if
	 * - \p v is the only predecessor of \p w, and
	 * - \p w is the only successor of \p v, and
	 * - \p v is either a split, or
	 *   * \p v is a condition and \p w has only one necessary in-arc, or
	 *   * \p v is a test and \p w has a number of necessary in-arcs <= number
	 *     of out-arcs of \p v with the highest grade.
	 *
	 * @param v node with successor \p w
	 * @param w successor of \p v
	 * @return whether a double contraction of \p v and \p w is allowed
	 */
	bool doubleContractPossible(const lemon::ListDigraph::Node &v,
			const lemon::ListDigraph::Node &w) const
	{
		// Split-likes with join-successors whose in-arcs all come from the
		// respective split-like can be deleted.
		// If v is not a split, this can only be done if the join w is unlocked
		// by one condition branch/all test branches with the highest grade.
		return hasOnlyOneSucc(v, w)
			&& hasOnlyOnePred(w, v)
			&& (m_net.isSplit(v)
			|| (m_net.isCondition(v) && 1 >= m_net.getNecessaryInArcs(w))
			|| (m_net.isTest(v) &&
			numberOfOutArcsWithMaxGrade(v) >= m_net.getNecessaryInArcs(w))
		);
	}

	/**
	 * Returns whether \p w can be contracted into its predecessor because it
	 * has no other incoming edge and is generally not significant for the
	 * learning path checks.
	 *
	 * This is the case if \p w has an indegree of 1, an outdegree of at most 1
	 * and -- if \p w is a test node with an outgoing edge -- the outgoing edge
	 * has max grade. It might not have max grade if #preprocess() removed an
	 * edge with max grade from the learning net because it led to a join node
	 * which was deleted.
	 *
	 * @param w node to potentially be contracted into its predecessor
	 * @return whether \p w can be contracted into its predecessor
	 */
	bool contractSingleEdgePossible(const lemon::ListDigraph::Node &w) const
	{
		return m_indeg[w] == 1
		    && hasAtMostNOutArcs(w, 1)
		    && (!m_net.isTest(w) ||
		        numberOfOutArcsWithMaxGrade(w) == countOutArcs(m_net, w));
	}

	/**
	 * @param v node with successor \p w
	 * @param w successor of \p v
	 * @param in single arc from \p v to \p w
	 * @return whether the target property can be transfered from \p w to \p v
	 * if \p w is deleted
	 */
	bool targetIsTransferable(const lemon::ListDigraph::Node &v,
			const lemon::ListDigraph::Node &w,
			const lemon::ListDigraph::InArcIt &in) const
	{
		return (hasOnlyOneSucc(v, w) && !m_net.isTest(v))
			|| m_net.isSplit(v)
			|| (m_net.isTest(v) && m_net.getConditionBranch(in) == MAX_GRADE);
	}

	/**
	 * Preprocesses the learning net #m_net by removing non-condition and
	 * non-test source nodes (with indegree 0) until all sources are only
	 * conditions and tests with more than one out-arc.
	 *
	 * The preprocessing is based on a topological sort that deletes all visited
	 * nodes (necessary in-arcs of join nodes are adjusted as well).
	 * Assigns the nodes that are sources after preprocessing to #m_sources.
	 * Assigns the indegree of every node after preprocessing to #m_indeg.
	 * Returns immediately if the target node is found.
	 *
	 * @return whether the target was found during topological sorting
	 */
	bool preprocess()
	{
		// Collect sources: nodes with indegree 0.
		std::vector<lemon::ListDigraph::Node> initialSources;
		for (auto v : m_net.nodes()) {
			m_indeg[v] = countInArcs(m_net, v);
			if (m_net.isSource(v)) {
				initialSources.push_back(v);
			}
		}

		while (!initialSources.empty()) {
			lemon::ListDigraph::Node v = initialSources.back();
			initialSources.pop_back();

			// If target can be reached, directly return.
			if (m_net.isTarget(v)) {
				return true;
			}

			if ((m_net.isCondition(v) || m_net.isTest(v)) &&
				!hasAtMostNOutArcs(v, 1)) {
				m_sources.push_back(v);
			} else {
				for (auto out : m_net.outArcs(v)) {
					lemon::ListDigraph::Node w = m_net.target(out);
					m_indeg[w]--;

					// Reduce necessary in-arcs for joins.
					if (m_net.isJoin(w)) {
						int oldInArcs = m_net.getNecessaryInArcs(w);
						if (oldInArcs > 0) {
							m_net.setNecessaryInArcs(w, oldInArcs - 1);
							if (oldInArcs == 1) {
								initialSources.push_back(w);
							}
						}

					} else {
						xassert(m_indeg[w] == 0,
							"preprocess(): m_indeg[w] > 0 for non-joins after "
							"decreasing their indegree.");
						initialSources.push_back(w);
					}
				}

				// Erase v unless v is a join with a condition predecessor/test
				// predecessor with non-highest grade.
				// In the latter case only erase its out-arc and in-arcs from
				// non-problematic predecessors. This way, the branches of the
				// problematic predecessor stay intact.
				if (m_net.isJoin(v)) {
					bool hasPredNonTransferable{false};
					lemon::ListDigraph::InArcIt next(m_net, v);
					for (lemon::ListDigraph::InArcIt in(m_net, v);
						in != lemon::INVALID; in = next) {
						next = in;
						++next;
						if (!targetIsTransferable(m_net.source(in), v, in)) {
							m_net.erase(in);
						} else {
							hasPredNonTransferable = true;
						}
					}
					if (hasPredNonTransferable) {
						// This is at most one out-arc.
						for (auto out : m_net.outArcs(v)) {
							m_net.erase(out);
						}
						m_net.setNecessaryInArcs(v, 1);
					} else {
						m_net.erase(v);
					}
				} else {
					m_net.erase(v);
				}
			}
		}

		return false;
	}

	/**
	 * Contract \p v into its successor \p w and push w's succs to #m_succs.
	 *
	 * The target property is handed from \p w to \p v if possible.
	 * #m_indeg and #m_visitedFromHowManyPreds are updated as well.
	 *
	 * @param v node with successor \p w
	 * @param w node to contract into its predecessor \p v
	 */
	void contract(
		const lemon::ListDigraph::Node &v,
		const lemon::ListDigraph::Node &w)
	{
		// Push succs to succs.
		for (auto out : m_net.outArcs(w)) {
			m_succs.push_back(m_net.target(out));
		}

		// The target property can be transfered from v to w if v is a
		// unit/split/join node or v is a condition node with only one successor
		// or v is a test with the highest grade leading to w.
		lemon::ListDigraph::InArcIt in(m_net, w);
		bool targetTransferable = targetIsTransferable(v, w, in);

		// Transfer target property from w to v if w is the target.
		if (m_net.isTarget(w)) {
			if (targetTransferable) {
				m_net.setTarget(v);
			} else {
				failContraction(v, w);
			}
		}

		// Contract w into v.
		if (m_indeg[w] == 1 && hasAtMostNOutArcs(w, 1)) {
			// If w is a unit node, a join with only one in-arc or a split-like
			// with at most one out-arc, contract w into v.
			lemon::ListDigraph::OutArcIt out(m_net, w);
			if (out != lemon::INVALID) {
				// Do the contraction manually, do not use m_net.contract()
				// directly because the condition/test branch would get lost.
				lemon::ListDigraph::InArcIt in(m_net, w);
				m_net.changeTarget(in, m_net.target(out));
				m_net.erase(w);
			} else {
				// If w has no out-edge, just erase it, unless:
				// If v is a condition with multiple succs, then do not erase w,
				// since w is proof that the condition v does not lead to a
				// target for every branch.
				// If v is a test with the highest grade leading to w, then do
				// not erase w, since w is proof that the highest grade does not
				// lead to the target.
				if (targetTransferable) {
					m_net.erase(w);
					if (m_net.isCondition(v)) {
						m_net.setType(v, NodeType::split);
					}
				}
			}
		} else if (m_net.isJoin(v) && m_net.isJoin(w)) {
			m_indeg[v] += m_indeg[w] - 1;
			m_visitedFromHowManyPreds[v] += m_visitedFromHowManyPreds[w] - 1;
			m_net.contract(v, w);
		} else { // w has more than one out-arc.
			// A condition or test node with more than one out-arc should not be
			// contracted into its predecessor. Hence, w must be a split.
			// Since splits with multiple out-arcs are only contracted into
			// other split, v must also be a split.
			xassert(m_net.isSplit(v), "contract(): v should be a split");
			xassert(m_net.isSplit(w), "contract(): w should be a split");
			m_net.contract(v, w);
		}
	}

	/**
	 * Remove split-like node \p v and its join-successor \p w,
	 * connecting the predecessor of \p v to the successor of \p w.
	 * If \p v has a predecessor, that predecessor will be given back to
	 * continue the main loop with the successor of \p w in #m_succs.
	 * If not, push the successor of \p w to #m_sources.
	 *
	 * @param v node with successor \p w
	 * @param w node to contract into its predecessor \p v
	 * @return predecessor of \p v if it exists (else an invalid dummy node)
	 */
	lemon::ListDigraph::Node doubleContract(
		const lemon::ListDigraph::Node &v,
		const lemon::ListDigraph::Node &w)
	{
		// Return if it was the target (do not delete the target in this
		// case such that the LearningNet is still valid).
		xassert(m_succs.empty(), "doubleContract(): m_succs should be empty");
		m_succs.clear();
		if (m_indeg[v] == 0) {
			if (m_net.isTarget(v) || m_net.isTarget(w)) {
				m_targetReached = TargetReachability::Yes;
			} else {
				// Before deletion of w, decrease indegree of w's succ.
				lemon::ListDigraph::OutArcIt out(m_net, w);
				if (out != lemon::INVALID) {
					lemon::ListDigraph::Node succ = m_net.target(out);
					m_indeg[succ]--;
					if (m_net.isJoin(succ)) {
						m_net.setNecessaryInArcs(succ,
							m_net.getNecessaryInArcs(succ) - 1);
					}

					if (m_indeg[succ] == 0) {
						if (m_net.isTarget(succ)) {
							m_targetReached = TargetReachability::Yes;
						} else {
							m_sources.push_back(succ);
						}
					}
				}
				m_net.erase(v);
				m_net.erase(w);
			}

			// Return dummy value: m_succs is empty, so the main loop continues.
			return lemon::INVALID;
		} else { // m_indeg[v] == 1
			xassert(m_indeg[v] == 1, "doubleContract(): m_indeg[v] > 1");
			// Backtrack with compression, contract v into its predecessor.
			lemon::ListDigraph::OutArcIt out(m_net, w);
			lemon::ListDigraph::InArcIt in(m_net, v);
			lemon::ListDigraph::Node pred = m_net.source(in);

			// Move target property where possible.
			if (m_net.isTarget(v) || m_net.isTarget(w)) {
				if (targetIsTransferable(pred, v, in)) {
					m_net.setTarget(pred);
				} else {
					failContraction(pred, v);
				}
			}

			// Change target of v's incoming edge to w's successor.
			if (out != lemon::INVALID) {
				lemon::ListDigraph::Node succ = m_net.target(out);
				m_net.changeTarget(in, succ);
				m_succs.push_back(succ);
			}

			// Erase v and w.
			// If one of them was the target but the target property could not
			// be transfered, the learning net is invalid anyway. In this case,
			// do not delete the target to prevent later errors.
			if (!m_net.isTarget(v)) {
				m_net.erase(v);
			}
			if (!m_net.isTarget(w)) {
				m_net.erase(w);
			}

			return pred;
		}
	}

	/**
	 * Check whether it is possible to contract \p w into its predecessor \p v.
	 * If yes, do so, else push \p w to #m_sources.
	 *
	 * @param v node with successor \p w
	 * @param w node to contract into its predecessor \p v
	 * @return new node to be used as source
	 */
	lemon::ListDigraph::Node tryContraction(
			const lemon::ListDigraph::Node &v,
			const lemon::ListDigraph::Node &w)
	{
		// Contract nodes depending on their type.
		if ((contractSingleEdgePossible(w)) ||
		    (m_net.isSplit(w) && m_net.isSplit(v))) {
			// If w is a unit node, a join with only one in-arc or a split-like
			// with at most one out-arc, just contract it into its predecessor
			// v. Also combine adjacent splits.
			contract(v, w);
		} else if (m_net.isJoin(w)) {
			nodeMapIncrement(m_visitedFromHowManyPreds, w);

			// Combine adjacent 1-joins or *-joins.
			if (m_net.isJoin(v)) {
				int necArcsV = m_net.getNecessaryInArcs(v);
				int necArcsW = m_net.getNecessaryInArcs(w);
				if (necArcsV == 1 && necArcsW == 1) {
					contract(v, w);
				} else if (necArcsV == m_indeg[v] && necArcsW == m_indeg[w]) {
					contract(v, w);
					m_net.setNecessaryInArcs(v, necArcsV + necArcsW - 1);
				} else {
					m_sources.push_back(w);
				}
			} else if (m_net.isSplitLike(v)) {
				// Check contraction of v->w only if w has been visited via all
				// its in-arcs to avoid unnecessary checks.
				if (m_visitedFromHowManyPreds[w] == m_indeg[w]) {
					if (doubleContractPossible(v,w)) {
						return doubleContract(v, w);
					} else {
						m_sources.push_back(w);
					}
				}
			} else { // w is a join that cannot be contracted.
				m_sources.push_back(w);
			}
		} else {
			// Push nodes that are not contracted to sources.
			m_sources.push_back(w);
		}

		return v;
	}

	/**
	 * Compresses #m_net and notes in #m_targetReached whether the compression
	 * already determined that the target can be reached.
	 */
	void compress()
	{
		// Remove non-condition sources until all sources are only conditions.
		if (preprocess()) {
			m_targetReached = TargetReachability::Yes;
			return;
		}

		// For each source v:
		while (!m_sources.empty()) {
			lemon::ListDigraph::Node v = m_sources.back();
			m_sources.pop_back();

			// Add unvisited successors to the stack.
			for (auto out : m_net.outArcs(v)) {
				m_succs.push_back(m_net.target(out));
			}

			// Get successor w of v. Try to contract v->w such that w is removed.
			while (!m_succs.empty()) {
				lemon::ListDigraph::Node w = m_succs.back();
				m_succs.pop_back();

				// The contraction might remove not just w but v itself.
				// In that case, tryContraction returns the predecessor of v to
				// be used as the new v and updates m_succs.
				v = tryContraction(v, w);

				// If we found out during contraction whether the target is
				// reachable in all cases, return this result.
				if (m_targetReached != TargetReachability::Unknown) {
					return;
				}
			}
		}
	}

public:
	/**
	 * Constructs a Compressor and compresses the given LearningNet.
	 *
	 * @param net LearningNet to be compressed
	 */
	Compressor(LearningNet &net)
		: m_net{net}
		, m_indeg{net, 0}
		, m_targetReached{TargetReachability::Unknown}
	{
		compress();
	}

	/**
	 * @return whether the compression determined that the target can be reached
	 * by every learner
	 */
	TargetReachability getResult() {
		return m_targetReached;
	}
};

}
