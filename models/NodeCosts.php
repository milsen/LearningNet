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
class NodeCosts extends Costs
{
    static protected function configure($config = array()) {
        $config['db_table'] = 'learningnet_node_costs';
        parent::configure($config);
    }

    /**
     * @param $nameAsIndex boolean whether the returned array should be indexed
     * by cost functions names instead of integers
     * @return an array of the form
     * [ { 'weight' => "weight", 'costs' => { "block_id" => "cost" } ]
     */
    static public function costs($courseId, $nameAsIndex = false) {
        return parent::costs($courseId, function(&$currentCosts, $row) {
            $currentCosts[$row['block_id']] = $row['cost'];
        }, $nameAsIndex);
    }
}
