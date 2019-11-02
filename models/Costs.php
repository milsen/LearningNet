<?php

namespace LearningNet\DB;

/**
 * TODO.Superclass for NodeCosts and NodePairCosts
 *
 * @author  <milsen@uos.de>
 */
class Costs extends \SimpleORMap
{
    static protected function configure($config = []) {
        parent::configure($config);
    }

    /**
     * @param string $courseId id of the course for which the costs should be found
     * @param callable $func adding to cost array using database row
     * @return array of the form
     * [ { 'weight' => "weight", 'costs' => { func(... "cost" ...) } ]
     */
    static protected function costs($courseId, $func) {
        $dbTable = self::config('db_table');

        // Use DBManager directly to get costs and cost function weights.
        // findBySQL cannot be used since only values from $dbTable would be
        // selected, but we also want to select "weight" from the joined table.
        $db = \DBManager::get();
        $stmt = $db->prepare("SELECT * "
            . "FROM {$dbTable} INNER JOIN learningnet_cost_functions "
            . "ON ({$dbTable}.seminar_id = learningnet_cost_functions.seminar_id "
            . "AND {$dbTable}.cost_func  = learningnet_cost_functions.cost_func ) "
            . "WHERE ({$dbTable}.seminar_id = ?) ORDER BY learningnet_cost_functions.cost_func "
        );
        $stmt->execute([$courseId]);
        $rows = $stmt->fetchAll();
        $result = [];

        // Collect costs in array:
        // [ { 'weight' => "weight", 'costs' => { func(... "cost" ...) } ]
        $prevCostFunc = $rows[0]['cost_func'];
        $prevWeight = $rows[0]['weight'];
        $currentCosts = [];
        foreach ($rows as $row) {
            $costFunc = $row['cost_func'];
            if ($costFunc !== $prevCostFunc) {
                // If a new cost function is found, push cost values for
                // previous cost function.
                $arr = [ 'weight' => $prevWeight, 'costs' => $currentCosts ];
                array_push($result, $arr);

                // Reset variables for new cost function.
                $currentCosts = [];
                $prevCostFunc = $costFunc;
                $prevWeight = $row['weight'];
            }

            // Add costs found in $row to $currentCosts using $func.
            $func($currentCosts, $row);
        }

        // Push last collected costs.
        if (!empty($row)) {
            $arr = [ 'weight' => $prevWeight, 'costs' => $currentCosts ];
            array_push($result, $arr);
        }

        return $result;
    }
}
