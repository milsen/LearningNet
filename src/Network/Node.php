<?php

namespace LearningNet\Network;

abstract class Node
{
    protected $id;

    private static $idCount = 0;

    public function __construct()
    {
        $this->id = $idCount++;
    }

    abstract public function isCompleted();

    abstract public function getActiveSuccessors();

    abstract public function indegree();

    /**
     * Returns the Viz statements that represent this (connective) unit.
     */
    /* abstract public function innerVizRep(); */

    /**
     * Returns the id shown after the " -> " that leads to this unit.
     */
    /* abstract public function startVizRep(); */

    /**
     * Returns the id shown before the " -> " that leads away from this unit.
     */
    /* abstract public function endVizRep(); */

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
