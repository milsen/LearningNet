<?php

namespace LearningNet\Network;

abstract class SplitConnector extends Node
{
    protected $predecessor;

    protected $successors;

    public function __construct($predecessor = null, $successors = array())
    {
        $this->predecessor = $predecessor;
        $this->successors = $successors;
    }

    public function getActiveSuccessors()
    {
        return $this->successors;
        // Condition should array_filter($this->successors, )
    }

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

    /* abstract public function isCompleted(); */

    /* public function isActive() */
    /* { */
    /*     return $this->predecessor === null ? */
    /*         $this->parentChain->isActive() : */
    /*         $this->predecessor->isCompleted(); */
    /* } */
}
