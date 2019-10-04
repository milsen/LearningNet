<?php

namespace LearningNet\DB;

/**
 * TODO.
 *
 * @property string seminar_id database column
 * @property string cost_func database column
 * @property double weight database column
 *
 * @author  <milsen@uos.de>
 */
class CostFunctions extends \SimpleORMap
{
    static protected function configure($config = array()) {
        $config['db_table'] = 'learningnet_cost_functions';
        parent::configure($config);
    }
}
