<?php

namespace LearningNet\CostFunctions;

use Mooc\DB\Block;
use LearningNet\DB\NodePairCosts;

/**
 * Base class for node pair cost functions, i.e. cost functions that calculate a
 * value for every pair of sections (rather than just any section).
 *
 * @author  <milsen@uos.de>
 */
abstract class NodePairCostFunction extends CostFunction
{
    /**
     * Calculates a cost function values for a given pair of sections.
     *
     * @param int $sectionFrom id of the first section
     * @param int $sectionTo id of the second section
     * @return float|int cost function value
     */
    abstract protected function calculate($sectionFrom, $sectionTo);

    /**
     * @inheritdoc
     */
    public function recalculateValues($courseId, $changedSections) {
        // TODO as part of Mooc\DB\Block
        $secs = Block::findBySQL(
            "type = 'Section' AND seminar_id = ?", [$courseId]);
        $sections = array_map(function ($sec) { return $sec['id']; }, $secs);

        // From changed section:
        foreach ($changedSections as $sectionFrom) {
            foreach ($sections as $sectionTo) {
                NodePairCosts::save($courseId, $this->getClassName($this),
                    $sectionFrom, $sectionTo,
                    $this->adjust($this->calculate($sectionFrom, $sectionTo))
                );
            }
        }

        // To changed section:
        foreach ($sections as $sectionFrom) {
            foreach ($changedSections as $sectionTo) {
                NodePairCosts::save($courseId, $this->getClassName($this),
                    $sectionFrom, $sectionTo,
                    $this->adjust($this->calculate($sectionFrom, $sectionTo))
                );
            }
        }
    }
}
