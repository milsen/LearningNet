<?php

require 'vendor/autoload.php'; // The autoloader provided by composer

namespace LearningNet\Ranking;

use SEIDS\Heaps\Pairing\PriorityQueue;

class PathRecommender
{
    public function __construct()
    {
    }

    public function recommendedPath($network)
    {
        // NEEDED: non-negative edge costs!

        // heap of active elementary units
        // Initialize.
        $priority = array();
        $source = $network->getChain()->firstUnit;

        $prioQueue = new PriorityQueue();
        foreach ($network->getElementaryUnits() as $elemUnit) {
            if ($elemUnit === $source) {
                $prioQueue->insert($elemUnit, 0);
                $priority[$elemUnit] = 0;
            } else {
                $prioQueue->insert($elemUnit, INF);
                $priority[$elemUnit] = INF;
            }
        }

        // Calculation.
        while (!$prioQueue->isEmpty()) {
            $unit = $prioQueue->extractMin();

            if ($unit === $network->getChain()->lastUnit) {
                break;
            }

            // For all "neighbours":
            // skip already completed units
            $neighbours = $unit->getSuccessors($unit);
            foreach ($neighbours as $neighbour) {
                $edgeVal = $this->evaluate($unit, $neighbour);
                $newPriority = $priority[$unit] + $edgeVal;

                // If old priority is worse (lower) than the new one, update it.
                if ($priority[$neighbour] < $newPriority) {
                    $prioQueue->decreaseKey($neighbour, $newPriority)
                }
            }
        }
    }

    private function evaluate($unit, $newUnit)
    {
        $value = 0;
        foreach ($criteria as $criterium) {
            if ($criterium->alreadyCalculated($unit, $newUnit)) {
            }
            $value += $criterium->evaluate($unit, $newUnit) * $this->weight[$criterium];
        }

        return $value;
    }

}
