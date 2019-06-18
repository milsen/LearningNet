<?php

class PropertiesController extends PluginController {

    /**
     * Action for properties/index.php
     */
    public function index_action()
    {
    }

    /**
     * Callback function being called before an action is executed.
     */
    public function before_filter(&$action, &$args)
    {
        parent::before_filter($action, $args);

        // Activate navigation item.
        if (Navigation::hasItem('/course/learningnet/properties')) {
            Navigation::activateItem('/course/learningnet/properties');
        }
    }
}
