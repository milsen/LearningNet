<?php

namespace LearningNet\Network;

class Network
{
    /**
     * The top-level chain of this network.
     */
    private $chain;

    public function __construct($chain = null)
    {
        if ($chain === null) {
            $chain = new Chain();
        }
        $this->chain = $chain;
    }

    public function getChain()
    {
        return $this->chain;
    }

    public function __toString()
    {
        return $this->chain->isEmpty() ?
            "s -> t" :
            "s -> " . $this->chain->__toString() . " -> t";
    }

}
