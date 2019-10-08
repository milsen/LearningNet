<form method="post" name="cost_form" >
    <?php foreach ($costs as $name => $costFunc) {
        $weight = $costFunc['weight'];
        $type = $costFunc['type'];
        $costValues = $costFunc['costs']; ?>
        <h2><?=$name ?>:</h2>
        <input type="range"
            name="weightrange[<?=$name ?>]"
            class="weightrange"
            min="0" max="1" step="0.01"
            value="<?=$weight ?>"
            oninput="this.form['weightinput[<?=$name ?>]'].value=this.value"
            />
        <input type="number"
            name="weightinput[<?=$name ?>]"
            class="weightinput"
            min="0" max="1" step="0.01"
            value="<?=$weight ?>"
            oninput="this.form['weightrange[<?=$name ?>]'].value=this.value"
            />
        <br/>
        <?php if ($type == 'node') {
            foreach ($costValues as $section => $cost) { ?>
                <input type="number"
                    name="costinput[<?=$name ?>][<?=$section ?>]"
                    class="costinput"
                    min="0" max="100" step="1"
                    value="<?=$cost ?>"
                    />
            <?php }
        } else if ($type == 'node_pair') {
            foreach ($costValues as $sectionFrom => $sectionToCost) {
                foreach ($sectionToCost as $sectionTo => $cost) { ?>
                    <input type="number"
                        name="costinput[<?=$name ?>][<?=$sectionFrom ?>][<?=$sectionTo ?>]"
                        class="costinput"
                        min="0" max="100" step="1"
                        value="<?=$cost ?>"
                        />
                <?php }
            }
        }
    } ?>
    <button type="submit" name="save_settings" value="1">Einstellungen speichern</button>
</form>
