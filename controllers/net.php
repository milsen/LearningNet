<?php

use Mooc\DB\Block;
use LearningNet\NetworkCalculations;
use LearningNet\DB\Networks;

class NetController extends PluginController {

    private $executableInterface;

    public function __construct($dispatcher)
    {
        parent::__construct($dispatcher);
        $this->executableInterface = new NetworkCalculations($this->plugin->getPluginPath());
    }

    /**
     * Action for net/index.php
     */
    public function index_action()
    {
        $this->setupPage('index');

        // Find out whether the correct Courseware version is installed.
        // Member variables can be used in corresponding view.
        $this->cwActivated = $this->coursewareInstalled();
    }

    /**
     * Action for net/edit.php
     */
    public function edit_action()
    {
        $this->setupPage('edit');
    }

    /**
     * Action for net/settings.php
     */
    public function settings_action()
    {
        $this->setupPage('settings');
    }

    /**
     * AJAX: Get representation of the network for a certain seminar_id
     * If getUserData is set, get activity of nodes and recommended path for
     * a certain seminar_id/user as well.
     */
    public function network_action()
    {
        $courseId = \Request::get('cid');
        $getUserData = \Request::get('getUserData');
        $graphRep = LearningNet\DB\Networks::find($courseId);

        $network = "";
        if ($graphRep === null) {
            // Create new network with one isolated node for each section.
            $sections = Mooc\DB\Block::findBySQL(
                "type = 'Section' AND seminar_id = ?", array($courseId));
            $sectionIds = array_map(function ($sec) { return $sec['id']; }, $sections);
            $network = $this->executableInterface->createNetwork($sectionIds)['message'];

            // Store new network in database.
            $graphRep = new LearningNet\DB\Networks();
            $graphRep->seminar_id = $courseId;
            $graphRep->network = $network;
            $graphRep->store();
        } else {
            // Get graph from database if possible.
            $network = $graphRep->network;
        }

        if ($getUserData === "true") {
            //  Get completed sections.
            $userId = isset($GLOBALS['user']) ? $GLOBALS['user']->id : 'nobody';
            $completed = Mooc\DB\Field::findBySQL(
                "user_id = ? AND name = 'visited' AND json_data = 'true'", array($userId));
            $completedIds = array_map(function ($sec) { return $sec['block_id']; }, $completed);
            $completedIds = join(" ", $completedIds);

            $network = $this->executableInterface->getActives(
                $graphRep->network, $completedIds
            )['message'];
        }

        $this->render_text($network);
    }

    /**
     * AJAX: Get map of ids to titles of all Courseware sections
     */
    public function section_titles_action()
    {
        $courseId = \Request::get('cid');
        $sectionTitles = array();

        $sections = Mooc\DB\Block::findBySQL(
            "type = 'Section' AND seminar_id = ?", array($courseId));
        foreach ($sections as $section) {
            $sectionTitles[$section['id']] = $section['title'];
        }

        $this->render_json($sectionTitles);
    }

    /**
     * AJAX: Store dot representation of the network for a certain seminar_id
     */
    public function store_action()
    {
        $courseId = \Request::get('cid');
        $network = \Request::get('network');

        $checkObj = $this->executableInterface->checkNetwork($network);

        if ($checkObj['succeeded']) {
            $graphRep = LearningNet\DB\Networks::find($courseId);
            $graphRep->network = $network;
            $graphRep->store();

            $this->render_html(MessageBox::success('Network stored successfully!'));
        } else {
            $this->render_html(MessageBox::error(
                'Network could not be stored successfully: ' . $checkObj['message']
            ));
        }
    }

    /**
     * Activate navigation item and load javascript for page.
     */
    private function setupPage($keyword) {
        $item = '/course/learningnet/' . $keyword;

        // Activate navigation item.
        if (Navigation::hasItem($item)) {
            Navigation::activateItem($item);
        }

        PageLayout::addScript(
            $this->plugin->getPluginURL() . '/assets/dist/' . $keyword . '.js'
        );
    }

    /**
     * @return bool whether the correct version of Courseware is installed.
     */
    private function coursewareInstalled()
    {
        $plugin_manager = PluginManager::getInstance();
        if ($plugin_manager->getPluginInfo('Courseware') == null) {
            return false;
        }

        $version = $plugin_manager->getPluginManifest(
            $plugin_manager->getPlugin('Courseware')->getPluginPath()
        )['version'];
        return version_compare('4.4.2', $version) >= 0;
    }

    /**
     * Callback function being called before an action is executed.
     */
    public function before_filter(&$action, &$args)
    {
        parent::before_filter($action, $args);

        // Set help keyword (page in the help wiki).
        PageLayout::setHelpKeyword('LearningNet.LearningNet');

        // Set help bar content.
        $description = _ln('Mit LearningNet können Lernmodule, die in Courseware erstellt wurden, in einem Netz angeordnet werden. Den Studierenden wird anhand auswählbarer Kriterien empfohlen, welche Lernmodule sie als nächstes bearbeiten sollen. Das Abarbeiten individualisierter Lernpfade ermöglicht es den Studierenden, effektiver zu lernen.');
        Helpbar::get()->addPlainText(_ln('Information'), $description, 'icons/white/info-circle.svg');
    }

    /**
     * Render Stud.IP specific HTML
     */
    private function render_html($html)
    {
        $this->response->add_header('Content-Type', 'text/html;charset=utf-8');
        $this->render_text($html);
    }
}
