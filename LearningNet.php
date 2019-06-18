<?php

/* require_once __DIR__.'/vendor/autoload.php'; */


/**
 * dgettext with domain for this plugin.
 * @param string $message to be passed to dgettext.
 */
function _ln($message)
{
    return dgettext('learningnet', $message);
}


/**
 * TODO
 *
 * @author  <milsen@uos.de>
 */
class LearningNet extends StudIPPlugin implements StandardPlugin
{
    /**
     * @var Container
     */
    private $container;

    public function __construct()
    {
        parent::__construct();
        /* $this->setupAutoload(); */

        // Set text-domain for translations in this plugin.
        bindtextdomain('learningnet', dirname(__FILE__) . '/locale');
    }

    /**
     * Executed when calling the plugin from plugin.php/learningnet/
     */
    public function initialize()
    {
        // Set title.
        $headerLine = class_exists('Context') ?
            Context::getHeaderLine() :
            $_SESSION['SessSemName']['header_line'];
        PageLayout::setTitle($headerLine . ' - ' . $this->getPluginName());
    }

    /**
     * {@inheritdoc}
     */
    public function getTabNavigation($courseId)
    {
        // Otherwise two params are given to URL.
        $cid = $courseId;

        // Create navigation.
        $navigation = new Navigation(
            $this->getPluginName(),
            PluginEngine::getURL($this, compact('cid'), 'net', true)
        );

        // In current StudIP version, images are only shown in top navigation.
        // Add images in case changes to this occur in the future.
        $navigation->setImage(Icon::create('group3', 'info_alt'));
        $navigation->setActiveImage(Icon::create('group3', 'info'));

        // Add subnavigation.
        // TODO edit/properties should be turned off for students
        // TODO disable complete plugin without courseware
        $this->addSubnavigation($navigation, _cw('LearningNet'), 'net');
        $this->addSubnavigation($navigation, _cw('Stuktur bearbeiten'), 'edit');
        $this->addSubnavigation($navigation, _cw('Einstellungen'), 'properties');
        /* $this->addSubnavigation($navigation, _cw('Export'), 'export'); */
        /* $this->addSubnavigation($navigation, _cw('Import'), 'import'); */

        $tabs = array();
        $tabs['learningnet'] = $navigation;
        return $tabs;
    }

    /**
     * Adds a subnavigation to be shown in the left sidebar.
     * @param Navigation navigation Navigation to which a subnavigation is edded.
     * @param string title Label shown in the sidebar.
     * @param string route Subroute of this plugin linked to.
     */
    private function addSubnavigation(&$navigation, $title, $route)
    {
        $url = PluginEngine::getURL($this, compact('cid'), $route, true);
        $nav_item = new Navigation($title, $url);
        $navigation->addSubnavigation($route, $nav_item);
    }

    /**
     * {@inheritdoc}
     */
    public function getIconNavigation($courseId, $last_visit, $user_id)
    {
        // Icon shown in "Veranstaltungen". Not needed.
        return null;
    }

    /**
     * {@inheritdoc}
     */
    public function getInfoTemplate($courseId)
    {
        // Template to be rendered on the course summary page. Not needed.
        return null;
    }

    /**
     * {@inheritdoc}
     */
    public function getMetadata()
    {
        // Metadata to be rendered on the course summary page. Not needed.
        return array();
    }

}
