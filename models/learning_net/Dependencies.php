<?php

/* ? namespace LearningNet\DB; */

/**
 * TODO.
 *
 * @author  <milsen@uos.de>
 */
class Dependencies extends \SimpleORMap
{
    private $default = null;

    static protected function configure($config = array()) {
        $config['db_table'] = 'ln_dependencies';
        parent::configure($config);
    }
}
