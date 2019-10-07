<?php

namespace LearningNet;

/**
 * TODO
 *
 * @author  <milsen@uos.de>
 */
class CostFunctionHandler
// TODO extends CoursewareObserver ? new calculation of costs every time
// courseware sections are added/deleted/changed
{
    const COST_FUNCTIONS = [
        'Test_Knotenkosten' =>     'node',
        'Test_Knotenpaarkosten' => 'node_pair'
    ];

    public function getCostFunctions() {
        return self::COST_FUNCTIONS;
    }
}
