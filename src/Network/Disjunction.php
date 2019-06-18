<?php

namespace LearningNet\Network;

class Disjunction extends ConnectiveUnit
{
    const CONNECTIVE = "OR";

    public function __toString()
    {
        return parent::toString(self::CONNECTIVE);
    }

    public function isCompleted()
    {
        $chainsCompleted = array_map(function ($chain) {
            return $chain->isCompleted();
        }, $this->chains);

        // If one entry is true, return true, else false.
        return in_array(true, $chainsCompleted, true);
    }
}
