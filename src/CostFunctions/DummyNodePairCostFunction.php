<?php

namespace LearningNet\CostFunctions;

/**
 * TODO
 *
 * @author  <milsen@uos.de>
 */
class DummyNodePairCostFunction extends NodePairCostFunction
{
    public function calculate($sectionFrom, $sectionTo) {
        return rand(0, 100);
    }
}
