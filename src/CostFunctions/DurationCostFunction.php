<?php

namespace LearningNet\CostFunctions;

/**
 * TODO
 *
 * @author  <milsen@uos.de>
 */
class DurationCostFunction extends NodeCostFunction
{
    public function calculate($section) {
        return rand(0, 100);
    }
}
