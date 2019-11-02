<?php

namespace LearningNet\CostFunctions;

use LearningNet\DB\NodeCosts;

/**
 * Base class for node cost functions, i.e. cost functions that calculate a
 * value for every section (rather than any pair of sections).
 *
 * @author  <milsen@uos.de>
 */
abstract class NodeCostFunction extends CostFunction
{
    /**
     * Calculates a cost function values for a given section.
     *
     * @param int $section id of the section
     * @return float|int cost function value
     */
    abstract protected function calculate($section);

    /**
     * @inheritdoc
     */
    public function recalculateValues($courseId, $changedSections) {
        foreach ($changedSections as $section) {
            NodeCosts::save($courseId, $this->getClassName($this), $section,
                $this->adjust($this->calculate($section))
            );
        }
    }
}
