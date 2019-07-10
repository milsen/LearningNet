<?php

namespace LearningNet\Network;

abstract class ConnectiveUnit extends Unit
{
    private $chains;

    private $id;

    private static $connective;

    private static $idCount = 0;

    public function __construct($chains = array(), $parentChain = null, $predecessor = null)
    {
        parent::__construct($parentChain, $predecessor);
        $this->chains = $chains;
        $this->id = self::$idCount++;
    }

    public function removeChain()
    {

    }

    public function addChain($chain)
    {
        $this->chains[] = $chain;
    }

    public function startVizRep()
    {
        return static::$connective . "_open_" . $this->id;
    }

    public function endVizRep()
    {
        return static::$connective . "_close_" . $this->id;
    }

    public function innerVizRep()
    {
        $str = "";
        $openName = $this->startVizRep();
        $closeName = $this->endVizRep();

        // For each chain in chains:
        foreach ($this->chains as $chain) {
            $str .= $chain->vizRep($openName, $closeName);
        }

        return $str;
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
