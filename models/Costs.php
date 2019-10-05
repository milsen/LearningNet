<?php

namespace LearningNet\DB;

/**
 * TODO.Superclass for NodeCosts and NodePairCosts
 *
 * @author  <milsen@uos.de>
 */
class Costs extends \SimpleORMap
{
    protected function costs($courseId, $func) {
        $rows = static::findBySQL('INNER JOIN learningnet_cost_functions
            ON (' . $this->db_table . '.seminar_id = learningnet_cost_functions.seminar_id
            AND ' . $this->db_table . '.cost_func  = learningnet_cost_functions.cost_func)
            WHERE (seminar_id = ? GROUP BY cost_func)', [$courseId]);
        $result = [];

        // Collect costs in array:
        // [ { 'weight' => "weight", 'costs' => { func(... "cost" ...) } ]
        $prevCostFunc = $rows[0]['cost_func'];
        $prevWeight = $rows[0]['weight'];
        $currentCosts = [];
        foreach ($rows as $row) {
            $costFunc = $row['cost_func'];
            if ($costFunc !== $prevCostFunc) {
                array_push($result, [
                    'weight' => $prevWeight,
                    'costs' => $currentCosts
                ];
                $currentCosts = [];
                $prevCostFunc = $costFunc;
                $prevWeight = $row['weight'];
            }

            $func($currentCosts, $row);
        }
        array_push($result, [
            'weight' => $prevWeight,
            'costs' => $currentCosts
        ];

        return $result;
    }
}
