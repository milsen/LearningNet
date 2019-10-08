<?php

use Mooc\DB\Block;
use Mooc\DB\Field;
use LearningNet\NetworkCalculations;
use LearningNet\ConditionHandler;
use LearningNet\CostFunctionHandler;
use LearningNet\DB\Networks;
use LearningNet\DB\CostFunctions;
use LearningNet\DB\NodeCosts;
use LearningNet\DB\NodePairCosts;

class NetController extends PluginController {

    private $executableInterface;

    public function __construct($dispatcher)
    {
        parent::__construct($dispatcher);
        $this->executableInterface = new NetworkCalculations(
            $this->plugin->getPluginPath()
        );
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
        $courseId = \Request::get('cid');

        // TODO storing of cost functions
        if (\Request::submitted('save_settings')) {
            $weightInput = \Request::getArray('weightinput');
            $costInput = \Request::getArray('costinput');

            foreach ($weightInput as $costFunc => $weight) {
                $costFuncRep = CostFunctions::find([$courseId, $costFunc]);
                if ($costFuncRep === null) {
                    if ($weight != 0) {
                        $costFuncRep = new CostFunctions();
                        $costFuncRep->seminar_id = $courseId;
                        $costFuncRep->cost_func = $costFunc;
                        $costFuncRep->weight = $weight;
                        $costFuncRep->store();
                    }
                } else {
                    if ($weight == 0) {
                        $costFuncRep->delete();
                    } else {
                        $costFuncRep->weight = $weight;
                        $costFuncRep->store();
                    }
                }
            }

            foreach ($costInput as $costFunc => $costs) {
                foreach ($costs as $blockId => $costValue) {
                    if (is_array($costValue)) {
                        foreach ($costValue as $blockIdTo => $costVal) {
                            $nodeCostRep = NodePairCosts::find([$courseId, $costFunc, $blockId, $blockIdTo]);
                            if ($nodeCostRep === null) {
                                $nodeCostRep = new NodePairCosts();
                                $nodeCostRep->seminar_id = $courseId;
                                $nodeCostRep->cost_func = $costFunc;
                                $nodeCostRep->block_id_from = $blockId;
                                $nodeCostRep->block_id_to = $blockIdTo;
                                $nodeCostRep->cost = $costVal;
                                $nodeCostRep->store();
                            } else {
                                $nodeCostRep->cost = $costVal;
                                $nodeCostRep->store();
                            }
                        }
                    } else {
                        $nodeCostRep = NodeCosts::find([$courseId, $costFunc, $blockId]);
                        if ($nodeCostRep === null) {
                            $nodeCostRep = new NodeCosts();
                            $nodeCostRep->seminar_id = $courseId;
                            $nodeCostRep->cost_func = $costFunc;
                            $nodeCostRep->block_id = $blockId;
                            $nodeCostRep->cost = $costValue;
                            $nodeCostRep->store();
                        } else {
                            $nodeCostRep->cost = $costValue;
                            $nodeCostRep->store();
                        }
                    }
                }
            }
        }

        // Collect node & node pair costs in $this->costs, remember their type.
        $nodeCosts = NodeCosts::costs($courseId, true);
        foreach ($nodeCosts as $nodeCost) {
            $nodeCost['type'] = 'node';
        }
        $nodePairCosts = NodePairCosts::costs($courseId, true);
        foreach ($nodePairCosts as $nodePairCost) {
            $nodePairCost['type'] = 'node_pair';
        }
        $this->costs = array_merge($nodeCosts, $nodePairCosts);
        // TODO weight may not be returned because not in join

        // Set weight of each active cost function that was not found to 0.
        $sectionIds = $this->sectionIds($courseId);
        $activeCostFunctions = (new CostFunctionHandler)->getCostFunctions();
        foreach ($activeCostFunctions as $costFunc => $type) {
            if (!array_key_exists($this->costs, $costFunc)) {
                $this->costs[$costFunc]['type'] = $type;
                $this->costs[$costFunc]['weight'] = 0.0;
                $costs = [];
                if ($type == 'node') {
                    foreach ($sectionIds as $sectionId) {
                        $costs[$sectionId] = 0.0;
                    }
                } else if ($type == 'node_pair') {
                    foreach ($sectionIds as $sectionFrom) {
                        foreach ($sectionIds as $sectionTo) {
                            if ($sectionFrom != $sectionTo) {
                                $costs[$sectionFrom][$sectionTo] = 0.0;
                            }
                        }
                    }
                }
                $this->costs[$costFunc]['costs'] = $costs;
            }
        }
    }

