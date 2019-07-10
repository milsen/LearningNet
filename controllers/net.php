<?php

use LearningNet\Network\Network;
use Fhaculty\Graph\Graph;
use Graphp\GraphViz\GraphViz;
use Mooc\DB\Block;

class NetController extends PluginController {

    /**
     * Action for net/index.php
     */
    public function index_action()
    {
        // Find out whether the correct Courseware version is installed.
        // Member variables can be used in corresponding view.
        $this->cwActivated = $this->coursewareInstalled();

        // Get all Courseware sections of this course.
        $courseId = \Request::option('cid');
        $sectionIds = array_map(function ($entry) { return $entry['id']; },
            Mooc\DB\Block::findBySQL("type = 'Section' AND seminar_id = ?", array($courseId))
        );
        $this->cid = $courseId;
        $this->sections = $sectionIds;

        // Show example graph.
        $graph = new Fhaculty\Graph\Graph();

        $blue = $graph->createVertex('blue');
        $blue->setAttribute('graphviz.color', 'blue');

        $red = $graph->createVertex('red');
        $red->setAttribute('graphviz.color', 'red');

        $edge = $blue->createEdgeTo($red);
        $edge->setAttribute('graphviz.color', 'grey');

        $graphviz = new Graphp\GraphViz\GraphViz();
        $graphviz->setFormat('svg');
        $this->netsvg = $graphviz->createImageHtml($graph);
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

        // Activate navigation item.
        if (Navigation::hasItem('/course/learningnet/net')) {
            Navigation::activateItem('/course/learningnet/net');
        }

        // Set help keyword (page in the help wiki).
        PageLayout::setHelpKeyword('LearningNet.LearningNet');

        // Set help bar content.
        $description = _ln('Mit LearningNet können Lernmodule, die in Courseware erstellt wurden, in einem Netz angeordnet werden. Den Studierenden wird anhand auswählbarer Kriterien empfohlen, welche Lernmodule sie als nächstes bearbeiten sollen. Das Abarbeiten individualisierter Lernpfade ermöglicht es den Studierenden, effektiver zu lernen.');
        Helpbar::get()->addPlainText(_ln('Information'), $description, 'icons/white/info-circle.svg');
    }
}
