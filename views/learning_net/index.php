LearningNet für Kurs <?= $cid; ?>: <br>

<?php
    if ($cwActivated) {
        print_r($sections);
    } else {
        echo _ln("Courseware (Version >= 4.4.2) muss installiert sein, um LearningNet nutzen zu können.");
    }
?>

<?php
    // Add Sidebar.
    /* $actions = new ActionsWidget(); */
    /* $actions->addLink(_ln('Netz bearbeiten'), */
    /*     PluginEngine::getURL($plugin,array(), 'edit_net'), */
    /*     null, */
    /*     array('data-dialog' => '1')); */
    /* Sidebar::Get()->addWidget($actions); */
?>
