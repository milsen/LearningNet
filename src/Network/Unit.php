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

    abstract public function __toString();

    abstract public function isCompleted();

    public function isActive()
    {
        return $this->predecessor === null ?
            $this->parentChain->isActive() :
            $this->predecessor->isCompleted();
    }

    public function setPredecessor($predecessor)
    {
        $this->predecessor = $predecessor;
    }

}
