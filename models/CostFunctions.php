<?php

namespace LearningNet\DB;

/**
 * TODO.
 * Cost functions with weight 0 are not stored in the database.
 *
 * @property string seminar_id database column
 * @property string cost_func database column
 * @property double weight database column
 *
 * @author  <milsen@uos.de>
 */
class CostFunctions extends \SimpleORMap
{
    static protected function configure($config = array()) {
        $config['db_table'] = 'learningnet_cost_functions';
        parent::configure($config);
    }

    /**
     * @param string $courseId id of the course
     * @param string[] $costFunctions array of cost function names
     * @return string[] subset of $costFunctions that is found in the database,
     * i.e that are assigned a non-zero weight
     */
    static public function getActivated($courseId, $costFunctions) {
        $rows = parent::findBySQL('seminar_id = :course_id '
            . 'AND cost_func in (:funcs) '
            . 'GROUP BY cost_func '
        , ['course_id' => $courseId, 'funcs' => $costFunctions]);
        return array_map(function ($row) { return $row['cost_func']; }, $rows);
    }
}
