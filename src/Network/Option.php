<?php

namespace LearningNet\Network;

// TODO remove, use disjunction with empty chain instead?
class Option extends ConnectiveUnit
{
    const CONNECTIVE = "?";

    public function isCompleted()
    {
        return $predecessor->isCompleted();
    }

    public function __toString()
    {
        return parent::toString(self::CONNECTIVE);
    }
}
