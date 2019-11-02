<?php

namespace LearningNet\CostFunctions;

/**
 * TODO
 *
 * @author  <milsen@uos.de>
 */
abstract class CostFunction
{
    public function __construct() { }

    abstract protected function recalculateValues($courseId, $changedSections);

    /**
     * Retrieves the class name for a cost function object, which is used as a
     * key in the database tables.
     *
     * @param object $obj object for which the classname should be gotten
     * @return string class name of $obj without the namespace
     */
    protected function getClassName($obj) {
        $classname = get_class($obj);
        $pos = strrpos($classname, '\\');
        if ($pos !== false) {
            return substr($classname, $pos + 1);
        }
        return $className;
    }

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