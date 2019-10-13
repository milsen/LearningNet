<?php

namespace LearningNet;

use LearningNet\DB\CostFunctions;
use LearningNet\DB\NodeCosts;
use LearningNet\DB\NodePairCosts;
use LearningNet\CostFunctions\DurationCostFunction;
use LearningNet\CostFunctions\DummyNodePairCostFunction;

/**
 * TODO
 * The CostFunctionHandler has two functions:
 * It serves as a wrapper around the CostFunctions model, removing database
 * entries for cost functions with weight 0.
 * Moreover, it notifies CostFunctions to recalculate cost values when the
 * respective Courseware blocks are edited.
 *
 * @author  <milsen@uos.de>
 */
class CostFunctionHandler
{
    const COST_FUNCTIONS = [
        'duration'        => 'LearningNet\\CostFunctions\\DurationCostFunction',
        'dummy_node_pair' => 'LearningNet\\CostFunctions\\DummyNodePairCostFunction'
    ];

    public function getCostFunctions() {
        return self::COST_FUNCTIONS;
    }

    /**
     * @var CostFunctionHandler instance
     */
    protected static $instance = null;

    /**
     * @return CostFunctionHandler
     */
    public static function getInstance() {
        if (self::$instance === null) {
            self::$instance = new self;
        }
        return self::$instance;
    }

    /**
     * Prohibits cloning of Singleton instance.
     */
    protected function __clone() {}

    /**
     * Prohibit external instantiation of Singleton.
     */
    protected function __construct() {
        /* NotificationCenter::addObserver($this, 'updateCW', 'recalculateCosts'); */
        /* NotificationCenter::addObserver($this, 'deleteCW', 'removeCosts'); */
        // No notification for courseware changes :(
    }

    /**
     * For each cost func with non-zero weight: Recalculate costs for
     * sections of course given by id.
     *
     * To be called when a sections are added or changed.
     */
    public function recalculateCosts($courseId, $changedSections) {
        $activatedCostFuncs = CostFunctions::getActivated($courseId,
            array_keys(self::COST_FUNCTIONS)
        );
        foreach ($activatedCostFuncs as $costFuncName) {
            $costFuncClass = self::COST_FUNCTIONS[$costFuncName];
            $costFunc = new $costFuncClass($costFuncName);
            $costFunc->recalculateValues($courseId, $changedSections);
        }
    }

    /**
     * To be called when Courseware sections are deleted.
     * TODO Generally this function is not needed since the foreign key relation
     * already ensures that these are deleted.
     */
    public function removeCosts($courseId, $deletedSections) {
        NodeCosts::deleteBySQL("seminar_id = :course_id "
            . "AND block_id in (:sections) "
        , [
            "course_id" => $courseId,
            "sections" => $deletedSections
        ]);
        NodePairCosts::deleteBySQL("seminar_id = :course_id "
            . "AND (block_id_from in (:sections) "
            . "OR block_id_to in (:sections)) "
        , [
            "course_id" => $courseId,
            "sections" => $deletedSections
        ]);
    }

    /**
     * Returns all cost functions in self::COST_FUNCTIONS with their weights for
     * the given course. If a cost function is not found in the database, its
     * returned weight is 0.
     *
     * @param string $courseId id of the course
     * @return float[] indexed by cost function name with one entry for each
     * cost function in self::COST_FUNCTIONS
     */
    public function getCostFunctionWeights($courseId) {
        $rows = CostFunctions::findBySQL('seminar_id = ?', [$courseId]);
        $funcToWeight = [];
        foreach ($rows as $row) {
            $funcToWeight[$row['cost_func']] = $row['weight'];
        }

        $costFunctions = [];
        foreach (array_keys(self::COST_FUNCTIONS) as $costFunc) {
            $costFunctions[$costFunc] =
                array_key_exists($costFunc, $funcToWeight) ?
                $funcToWeight[$costFunc] : 0.0;
        }
        return $costFunctions;
    }

    /**
     * Sets weights for the given cost functions and the given course.
     * Deletes database entries for cost functions whose weight is set to 0.
     *
     * @param string $courseId id of the course
     * @param float[] $weightInput indexed by cost function names
     * @return void
     */
    public function setCostFunctionWeights($courseId, $weightInput) {
        foreach ($weightInput as $costFunc => $weight) {
            if (array_key_exists($costFunc, self::COST_FUNCTIONS)) {
                $costFuncRep = CostFunctions::find([$courseId, $costFunc]);
                if ($costFuncRep === null) {
                    if ($weight != 0) {
                        $costFuncRep = new CostFunctions();
                        $costFuncRep->seminar_id = $courseId;
                        $costFuncRep->cost_func = $costFunc;
                        $costFuncRep->weight = $weight;
                        $costFuncRep->store();
                    }
                } else {
                    if ($weight == 0) {
                        $costFuncRep->delete();
                    } else {
                        $costFuncRep->weight = $weight;
                        $costFuncRep->store();
                    }
                }
            }
        }
    }
}
