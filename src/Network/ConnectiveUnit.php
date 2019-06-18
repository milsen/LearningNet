<?php

namespace LearningNet\Network;

abstract class ConnectiveUnit extends Unit
{
    private $chains;

    public function __construct($chains = array(), $parentChain = null, $predecessor = null)
    {
        parent::__construct($parentChain, $predecessor);
        $this->chains = $chains;
    }

    public function removeChain()
    {

    }

    public function addChain($chain)
    {
        $this->chains[] = $chain;
    }

    abstract public function __toString();

    /**
     *
     */
    protected function toString($connective)
    {
        $first = true;
        $str = "<" . $connective . ">( ";

        // For each chain in chains:
        foreach ($this->chains as $chain) {
            if ($first) {
                $first = false;
            } else {
                $str .= " | ";
            }

            $str .= $chain->__toString();
        }
        return $str . " )";
    }

    /**
     * Can be overridden by subclasses of ConnectiveUnit that only activate
     * specific chains.
     * By default all chains are active if this connective is active.
     * @param Chain chain
     * @return whether chain is active
     */
    public function isActiveChain($chain)
    {
        return $this->isActive();
    }

    public function getChains()
    {
        return $chains;
    }
}
