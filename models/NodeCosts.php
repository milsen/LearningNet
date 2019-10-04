<?php

namespace LearningNet\DB;

/**
 * TODO.
 *
 * @property string seminar_id database column
 * @property string cost_func database column
 * @property int block_id database column
 * @property int cost database column
 *
 * @author  <milsen@uos.de>
 */
class NodeCosts extends \SimpleORMap
{
    static protected function configure($config = array()) {
        $config['db_table'] = 'learningnet_node_costs';
        parent::configure($config);
    }

    static public function costs($courseId) {
        $rows = static::findBySQL('INNER JOIN learningnet_cost_functions
            ON (' . $config['db_table'] . '.seminar_id = learningnet_cost_functions.seminar_id
            AND ' . $config['db_table'] . '.cost_func  = learningnet_cost_functions.cost_func)
            WHERE (seminar_id = ? GROUP BY cost_func)', [$courseId]);
        $nodeCosts = [];

        // Collect costs in array:
        // [ { 'name' => "cost_func", 'costs' => { "block_id" => "cost" } ]
        $prevCostFunc = $rows[0]['cost_func'];
        $prevWeight = $rows[0]['weight'];
        $currentCosts = [];
        foreach ($rows as $row) {
            $costFunc = $row['cost_func'];
            if ($costFunc !== $prevCostFunc) {
                array_push($nodeCosts, [
                    'weight' => $prevWeight,
                    'costs' => $currentCosts
                ];
                $currentCosts = [];
                $prevCostFunc = $costFunc;
                $prevWeight = $row['weight'];
            }
            $currentCosts[$row['block_id']] = $row['cost'];
        }
        array_push($nodeCosts, [
            'weight' => $prevWeight,
            'costs' => $currentCosts
        ];

        return $nodeCosts;
    }
}
