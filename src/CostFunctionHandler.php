<?php

namespace LearningNet;

use LearningNet\DB\CostFunctions;

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
// TODO extends CoursewareObserver ? new calculation of costs every time
// courseware sections are added/deleted/changed
{
    const COST_FUNCTIONS = [
        // TODO use cost functions class names as values,
        // e.g. 'LearningNet\\CostFunctions\\Duration'
        'Test_Knotenkosten' =>     'node',
        'Test_Knotenpaarkosten' => 'node_pair'
    ];

    public function getCostFunctions() {
        return self::COST_FUNCTIONS;
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
