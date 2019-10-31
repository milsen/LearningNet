<?php

namespace LearningNet\CostFunctions;

use LearningNet\DB\NodeCosts;

/**
 * TODO
 *
 * @author  <milsen@uos.de>
 */
abstract class NodeCostFunction extends CostFunction
{
    abstract protected function calculate($section);

    public function recalculateValues($courseId, $changedSections) {
        foreach ($changedSections as $section) {
            NodeCosts::save($courseId, $this->costFuncName, $section,
                $this->adjust($this->calculate($section))
            );
        }
    }
}
