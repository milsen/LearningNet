<?php

namespace LearningNet\CostFunctions;

/**
 * TODO
 *
 * @author  <milsen@uos.de>
 */
abstract class CostFunction
{
    /** string $name name used in database tables for this cost function **/
    // TODO not how that works
    public $costFuncName;

    public function __construct($costFuncName) {
        $this->costFuncName = $costFuncName;
    }

    abstract protected function recalculateValues($courseId, $changedSections);

    /**
     * Adjust a cost function value such that it is an int and fits into the
     * interval [0,100].
     *
     * All cost function values should be adjusted this way such that their
     * output is comparable.
     *
     * @param float $value value to be adjusted
     * @return int $value casted to an int and ajusted to fit into [0,100]
     */
    protected function adjust($value) {
        $value = (int) $value;
        $value = min(100, $value);
        $value = max(0, $value);
        return $value;
    }
}
