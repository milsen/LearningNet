<?php

namespace LearningNet\Network;

class Chain
{
    /**
     * Order is given by predecessor order of units, not by array order!
     */
    private $units;

    private $lastUnit;

    private $firstUnit;

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
        $this->setPointers();
        $this->lastUnit = end($this->units);
        $this->firstUnit = reset($this->units);
    }

    /**
     * Sets predecessors and successors for units in this chain based on the
     * order that $units is in.
     */
    private function setPointers()
    {
        $pred = null;
        foreach ($this->units as $unit) {
            $unit->setPred($pred);
            /* $pred->setSucc($unit); */
            $pred = $unit;
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

    public function vizRep($from, $to)
    {
        if ($this->isEmpty()) {
            return $from . " -> " . $to . ";\n";
        } else {
            return $from . " -> " . $this->firstUnit->startVizRep() . ";\n"
                . $this->innerVizRep()
                . $this->lastUnit->endVizRep() . " -> ". $to . ";\n";
        }
    }

    private function innerVizRep()
    {
        // If there is only one unit, its innerVizRep has to be output as well.
        if (count($this->units) === 1) {
            return $this->firstUnit->innerVizRep();
        }

        $str = "";

        $cur = $this->lastUnit;
        while ($cur !== $this->firstUnit) {
            $pred = $cur->getPred();
            $str .= $pred->endVizRep() . " -> " . $cur->startVizRep() . ";\n";
            $str .= $cur->innerVizRep();
            $cur = $pred;
        }

        return $str;
    }

}
