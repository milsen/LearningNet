<?php

namespace LearningNet\DB;

/**
 * TODO.
 *
 * @property string seminar_id database column
 * @property string cost_func database column
 * @property int block_id_from database column
 * @property int block_id_to database column
 * @property int cost database column
 *
 * @author  <milsen@uos.de>
 */
class NodePairCosts extends Costs
{
    static protected function configure($config = array()) {
        $config['db_table'] = 'learningnet_node_pair_costs';
        parent::configure($config);
    }

    /**
     * @return an array of the form
     * [ { 'weight' => "weight", 'costs' => { "block_id_from" => { "block_id_to" => "cost" } } ]
     */
    static public function costs($courseId) {
        return parent::costs($courseId, function(&$currentCosts, $row) {
            if (!array_key_exists($row['block_id_from'], $currentCosts)) {
                $currentCosts[$row['block_id_from']] = [];
            }
            $currentCosts[$row['block_id_from']][$row['block_id_to']] = $row['cost'];
        });
    }
}
