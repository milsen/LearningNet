<?php

namespace LearningNet\CostFunctions;

use Mooc\DB\Field;

/**
 * Abstract base class for classes implementing cost functions.
 * Values of these cost functions determine which learning path will be
 * recommended to a student.
 *
 * @author  <milsen@uos.de>
 */
abstract class CostFunction
{
    /**
     * @var string[] names of the entries in the mooc_fields table which are
     * needed for the calculation of this cost function, values are set directly
     * by the course instructor
     **/
    public static $instructorSetFields = [];

    /**
     * Recalculate and store cost function values for the given sections of the
     * given course.
     *
     * @param string $courseId id of the course
     * @param int[] $changedSections sections for which to recalculate costs
     */
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

    /**
     * Retrieves and decodes the json data stored in the mooc_fields database
     * table for the given block id and field name.
     *
     * @param int $blockid id of the section
     * @param string $name mooc field name
     * @return mixed decoded json data
     */
    protected function getData($blockId, $name) {
        return json_decode(
            Field::find([$blockId, '', $name])['json_data'],
        true);
    }
}
