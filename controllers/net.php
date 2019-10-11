<?php

use Mooc\DB\Block;
use Mooc\DB\Field;
use Mooc\DB\UserProgress;
use LearningNet\NetworkCalculations;
use LearningNet\ConditionHandler;
use LearningNet\CostFunctionHandler;
use LearningNet\DB\Networks;
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
        $costFunctionHandler = new CostFunctionHandler();

        if (\Request::submitted('save_settings')) {
            $weightInput = \Request::getArray('weightinput');
            $costFunctionHandler->setCostFunctionWeights($courseId, $weightInput);
        }

        $this->costFunctions = $costFunctionHandler->getCostFunctionWeights($courseId);
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
                Networks::save($courseId, $output['message']);
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
            $testGrades = [];
            $rows = Mooc\DB\UserProgress::findBySQL("INNER JOIN mooc_blocks
                ON mooc_userprogress.block_id = mooc_blocks.id
                WHERE user_id = :user_id
                AND seminar_id = :course_id
            ", ['course_id' => $courseId, 'user_id' => $userId]);
            foreach ($rows as $row) {
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
        $testTitles = [];
        $testRows = Mooc\DB\Field::findBySQL("INNER JOIN mooc_blocks
            ON mooc_fields.block_id = mooc_blocks.id
            WHERE type = 'TestBlock'
            AND name = 'assignment_id'
            AND seminar_id = :course_id
        ", ['course_id' => $courseId]);

        // Normally this could be with a join of mooc_blocks and vips_test, but
        // json_data wraps the assignment_id in double quotes...
        $assignmentToBlockIds = [];
        foreach ($testRows as $testRow) {
            $assignmentToBlockIds[json_decode($testRow['json_data'])] = $testRow['block_id'];
        }
        $db = \DBManager::get();
        $stmt = $db->prepare("SELECT * FROM vips_test WHERE id IN (:ids)");
        $stmt->execute(["ids" => array_keys($assignmentToBlockIds)]);
        foreach ($stmt->fetchAll() as $row) {
            $testTitles[$assignmentToBlockIds[$row['id']]] = $row['title'];
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
            Networks::save($courseId, $network);
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
