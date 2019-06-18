<?php

namespace LearningNet\Network;

class Elective extends ConnectiveUnit
{
    const CONNECTIVE = "#";

    private $howMany;

    public function __construct($howMany, $chains = array(), $parentChain = null, $predecessor = null)
    {
        parent::__construct($chains, $parentChain, $predecessor);
        $this->howMany = $howMany;
        // TODO restrict howMany to >= 1, use disjunction with empty chain instead
    }

    public function isCompleted()
    {
        if ($howMany === 0) {
            return true;
        }

        $count = 0;
        foreach ($this->chains as $chain) {
            if ($chain->isCompleted()) {
                $count++;
            }
            if ($count >= $howMany) {
                return true;
            }
        }

        // If one entry is true, return true, else false.
        return false;
    }

    public function __toString()
    {
        return parent::toString(self::CONNECTIVE);
    }
}
