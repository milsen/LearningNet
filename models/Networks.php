<?php

namespace LearningNet\DB;

/**
 * TODO.
 *
 * @property string seminar_id database column
 * @property string network database column
 *
 * @author  <milsen@uos.de>
 */
class Networks extends \SimpleORMap
{
    static protected function configure($config = array()) {
        $config['db_table'] = 'learningnet_networks';
        parent::configure($config);
    }

}
