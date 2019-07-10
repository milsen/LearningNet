<?php

namespace LearningNet\Network;

class Disjunction extends ConnectiveUnit
{
    protected static $connective = "OR";

    public function isCompleted()
    {
        // If one chain is completed, return true, else false.
        foreach ($this->chains as $chain) {
            if ($chain->isCompleted()) {
                return true;
            }
        }

        // Return false (unless there are no chains anyway).
        return $chain->isEmpty();
    }
}
