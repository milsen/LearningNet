<?php

namespace LearningNet\CostFunctions;

use Mooc\DB\Block;
use LearningNet\DB\NodePairCosts;

/**
 * TODO
 *
 * @author  <milsen@uos.de>
 */
abstract class NodePairCostFunction extends CostFunction
{
    abstract protected function calculate($sectionFrom, $sectionTo);

    public function recalculateValues($courseId, $changedSections) {
        // TODO as part of Mooc\DB\Block
        $secs = Block::findBySQL(
            "type = 'Section' AND seminar_id = ?", [$courseId]);
        $sections = array_map(function ($sec) { return $sec['id']; }, $secs);

        // From changed section:
        foreach ($changedSections as $sectionFrom) {
            foreach ($sections as $sectionTo) {
                NodePairCosts::save($courseId, $this->costFuncName,
                    $sectionFrom, $sectionTo,
                    $this->adjust($this->calculate($sectionFrom, $sectionTo))
                );
            }
        }

        // To changed section:
        foreach ($sections as $sectionFrom) {
            foreach ($changedSections as $sectionTo) {
                NodePairCosts::save($courseId, $this->costFuncName,
                    $sectionFrom, $sectionTo,
                    $this->adjust($this->calculate($sectionFrom, $sectionTo))
                );
            }
        }
    }
}
