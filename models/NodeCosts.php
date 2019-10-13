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
     * @return array of the form
     * [ { 'weight' => "weight", 'costs' => { "block_id" => "cost" } ]
     */
    static public function costs($courseId) {
        return parent::costs($courseId, function(&$currentCosts, $row) {
            $currentCosts[$row['block_id']] = $row['cost'];
        });
    }

    /**
     * Stores cost of a Courseware section according to a given cost function.
     *
     * @param string $courseId id of the course
     * @param string $costFunc name/id of the cost function
     * @param string $sectionId block id of the Courseware section
     * @param int $costValue value in [0,100]
     * @return void
     */
    static public function save($courseId, $costFunc, $sectionId, $costValue) {
        $costRep = self::find([$courseId, $costFunc, $sectionId]);
        if ($costRep === null) {
            $costRep = new self;
            $costRep->seminar_id = $courseId;
            $costRep->cost_func = $costFunc;
            $costRep->block_id = $sectionId;
        }
        $costRep->cost = $costValue;
        $costRep->store();
    }
}
