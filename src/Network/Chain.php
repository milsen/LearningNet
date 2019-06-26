<?php

namespace LearningNet\Network;

class Chain
{
    /**
     * Order is given by predecessor order of units, not by array order!
     */
    private $units;

    private $lastUnit;

    /**
     * ConnectiveUnit that this Chain belongs to.
     */
    private $parent;

    /**
     * @param units array(Unit) order of array enforced as order of units
     */
    public function __construct($units = array(), $parent = null)
    {
        $this->units = $units;
        $this->parent = $parent;
        $this->setPredecessors();
        $this->lastUnit = end($this->units);
    }

    private function setPredecessors()
    {
        $prevPred = null;
        foreach ($this->units as $unit) {
            $unit->setPredecessor($prevPred);
            $prevPred = $unit;
        }
    }

    /* public function appendElementaryUnit($id, $completed = null) */
    /* { */
    /*     $this->lastUnit = new ElementaryUnit($id, $completed, $this, $lastUnit); */
    /*     $this->units[] = $this->lastUnit; */
    /* } */

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
            $this->lastUnit->isCompleted();
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
