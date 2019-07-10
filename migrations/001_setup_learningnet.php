<?php

require __DIR__.'/../vendor/autoload.php';

/**
 * Setup tables for the LearningNet plugin and add editable display name of the plugin.
 *
 * @author <milsen@uos.de>
 */

class SetupLearningNet extends Migration
{

    public function description()
    {
        return 'Setup tables for the LearningNet plugin and add editable display name of the plugin.';
    }

    public function up()
    {
        $db = DBManager::get();

        // If no LearningNet.installation is found, create the tables.
        // Create table ln_dependencies here.
        $db->exec("CREATE TABLE IF NOT EXISTS `learningnet_networks` (
          `seminar_id` varchar(32) DEFAULT NULL,
          `network` mediumtext NOT NULL,
          PRIMARY KEY (`seminar_id`)
        )");

        SimpleORMap::expireTableScheme();
    }

    public function down()
    {
        // To avoid data loss, nothing is deleted by default
        // remove the following "return;"-statement to clean tables on uninstall
        return;

        DBManager::get()->exec("DROP TABLE learningnet_networks");

        SimpleORMap::expireTableScheme();
    }
}
