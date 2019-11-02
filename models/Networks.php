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
    static protected function configure($config = []) {
        $config['db_table'] = 'learningnet_networks';
        parent::configure($config);
    }

    /**
     * Stores network for the given course id.
     *
     * @param string $courseId id of the course
     * @param string $network representation of network in LGF
     * @return void
     */
    static public function save($courseId, $network) {
        $graphRep = self::find($courseId);
        if ($graphRep === null) {
            $graphRep = new self;
            $graphRep->seminar_id = $courseId;
        }
        $graphRep->network = $network;
        $graphRep->store();
    }
}
