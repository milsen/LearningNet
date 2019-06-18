<?php

namespace LearningNet\Network;

class Chain
{
    private $units;

    /**
     * ConnectiveUnit that this Chain belongs to.
     */
    private $parent;

    public function __construct($units = array(), $parent = null)
    {
        $this->units = $units;
        $this->parent = $parent;
        $this->setPredecessors();
    }

    private function setPredecessors()
    {
        $prevPred = null;
        foreach ($this->units as $unit) {
            $unit->setPredecessor($prevPred);
            $prevPred = $unit;
        }
    }

    public function appendElementaryUnit($id, $completed = null)
    {
        $this->units[] = new ElementaryUnit($id, $completed, $this, end($this->units));
    }

    public function getUnits()
    {
        return $this->units;
    }

    public function isEmpty()
    {
        return empty($this->units);
    }

    public function isCompleted()
    {
        return $this->isEmpty() ?
            $this->parent->isActiveChain($this) :
            end($this->units)->isCompleted();
    }

    public function isActive()
    {
        // If this is the top-level chain, activate it.
        // Otherwise let the parent connective decide whether it is active.
        return $this->parent === null || $this->parent->isActiveChain($this);
    }

    public function __toString()
    {
        $str = "";
        $first = true;

        foreach ($this->units as $unit) {
            if ($first) {
                $first = false;
            } else {
                $str .= " -> ";
            }
            $str .= $unit->__toString();
        }

        return $str;
    }

}
