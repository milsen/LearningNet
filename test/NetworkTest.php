<?php

use LearningNet\Network\Network;
use LearningNet\Network\Chain;
use LearningNet\Network\ElementaryUnit;
use LearningNet\Network\Conjunction;
use PHPUnit\Framework\TestCase;

class NetworkTestCase extends TestCase
{
    public function setUp()
    {
    }

    public function testNetworkPrint() {
        $network = new Network();

        $unit1 = new ElementaryUnit("id1");
        $unit2 = new ElementaryUnit("id2");
        $unit3 = new ElementaryUnit("id3");
        $unit4 = new ElementaryUnit("id4");

        $subchain1 = new Chain(array($unit2, $unit3));
        $subchain2 = new Chain(array($unit4));
        $conjunction = new Conjunction(array($subchain1, $subchain2));
        $chain = new Chain(array($unit1, $conjunction));

        $network = new Network($chain);
        echo $network->vizRep();

        echo (new Network(new Chain(array($conjunction))))->vizRep();
    }
}
