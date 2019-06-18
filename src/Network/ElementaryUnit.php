<?php

namespace LearningNet\Network;

class ElementaryUnit extends Unit
{
    private $id;

    private $completed;

    public function __construct($id, $completed = false, $parentChain = null, $predecessor = null)
    {
        parent::__construct($parentChain, $predecessor);
        $this->id = $id;
        $this->completed = $completed;
    }

    public function __toString()
    {
        return "[" . $this->id . "]";
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
