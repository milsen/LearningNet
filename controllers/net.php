<?php

use Mooc\DB\Block;
use LearningNet\Network\Network;
use LearningNet\DB\Networks;

class NetController extends PluginController {

    /**
     * Action for net/index.php
     */
    public function index_action()
    {
        // Find out whether the correct Courseware version is installed.
        // Member variables can be used in corresponding view.
        $this->cwActivated = $this->coursewareInstalled();

        // Get all Courseware section ids of this course.
        $courseId = \Request::option('cid');
        $this->cid = $courseId;
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

    public function ajaxdata_action()
    {
        $graphRep = LearningNet\DB\Networks::find($courseId);

        $network = "";
        if ($graphRep) {
            // Get graph from database if possible.
            $network = $graphRep->network;
        } else {
            // Create new network with one isolated node for each section.
            $network = new LearningNet\Network\Network();
            $courseId = \Request::get('cid');
            $sections = Mooc\DB\Block::findBySQL(
                "type = 'Section' AND seminar_id = ?", array($courseId));
            $sectionIds = array_map(function ($sec) { return $sec['id']; }, $sections);
            $network = "digraph D { " . join("; ", $sectionIds) . "; }";
        }

        $this->data = $network;
    }

    /**
     * Callback function being called before an action is executed.
     */
    public function before_filter(&$action, &$args)
    {
        parent::before_filter($action, $args);

        // Activate navigation item.
        if (Navigation::hasItem('/course/learningnet/net')) {
            Navigation::activateItem('/course/learningnet/net');
        }

        // Set help keyword (page in the help wiki).
        PageLayout::setHelpKeyword('LearningNet.LearningNet');

        // Set help bar content.
        $description = _ln('Mit LearningNet können Lernmodule, die in Courseware erstellt wurden, in einem Netz angeordnet werden. Den Studierenden wird anhand auswählbarer Kriterien empfohlen, welche Lernmodule sie als nächstes bearbeiten sollen. Das Abarbeiten individualisierter Lernpfade ermöglicht es den Studierenden, effektiver zu lernen.');
        Helpbar::get()->addPlainText(_ln('Information'), $description, 'icons/white/info-circle.svg');

        PageLayout::addScript($this->plugin->getPluginURL().'/assets/dist/net.js');
    }
}
