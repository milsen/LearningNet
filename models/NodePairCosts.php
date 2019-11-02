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
    static protected function configure($config = []) {
        $config['db_table'] = 'learningnet_node_pair_costs';
        parent::configure($config);
    }

    /**
     * @return array of the form
     * [ { 'weight' => "weight", 'costs' => { "block_id_from" => { "block_id_to" => "cost" } } ]
     */
    static public function getByCourseId($courseId) {
        return parent::costs($courseId, function(&$currentCosts, $row) {
            if (!array_key_exists($row['block_id_from'], $currentCosts)) {
                $currentCosts[$row['block_id_from']] = [];
            }
            $currentCosts[$row['block_id_from']][$row['block_id_to']] = $row['cost'];
        });
    }

    /**
     * Stores cost of a pair of Courseware sections according to a given cost
     * function.
     *
     * @param string $courseId id of the course
     * @param string $costFunc name/id of the cost function
     * @param string $sectionIdFrom block id of the first Courseware section
     * @param string $sectionIdTo block id of the second Courseware section
     * @param int $costValue value in [0,100]
     * @return void
     */
    static public function save($courseId, $costFunc,
        $sectionIdFrom, $sectionIdTo, $costValue) {
        $costRep = self::find([$courseId, $costFunc,
            $sectionIdFrom, $sectionIdTo
        ]);
        if ($costRep === null) {
            $costRep = new self;
            $costRep->seminar_id = $courseId;
            $costRep->cost_func = $costFunc;
            $costRep->block_id_from = $sectionIdFrom;
            $costRep->block_id_to = $sectionIdTo;
        }
        $costRep->cost = $costValue;
        $costRep->store();
    }
}
