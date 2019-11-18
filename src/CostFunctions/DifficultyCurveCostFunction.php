<?php

namespace LearningNet\CostFunctions;

/**
 * Estimates the difficulty difference between Courseware sections based on
 * difficulty values set by a course instructor.
 *
 * @author  <milsen@uos.de>
 */
class DifficultyCurveCostFunction extends NodePairCostFunction
{
    /** {@inheritdoc} **/
    public static $instructorSetFields = ["difficulty"];

    /**
     * @param int $sectionFrom id of the first section
     * @param int $sectionTo id of the second section
     * @return int difference in difficulties of $sectionFrom and $sectionTo as
     * a number in [0,100]
     */
    public function calculate($sectionFrom, $sectionTo) {
        $difficultyFrom = $this->getData($sectionFrom, 'difficulty');
        $difficultyFrom = $difficultyFrom ? $difficultyFrom : 0;
        $difficultyTo = $this->getData($sectionTo, 'difficulty');
        $difficultyTo = $difficultyTo ? $difficultyTo : 0;

        // Assume that the difficulties are in the interval [0,10].
        // Scale them up to reach max. 100.
        return abs($difficultyFrom - $difficultyTo) * 10;
    }
}
