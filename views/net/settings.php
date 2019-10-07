<form>
    <?php print_r($costs)?>
    <?php foreach ($costs as $name => $costFunc) { ?>
        <?php $weight = $costFunc['weight']; ?>
        <?php $type = $costFunc['type']; ?>
        <?php $costValues = $costFunc['cost']; ?>
        <h2><?=$name ?>:</h2>
        <input type="range" id="<?=$name ?>_weight_range"
            name="<?=$name ?>_weight_range"
            class="weight_range"
            min="0" max="1" step="0.01"
            value="<?=$weight ?>"
            oninput="this.form.<?=$name ?>_weight_input.value=this.value"
            />
        <input type="number"
            name="<?=$name ?>_weight_input"
            class="weight_input"
            min="0" max="1" step="0.01"
            value="<?=$weight ?>"
            oninput="this.form.<?=$name ?>_weight_range.value=this.value"
            />
        <br/>
        <?php if ($type == 'node') { ?>
            <?php foreach ($costValues as $section => $cost) { ?>
                <input type="number"
                    name="<?=$name ?>_<?=$section ?>_cost_input"
                    class="cost_input"
                    min="0" max="100" step="1"
                    value="<?=$cost ?>"
                    />
            <?php } ?>
        <?php } else if ($type == 'node_pair') { ?>
            <?php foreach ($costValues as $sectionFrom => $sectionToCost) { ?>
                <?php foreach ($sectionToCost as $sectionTo => $cost) { ?>
                    <input type="number"
                        name="<?=$name ?>_<?=$sectionFrom ?>-<?=$sectionTo ?>_cost_input"
                        class="cost_input"
                        min="0" max="100" step="1"
                        value="<?=$cost ?>"
                        />
                <?php } ?>
            <?php } ?>
        <?php } ?>
    <?php } ?>
</form>
