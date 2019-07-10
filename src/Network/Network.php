<?php

namespace LearningNet\Network;

class Network
{
    public function __construct()
    {
    }

    public function getNodes()
    {
    }

    public function vizRep()
    {
        return "digraph Network {\n" . $this->chain->vizRep("S", "T") . "}\n";
    }

}
