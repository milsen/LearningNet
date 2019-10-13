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
}
