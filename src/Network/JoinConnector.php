<?php

namespace LearningNet\Network;

abstract class JoinConnector extends Node
{
    protected $predecessors;

    protected $successor;

    public function __construct($predecessors = array(), $successor = null)
    {
        $this->predecessors = $predecessors;
        $this->successor = $successor;
    }

    public function getActiveSuccessors()
    {
        return $this->isCompleted ? array($successor) : array();
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
