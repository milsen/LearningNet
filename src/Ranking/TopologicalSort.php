<?php

require 'vendor/autoload.php'; // The autoloader provided by composer

namespace LearningNet\Ranking;

class TopologicalSort
{
    private $failed;

    private $sortedNodes;

    public function __construct($network)
    {
        $this->failed = false;
        $this->sortedNodes = array();
        $this->topologicalSort($network);
    }

    private function topologicalSort(&$network)
    {
        // Get indegree for each node, store nodes with indegree 0.
        $zeroIndegreeNodes = array();
        $indegree = array()
        foreach ($network->getNodes() as $v) {
            $indegree[$v] = $v->indegree();
            if ($indegree[$v] === 0) {
                $zeroIndegreeNodes[] = $v;
            }
        }

        // While there are still nodes with indegree 0:
        while (!empty($zeroIndegreeNodes)) {
            // Put next node in list of sorted nodes.
            $v = array_pop($zeroIndegreeNodes);
            $sortedNodes[] = $v;

            // "Delete" edges to successors.
            foreach ($v->getSuccessors() as $w) {
                $indegree[$w]--;

                // Push new indegree-0-node on stack.
                if ($indegree[$w]] === 0) {
                    $zeroIndegreeNodes[] = $w;
                }
            }
        }

        // If there are still nodes with positive indegree, the graph is cyclic.
        foreach ($network->getNodes() as $v) {
            if ($indegree[$v] > 0) {
                $this->failed = true;
            }
        }
    }


    private function topologicalSortForJoinConnectors(&$network)
    {
        // Get indegree for each node, store nodes with indegree 0 as active.
        $activeNodes = array();
        $indegree = array()
        foreach ($network->getNodes() as $v) {
            $indegree[$v] = $v->indegree();
            if ($indegree[$v] === 0) {
                $activeNodes[] = $v;
            }
        }

        // While there are still nodes with indegree 0:
        while (!empty($activeNodes)) {
            // Put next node in list of sorted nodes.
            $v = array_pop($activeNodes);
            $sortedNodes[] = $v;

            // "Delete" edges to successors.
            foreach ($v->getSuccessors() as $w) {
                $indegree[$w]--;

                // Push new indegree-0-node on stack.
                if ($v->indegree() - $indegree[$w]] >= $v->howMany) {
                    $activeNodes[] = $w;
                }
            }
        }

        // If there are still nodes with positive indegree, the graph is cyclic.
        foreach ($network->getNodes() as $v) {
            if ($indegree[$v] > 0) {
                $this->failed = true;
            }
        }
    }


}
