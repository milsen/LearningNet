<?php

require 'vendor/autoload.php'; // The autoloader provided by composer

namespace LearningNet\Ranking;

class PathRecommender
{
    public function __construct()
    {
    }

    public function recommendedPath($network)
    {
        // NEEDED for Dijsktra: non-negative edge costs!

        // Shortest Path in DAG using topological sorting in linear time
        // ... Does not really work because learning path may make the graph cyclic

        // Cormen, Thomas H.; Leiserson, Charles E.; Rivest, Ronald L.; Stein,
        // Clifford (2009) [1990]. Introduction to Algorithms (3rd ed.). MIT
        // Press and McGraw-Hill. pp. 655–657. ISBN 0-262-03384-4.

        // First variables.
        $numTargetNodes = count($network->getTargetNodes());
        $priority = array();
        $predecessorInPath = array();

        // Topological sorting.
        $topSort = new TopologicalSort($network);
        if ($topSort->failed()) {
            return false;
        }
        $sortedNodes = $topSort->getSortedNodes();
        $source = reset($sortedNodes);

        // Initialize priorities and predecessors.
        // TODO all units, not just elementary ones
        foreach ($network->getElementaryUnits() as $v) {
            $priority[$v] = $v === $source ? 0 : INF;
            $predecessorInPath[$v] = null;
        }

        // Calculation.
        $foundTargetNodes = 0;

        foreach ($sortedNodes as $v) {
            // Break if all target nodes found.
            if ($v->isTargetNode()) {
                $foundTargetNodes++;
                if ($foundTargetNodes >= $numTargetNodes)) {
                    break;
                }
            }

            // For all "neighbours":
            // skip already completed units
            foreach ($this->getFollowingNodes($v) as $w) {
                $edgeVal = $this->evaluate($v, $w);
                $newPriority = $priority[$v] + $edgeVal;

                // If old priority is worse (lower) than the new one, update it.
                if ($priority[$w] < $newPriority) {
                    $priority[$w] = $newPriority;
                    $predecessorInPath[$w] = $v;
                }
            }
        }
    }

    /**
     * Nodes that can be reached after completing this node v.
     */
    private function getFollowingNodes($v)
    {
        $successors = array();
        foreach ($v->getActiveSuccessors() as $w) {
            if ($w instanceof Join) {

            }
        }
    }

    private function evaluate($v, $w)
    {
        $value = 0;
        foreach ($criteria as $criterium) {
            if ($criterium->alreadyCalculated($v, $w)) {
            }
            $value += $criterium->evaluate($v, $w) * $this->weight[$criterium];
        }

        return $value;
    }
}
