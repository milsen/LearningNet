<?php

namespace LearningNet\Network;

class Condition extends ConnectiveUnit
{
    protected static $connective = "?";

    /**
     * Map: Chain => function returning bool
     */
    private conditions;

    public function __construct($conditions, $chains = array(), $parentChain = null, $predecessor = null)
    {
        parent::__construct($chains, $parentChain, $predecessor);
        $this->conditions = $conditions;
    }

    public function addChain($chain)
    {

    }

    public function isActiveChain($chain)
    {
        // Active if the connective itself is active and the condition is met.
        return $this->isActive() && $this->conditions[$chain]();
    }

    /**
     * Like Disjunction.
     */
    public function isCompleted()
    {
        // If one chain is completed, return true, else false.
        foreach ($this->chains as $chain) {
            if ($chain->isCompleted()) {
                return true;
            }
        }

        // Return false (unless there are no chains anyway).
        return $chain->isEmpty();
    }
}
