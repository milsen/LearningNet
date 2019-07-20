LearningNet für Kurs <?= $cid; ?>: <br>
<?php
    print_r($out);
?>
<br>

<?php
    if ($cwActivated) {
        print_r($sections);
    } else {
        echo _ln("Courseware (Version >= 4.4.2) muss installiert sein, um LearningNet nutzen zu können.");
    }
?>

<br>
<svg></svg>
