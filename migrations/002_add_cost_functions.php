<?php

require __DIR__.'/../vendor/autoload.php';

/**
 * Setup cost functions for the LearningNet plugin.
 *
 * @author <milsen@uos.de>
 */

class AddCostFunctions extends Migration
{
    public function description()
    {
        return 'Setup cost functions for the LearningNet plugin.';
    }

    public function up()
    {
        $db = DBManager::get();

        $db->exec("CREATE TABLE IF NOT EXISTS `learningnet_cost_functions` (
          `seminar_id` varchar(32) DEFAULT NULL,
          `cost_func` varchar(32),
          `weight` numeric(3,2) NOT NULL DEFAULT 0.0,
          CONSTRAINT `weight_interval` CHECK ((`weight` between 0.0 and 1.0)),
          PRIMARY KEY (`seminar_id`, `cost_func`)
        )");

        $db->exec("CREATE TABLE IF NOT EXISTS `learningnet_node_costs` (
          `seminar_id` varchar(32) DEFAULT NULL,
          `cost_func` varchar(32),
          `block_id` int(11) NOT NULL,
          `cost` int(3) NOT NULL DEFAULT 50,
          CONSTRAINT `cost_interval` CHECK ((`cost` between 0 and 100)),
          PRIMARY KEY (`seminar_id`, `cost_func`, `block_id`),
          FOREIGN KEY (`seminar_id`, `cost_func`)
            REFERENCES `learningnet_cost_functions` (`seminar_id`, `cost_func`)
            ON UPDATE CASCADE ON DELETE CASCADE,
          FOREIGN KEY (`block_id`) REFERENCES `mooc_blocks` (`id`)
            ON UPDATE CASCADE ON DELETE CASCADE
        )");

        $db->exec("CREATE TABLE IF NOT EXISTS `learningnet_node_pair_costs` (
          `seminar_id` varchar(32) DEFAULT NULL,
          `cost_func` varchar(32),
          `block_id_from` int(11) NOT NULL,
          `block_id_to` int(11) NOT NULL,
          `cost` int(3) NOT NULL DEFAULT 50,
          CONSTRAINT `cost_interval` CHECK ((`cost` between 0 and 100)),
          PRIMARY KEY (`seminar_id`, `cost_func`, `block_id_from`, `block_id_to`),
          FOREIGN KEY (`seminar_id`, `cost_func`)
            REFERENCES `learningnet_cost_functions` (`seminar_id`, `cost_func`)
            ON UPDATE CASCADE ON DELETE CASCADE,
          FOREIGN KEY (`block_id_from`) REFERENCES `mooc_blocks` (`id`)
            ON UPDATE CASCADE ON DELETE CASCADE,
          FOREIGN KEY (`block_id_to`) REFERENCES `mooc_blocks` (`id`)
            ON UPDATE CASCADE ON DELETE CASCADE
        )");

        SimpleORMap::expireTableScheme();
    }

    public function down()
    {
        // To avoid data loss, nothing is deleted by default
        // remove the following "return;"-statement to clean tables on uninstall
        return;

        DBManager::get()->exec("DROP TABLE learningnet_costs");
        DBManager::get()->exec("DROP TABLE learningnet_node_costs");
        DBManager::get()->exec("DROP TABLE learningnet_node_pair_costs");

        SimpleORMap::expireTableScheme();
    }
}