    /**
     * AJAX: Get representation of the network for a certain seminar_id
     * If getUserData is set, get activity of nodes and recommended path for
     * a certain seminar_id/user as well.
     */
    public function network_action()
    {
        $output = array(
            'succeeded' => true,
            'message' => '' // if succeeded: network, else: html with error
        );

        $courseId = \Request::get('cid');
        $getUserData = \Request::get('getUserData') === 'true';
        $graphRep = Networks::find($courseId);

        if ($graphRep === null) {
            // Create new network with one isolated node for each section.
            $output = $this->executableInterface->createNetwork(
                $this->sectionIds($courseId)
            );

            // Store new network in database.
            if ($output['succeeded']) {
                $graphRep = new Networks();
                $graphRep->seminar_id = $courseId;
                $graphRep->network = $output['message'];
                $graphRep->store();
            }
        } else {
            // Get graph from database if possible.
            $output['message'] = $graphRep->network;
        }

        if ($output['succeeded'] && $getUserData) {
            // Get completed sections.
            $userId = isset($GLOBALS['user']) ? $GLOBALS['user']->id : 'nobody';
            $completed = Mooc\DB\Field::findBySQL(
                "user_id = ? AND name = 'visited' AND json_data = 'true'", [$userId]);
            $completedIds = array_map(function ($sec) { return $sec['block_id']; }, $completed);

            // Get grades for test blocks.
            $userProgress = Mooc\DB\UserProgress::findBySQL("user_id = ?", [$userId]);
            $testGrades = [];
            foreach ($userProgress as $row) {
                $testGrades[$row['block_id']] = $row['grade'];
            }
            if (empty($testGrades)) {
                // The backend expects an object, so json_encode should output
                // an object even if $testGrades is empty.
                $testGrades = new stdClass();
            }

            // Set active nodes in network.
            $output = $this->executableInterface->getRecommended(
                $output['message'],
                $completedIds,
                (new ConditionHandler())->getConditionValues($userId),
                $testGrades,
                NodeCosts::costs($courseId),
                NodePairCosts::costs($courseId)
            );
        }

        // If something failed, wrap the message in an error box.
        if ($output['succeeded'] === false) {
            $output['message'] = MessageBox::error($output['message'])->__toString();
        }

        $this->render_json($output);
    }

    /**
     * AJAX: Get map of ids to titles of all Courseware sections
     */
    public function labels_action()
    {
        $courseId = \Request::get('cid');
        $conditionBranchesByID = \Request::getInstance()['conditionBranchesByID'];

        // Get section titles.
        $sectionTitles = array();
        $sections = Mooc\DB\Block::findBySQL(
            "type = 'Section' AND seminar_id = ?", array($courseId));
        foreach ($sections as $section) {
            $sectionTitles[$section['id']] = $section['title'];
        }

        // Get test titles, i.e. titles of assignments used in TestBlocks.
        // Normally this could be with a join of mooc_blocks and vips_test, but
        // json_data wraps the assignment_id in double quotes.
        $testTitles = array();
        $tests = Mooc\DB\Block::findBySQL(
            "type = 'TestBlock' AND seminar_id = ?", array($courseId));
        $assignmentRows = Mooc\DB\Field::findBySQL("name = 'assignment_id'");

        $db = \DBManager::get();
        foreach ($assignmentRows as $assignmentRow) {
            $assignmentId = json_decode($assignmentRow['json_data']);
            $stmt = $db->prepare("SELECT title FROM vips_test WHERE id = :id");
            $stmt->bindParam(':id', $assignmentId);
            $stmt->execute();
            $row = $stmt->fetch(\PDO::FETCH_NUM, \PDO::FETCH_ORI_NEXT);
            $testTitles[$assignmentRow['block_id']] = $row[0];
        }

        // Get condition titles.
        $conditionHandler = new ConditionHandler();
        $conditionTitles = $conditionHandler->getConditionTitles();

        // Get condition branch titles.
        $conditionBranches =
            $conditionHandler->getConditionBranches($conditionBranchesByID);

        $this->render_json([
            'section_titles' => $sectionTitles,
            'test_titles' => $testTitles,
            'condition_titles' => $conditionTitles,
            'condition_branches' => $conditionBranches
        ]);
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
            $graphRep = Networks::find($courseId);
            $graphRep->network = $network;
            $graphRep->store();

            $this->render_html(MessageBox::success(_ln('Netzwerk gespeichert.')));
        } else {
            $this->render_html(MessageBox::error(
                _ln('Netzwerk konnte nicht gespeichert werden: ') . $checkObj['message']
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

    /**
     * TODO move to Mooc\DB\Block
     * @return array of Courseware section ids for course with id $courseId
     */
    private function sectionIds($courseId)
    {
        $sections = Mooc\DB\Block::findBySQL(
            "type = 'Section' AND seminar_id = ?", [$courseId]);
        return array_map(function ($sec) { return $sec['id']; }, $sections);
    }
}
