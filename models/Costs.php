<?php

namespace LearningNet\DB;

/**
 * TODO.Superclass for NodeCosts and NodePairCosts
 *
 * @author  <milsen@uos.de>
 */
class Costs extends \SimpleORMap
{
    /**
     * @param $courseId string id of the course for which the costs should be found
     * @param $func function adding to cost array using database row
     * @param $nameAsIndex boolean whether the returned array should be indexed
     * by cost functions names instead of integers
     * @return  array of the form
     * [ { 'weight' => "weight", 'costs' => { func(... "cost" ...) } ]
     * indexed by integers or cost functions names depending on $nameAsIndex
     */
    static protected function costs($courseId, $func, $nameAsIndex = false) {
        $dbTable = self::config('db_table');
        $rows = static::findBySQL('INNER JOIN learningnet_cost_functions
            ON (' . $dbTable . '.seminar_id = learningnet_cost_functions.seminar_id
            AND ' . $dbTable . '.cost_func  = learningnet_cost_functions.cost_func )
            WHERE (' . $dbTable . '.seminar_id = ?) GROUP BY cost_func', [$courseId]);
        $result = [];

        // Collect costs in array:
        // [ { 'weight' => "weight", 'costs' => { func(... "cost" ...) } ]
        $prevCostFunc = $rows[0]['cost_func'];
        $prevWeight = $rows[0]['weight'];
        $currentCosts = [];
        foreach ($rows as $row) {
            $costFunc = $row['cost_func'];
            if ($costFunc !== $prevCostFunc) {
                // If a new cost function is found, push cost values for
                // previous cost function.
                $arr = [ 'weight' => $prevWeight, 'costs' => $currentCosts ];
                if ($nameAsIndex) {
                    $result[$costFunc] = $arr;
                } else {
                    array_push($result, $arr);
                }

                // Reset variables for new cost function.
                $currentCosts = [];
                $prevCostFunc = $costFunc;
                $prevWeight = $row['weight'];
            }

            // Add costs found in $row to $currentCosts using $func.
            $func($currentCosts, $row);
        }

        // Push last collected costs.
        if (!empty($row)) {
            $arr = [ 'weight' => $prevWeight, 'costs' => $currentCosts ];
            if ($nameAsIndex) {
                $result[$costFunc] = $arr;
            } else {
                array_push($result, $arr);
            }
        }

        return $result;
    }
}
