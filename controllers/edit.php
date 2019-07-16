<?php

class EditController extends PluginController {

    /**
     * Action for edit/index.php
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
        if (Navigation::hasItem('/course/learningnet/edit')) {
            Navigation::activateItem('/course/learningnet/edit');
        }

        PageLayout::addScript($this->plugin->getPluginURL().'/assets/dist/edit.js');
    }
}
