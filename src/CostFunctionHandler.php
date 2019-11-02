<?php

namespace LearningNet;

use LearningNet\DB\CostFunctions;
use LearningNet\DB\NodeCosts;
use LearningNet\DB\NodePairCosts;
use LearningNet\CostFunctions\DurationCostFunction;
use LearningNet\CostFunctions\DummyNodePairCostFunction;

/**
 * The CostFunctionHandler has two functions:
 * It serves as a wrapper around the CostFunctions model, removing database
 * entries for cost functions with weight 0.
 * Moreover, it notifies CostFunction classes to recalculate cost values when
 * the respective Courseware blocks are edited.
 *
 * @author  <milsen@uos.de>
 */
class CostFunctionHandler
{

    const COST_FUNCTIONS = [
        'DurationCostFunction',
        'DummyNodePairCostFunction'
    ];

    /** @var CostFunctionHandler instance */
    protected static $instance = null;

    /** @return CostFunctionHandler */
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
     * Prohibits external instantiation of Singleton.
     */
    protected function __construct() {
        /* NotificationCenter::addObserver($this, 'updateCW', 'recalculateCosts'); */
        /* NotificationCenter::addObserver($this, 'deleteCW', 'removeCosts'); */
        // No notification for courseware changes :(
    }

    /**
     * @param string $className name of a cost function class
     * @return string $className with a prepended namespace such that an object
     * of the class can be created
     */
    private function toClass($className) {
        return 'LearningNet\\CostFunctions\\' . $className;
    }

    /**
     * For each cost func with non-zero weight: Recalculate costs for
     * sections of course given by id.
     *
     * To be called when a sections are added or changed.
     *
     * @param string $courseId id of the course
     * @param int[] $changedSections sections for which to recalculate costs
     */
    public function recalculateCosts($courseId, $changedSections) {
        $activatedCostFuncs = CostFunctions::getActivated($courseId,
            self::COST_FUNCTIONS
        );
        foreach ($activatedCostFuncs as $costFuncName) {
            $costFuncClass = $this->toClass($costFuncName);
            $costFunc = new $costFuncClass($costFuncName);
            $costFunc->recalculateValues($courseId, $changedSections);
        }
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
        foreach (self::COST_FUNCTIONS as $costFunc) {
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
            if (in_array($costFunc, self::COST_FUNCTIONS)) {
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
