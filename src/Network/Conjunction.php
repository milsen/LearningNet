<?php

namespace LearningNet\Network;

class Conjunction extends ConnectiveUnit
{
    const CONNECTIVE = "AND";

    public function __toString()
    {
        return parent::toString(self::CONNECTIVE);
    }

    public function isCompleted()
    {
        $chainsCompleted = array_map(function ($chain) {
            return $chain->isCompleted();
        }, $this->chains);

        // If all entries are true, return true, else false.
        return in_array(false, $chainsCompleted, true) === false;
    }
}
