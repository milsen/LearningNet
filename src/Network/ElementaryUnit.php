<?php

namespace LearningNet\Network;

class ElementaryUnit extends Node
{
    private $id;

    private $completed;

    private $predecessor;

    private $successor;

    private $isTarget;

    public function __construct($id, $completed = false, $parentChain = null, $predecessor = null)
    {
        parent::__construct();
        $this->id = $id;
        $this->completed = $completed;
    }

    public function getActiveSuccessors()
    {
        return $this->isCompleted ? array($successor) : array();
    }









    public function startVizRep()
    {
        return $this->id;
    }

    public function innerVizRep()
    {
        return "";
    }

    public function endVizRep()
    {
        return $this->id;
    }

    public function isCompleted()
    {
        return $this->completed;
    }

    public function setCompleted($completed)
    {
        $this->completed = $completed;
    }
}
