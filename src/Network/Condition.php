<?php

namespace LearningNet\Network;

class Condition extends ConnectiveUnit
{
    const CONNECTIVE = "X";

    /**
     * Map: Chain => function returning bool
     */
    private conditions;

    public function __construct($conditions, $chains = array(), $parentChain = null, $predecessor = null)
    {
        parent::__construct($chains, $parentChain, $predecessor);
        $this->conditions = $conditions;
    }

    public function __toString()
    {
        return parent::toString(self::CONNECTIVE);
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
        $chainsCompleted = array_map(function ($chain) {
            return $chain->isCompleted();
        }, $this->chains);

        // If one entry is true, return true, else false.
        return in_array(true, $chainsCompleted, true);
    }
}
