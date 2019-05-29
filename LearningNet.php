<?php

/**
 * TODO
 *
 * @author  <milsen@uos.de>
 * TODO StandardPlugin or other Plugin
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
        /* $this->setupContainer(); */

        /* // more setup if this plugin is active in this course */
        /* if ($this->isActivated($this->container['cid'])) { */
        /*     // markup for link element to courseware */
        /*     StudipFormat::addStudipMarkup('courseware', '\[(mooc-forumblock):([0-9]{1,32})\]', null, 'Courseware::markupForumLink'); */

        /*     $this->setupNavigation(); */
        /* } */

        /* // set text-domain for translations in this plugin */
        /* bindtextdomain('courseware', dirname(__FILE__).'/locale'); */
    }

    public function getPluginname()
    {
        return 'LearningNet';
    }

    // bei Aufruf des Plugins über plugin.php/mooc/...
    public function initialize()
    {
        /* PageLayout::setHelpKeyword('MoocIP.Courseware'); // Hilfeseite im Hilfewiki */
        /* $this->getHelpbarContent(); */
    }

    /**
     * {@inheritdoc}
     */
    public function getTabNavigation($courseId)
    {
        $cid = $courseId;
        $tabs = array();

        /* $courseware = $this->container['current_courseware']; */

        /* $navigation = new Navigation( */
        /*     $courseware->title, */
        /*     PluginEngine::getURL($this, compact('cid'), 'courseware', true) */
        /* ); */
        /* $navigation->setImage(Icon::create('group3', 'info_alt')); */
        /* $navigation->setActiveImage(Icon::create('group3', 'info')); */
        /* $tabs['mooc_courseware'] = $navigation; */

        /* $navigation->addSubnavigation('index', clone $navigation); */


        /* //NavigationForLecturers */
        /* if ($this->container['current_user']->hasPerm($courseId, 'tutor')) { */
        /*     $managerUrl = PluginEngine::getURL($this, compact('cid'), 'block_manager', true); */
        /*     $navigation->addSubnavigation( */
        /*         'block_manager', */
        /*         new Navigation(_cw('Struktur bearbeiten'), $managerUrl) */
        /*     ); */
        /*     $settingsUrl = PluginEngine::getURL($this, compact('cid'), 'courseware/settings', true); */
        /*     $navigation->addSubnavigation( */
        /*         'settings', */
        /*         new Navigation(_cw('Einstellungen'), $settingsUrl) */
        /*     ); */
        /*     $navigation->addSubnavigation( */
        /*         'news', */
        /*         new Navigation( */
        /*             _cw('Letzte Änderungen'), */
        /*             PluginEngine::getURL($this, compact('cid'), 'courseware/news', true) */
        /*         ) */
        /*     ); */
        /*     $cpoUrl = PluginEngine::getURL($this, compact('cid'), 'cpo', true); */
        /*     $navigation->addSubnavigation( */
        /*         'progressoverview', */
        /*         new Navigation(_cw('Fortschrittsübersicht'), $cpoUrl) */
        /*     ); */

        /*     $postoverviewUrl = PluginEngine::getURL($this, compact('cid'), 'cpo/postoverview', true); */
        /*     $navigation->addSubnavigation( */
        /*         'postoverview', */
        /*         new Navigation(_cw('Diskussionsübersicht'), $postoverviewUrl) */
        /*     ); */
        /*     $exportUrl = PluginEngine::getURL($this, compact('cid'), 'export', true); */
        /*     $navigation->addSubnavigation( */
        /*         'export', */
        /*         new Navigation(_cw('Export'), $exportUrl) */
        /*     ); */
        /*     $importUrl = PluginEngine::getURL($this, compact('cid'), 'import', true); */
        /*     $navigation->addSubnavigation( */
        /*         'import', */
        /*         new Navigation(_cw('Import'), $importUrl) */
        /*     ); */

        /* //NavigationForStudents */
        /* } else { */
        /*     if (!$this->container['current_user']->isNobody()) { */
        /*         $navigation->addSubnavigation( */
        /*             'news', */
        /*             new Navigation( */
        /*                 _cw('Letzte Änderungen'), */
        /*                 PluginEngine::getURL($this, compact('cid'), 'courseware/news', true) */
        /*             ) */
        /*         ); */
        /*         $progressUrl = PluginEngine::getURL($this, compact('cid'), 'progress', true); */
        /*         $navigation->addSubnavigation( */
        /*             'progress', */
        /*             new Navigation(_cw('Fortschrittsübersicht'), $progressUrl) */
        /*         ); */
        /*     } */
        /* } */

        return $tabs;
    }

    /**
     * {@inheritdoc}
     */
    public function getIconNavigation($courseId, $last_visit, $user_id)
    {
        /* if (!$user_id) { */
        /*     $user_id = $GLOBALS['user']->id; */
        /* } */
        /* $icon = new AutoNavigation( */
        /*     $this->getDisplayTitle(), */
        /*     PluginEngine::getURL($this, array('cid' => $courseId, 'iconnav' => 'true'), 'courseware/news', true) */
        /* ); */

        /* $db = DBManager::get(); */

        /* $stmt = $db->prepare(' */
        /*     SELECT */
        /*         COUNT(*) */
        /*     FROM */
        /*         mooc_blocks */
        /*     WHERE */
        /*         seminar_id = :cid */
        /*     AND */
        /*         chdate >= :last_visit */
        /* '); */
        /* $stmt->bindParam(':cid', $courseId); */
        /* $stmt->bindParam(':last_visit', $last_visit); */
        /* $stmt->execute(); */
        /* $new_ones = (int) $stmt->fetch(PDO::FETCH_ASSOC)['COUNT(*)']; */

        /* $plugin_manager = \PluginManager::getInstance(); */
        /* $vips = true; */
        /* if ($plugin_manager->getPluginInfo('VipsPlugin') == null){ */
        /*     $vips = false; */
        /* } */
        /* if($plugin_manager->getPlugin('VipsPlugin')){ */
        /*     $version = $plugin_manager->getPluginManifest($plugin_manager->getPlugin('VipsPlugin')->getPluginPath())['version']; */
        /*     if (version_compare('1.3',$version) > 0) { */
        /*         $vips = false; */
        /*     } */
        /* } else { */
        /*     $vips = false; */
        /* } */

        /* if ($vips) { */
        /*     // getting all tests */
        /*     $stmt = $db->prepare(" */
        /*         SELECT */
        /*             json_data */
        /*         FROM */
        /*             mooc_blocks */
        /*         JOIN */
        /*             mooc_fields */
        /*         ON */
        /*             mooc_blocks.id = mooc_fields.block_id */
        /*         WHERE */
        /*             mooc_blocks.type = 'TestBlock' */
        /*         AND */
        /*             mooc_blocks.seminar_id = :cid */
        /*         AND */
        /*             mooc_fields.name = 'test_id' */
        /*     "); */
        /*     $stmt->bindParam(':cid', $courseId); */
        /*     $stmt->execute(); */

        /*     $tests = $stmt->fetch(PDO::FETCH_ASSOC); */
        /*     if ($tests) { */
        /*         $test_ids = array(); */
        /*         foreach ($tests as $key => $value) { */
        /*             array_push($test_ids, (int) str_replace('"', '', $value)); */
        /*         } */
        /*         //looking for new tests */
        /*         $stmt = $db->prepare(' */
        /*             SELECT */
        /*                 COUNT(*) */
        /*             FROM */
        /*                 vips_exercise_ref */
        /*             JOIN */
        /*                 vips_exercise */
        /*             ON */
        /*                 vips_exercise_ref.exercise_id = vips_exercise.ID */
        /*             WHERE */
        /*                 vips_exercise_ref.test_id IN ('.implode(', ', $test_ids).') */
        /*             AND */
        /*                 unix_timestamp(created) >=  :last_visit */
        /*         '); */
        /*         $stmt->bindParam(':last_visit', $last_visit); */
        /*         $stmt->execute(); */
        /*         $new_ones += (int) $stmt->fetch(PDO::FETCH_ASSOC)['COUNT(*)']; */
        /*     } */
        /* } */
        /* if ($new_ones) { */
        /*     $title = $new_ones > 1 ? sprintf(_('%s neue Courseware-Inhalte'), $new_ones) : _('1 neuer Courseware-Inhalt'); */
        /*     $icon->setImage(Icon::create('group3', 'attention', ['title' => $title])); */
        /*     $icon->setBadgeNumber($new_ones); */
        /* } else { */
        /*     $icon->setImage(Icon::create('group3', 'inactive', ['title' => 'Courseware'])); */
        /* } */

        /* return $icon; */
        return null;
    }

    /**
     * {@inheritdoc}
     */
    public function getInfoTemplate($courseId)
    {
        return null;
    }

}
