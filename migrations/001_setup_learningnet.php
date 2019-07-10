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
          `network` varchar(64) NOT NULL,
          PRIMARY KEY (`id`)
        )");

        /* $db->exec("CREATE TABLE IF NOT EXISTS `mooc_fields` ( */
        /*   `block_id` int(11) NOT NULL, */
        /*   `user_id` varchar(32) NOT NULL, */
        /*   `name` varchar(64) NOT NULL, */
        /*   `json_data` mediumtext, */
        /*   PRIMARY KEY (`block_id`,`user_id`,`name`) */
        /* )"); */

        /* $db->exec("CREATE TABLE IF NOT EXISTS `mooc_userprogress` ( */
        /*   `block_id` int(11) NOT NULL, */
        /*   `user_id` varchar(32) NOT NULL DEFAULT '', */
        /*   `grade` double DEFAULT NULL, */
        /*   `max_grade` double NOT NULL DEFAULT '1', */
        /*   PRIMARY KEY (`block_id`,`user_id`) */
        /* )"); */

        if (is_null(Config::get()->getValue(\LearningNet\PLUGIN_DISPLAY_NAME_ID))) {
            Config::get()->create(\LearningNet\PLUGIN_DISPLAY_NAME_ID, array(
                'value'       => 'LearningNet',
                'is_default'  => 1,
                'type'        => 'string',
                'range'       => 'global',
                'section'     => 'global',
                'description' => 'Angezeigter Name des Plugins'
            ));
        }

        SimpleORMap::expireTableScheme();
    }

    public function down()
    {
        // To avoid data loss, nothing is deleted by default
        // remove the following "return;"-statement to clean tables on uninstall
        return;

        DBManager::get()->exec("DROP TABLE learningnet_networks");

        Config::get()->delete(\LearningNet\PLUGIN_DISPLAY_NAME_ID);

        SimpleORMap::expireTableScheme();
    }
}
