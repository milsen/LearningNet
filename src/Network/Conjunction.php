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
        foreach ($this->chains as $chain) {
            if (!$chain->isCompleted()) {
                return false;
            }
        }

        // If all entries are true, return true, else false.
        return true;
    }
}
