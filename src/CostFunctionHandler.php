<?php

namespace LearningNet;

use Mooc\DB\Field;
use Mooc\DB\Block;
use LearningNet\DB\CostFunctions;
use LearningNet\DB\NodeCosts;
use LearningNet\DB\NodePairCosts;
use LearningNet\CostFunctions\DurationCostFunction;
use LearningNet\CostFunctions\DifficultyCurveCostFunction;

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
        'DifficultyCurveCostFunction'
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

    /**
     * Returns all section fields needed by cost functions in
     * self::COST_FUNCTIONS with their currently stored values.
     *
     * @param string $courseId id of the course
     * @return array array of the form [ fieldName => [ sectionId => value ] ]
     */
    public function getCostFunctionFields($courseId) {
        // Get section ids.
        $sections = Block::findBySQL(
            "type = 'Section' AND seminar_id = ?", [$courseId]);
        $sectionIds = array_map(function ($sec) {
            return $sec['id'];
        }, $sections);

        $fields = [];
        foreach (self::COST_FUNCTIONS as $costFuncName) {
            $costFuncClass = $this->toClass($costFuncName);
            if (!empty($costFuncClass::$instructorSetFields)) {
                foreach ($costFuncClass::$instructorSetFields as $field) {
                    // Avoid repetition if field used by multiple cost functions.
                    if (!array_key_exists($field, $fields)) {
                        $fields[$field] = [];

                        // Get field for each section id.
                        $rows = Field::findBySQL("user_id = '' AND name = ?",
                            [$field]);
                        $sectionToVal = [];
                        foreach ($rows as $row) {
                            $sectionToVal[$row['block_id']] = $row['json_data'];
                        }

                        foreach ($sectionIds as $sectionId) {
                            $fields[$field][$sectionId] =
                                $sectionToVal[$sectionId] ?
                                $sectionToVal[$sectionId] : 0;
                        }
                    }
                }
            }
        }
        return $fields;
    }

    /**
     * Sets field values for the given course.
     *
     * @param string $courseId id of the course
     * @param array $valueInput field values as an array of the form
     * [ fieldName => [ sectionId => value ] ]
     * @return void
     */
    public function setCostFunctionFields($courseId, $valueInput) {
        foreach ($valueInput as $field => $values) {
            foreach ($values as $sectionId => $val) {
                $fieldRep = Field::find([$sectionId, '', $field]);
                if ($fieldRep === null) {
                    $fieldRep = new Field();
                    $fieldRep->block_id = $sectionId;
                    $fieldRep->user_id = '';
                    $fieldRep->name = $field;
                }
                $fieldRep->json_data = $val;
                $fieldRep->store();
            }
        }
    }

}
