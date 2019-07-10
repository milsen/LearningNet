<?php

namespace LearningNet\Network;

class Conjunction extends ConnectiveUnit
{
    protected static $connective = "AND";

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
