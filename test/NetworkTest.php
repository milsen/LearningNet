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
        $this->assertSame($network->__toString(), "s -> t");

        $unit = new ElementaryUnit("id1");
        $this->assertSame($unit->__toString(), "[id1]");

        $chain = new Chain(array($unit));
        $this->assertSame($chain->__toString(), "[id1]");
        $network = new Network($chain);
        $this->assertSame($network->__toString(), "s -> [id1] -> t");

        $unit2 = new ElementaryUnit("id2");
        $chain = new Chain(array($unit, $unit2));
        $this->assertSame($chain->__toString(), "[id1] -> [id2]");
        $network = new Network($chain);
        $this->assertSame($network->__toString(), "s -> [id1] -> [id2] -> t");

        $unit3 = new ElementaryUnit("id3");
        $chain2 = new Chain(array($unit3));
        $conjunction = new Conjunction(array($chain, $chain2));
        $this->assertSame($conjunction->__toString(), "<AND>( [id1] -> [id2] | [id3] )");
        $chain = new Chain(array($unit, $conjunction));
        $this->assertSame($chain->__toString(), "[id1] -> <AND>( [id1] -> [id2] | [id3] )");
        $network = new Network($chain);
        $this->assertSame($network->__toString(), "s -> [id1] -> <AND>( [id1] -> [id2] | [id3] ) -> t");
    }
}
