<?php

namespace LearningNet\Network;

class Elective extends ConnectiveUnit
{
    protected static $connective = "#";

    /**
     * How many chains have to be completed before this elective connective is
     * completed.
     */
    private $howMany;

    public function __construct($howMany, $chains = array(), $parentChain = null, $predecessor = null)
    {
        parent::__construct($chains, $parentChain, $predecessor);
        $this->howMany = $howMany;
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
}
