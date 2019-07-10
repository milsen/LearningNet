<?php

namespace LearningNet\Network;

abstract class Unit
{
    protected $parentChain;

    /**
     * Unit coming before this Unit in the chain.
     */
    protected $predecessor;

    public function __construct($parentChain = null, $predecessor = null)
    {
        $this->parentChain = $parentChain;
        $this->predecessor = $predecessor;
    }

    /**
     * Returns the Viz statements that represent this (connective) unit.
     */
    abstract public function innerVizRep();

    /**
     * Returns the id shown after the " -> " that leads to this unit.
     */
    abstract public function startVizRep();

    /**
     * Returns the id shown before the " -> " that leads away from this unit.
     */
    abstract public function endVizRep();

    abstract public function isCompleted();

    public function isActive()
    {
        return $this->predecessor === null ?
            $this->parentChain->isActive() :
            $this->predecessor->isCompleted();
    }

    public function setPred($predecessor)
    {
        $this->predecessor = $predecessor;
    }

    public function getPred()
    {
        return $this->predecessor;
    }

}
