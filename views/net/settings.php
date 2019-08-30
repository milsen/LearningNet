<form>
    <?php foreach ($costFunctions as $name => $weight) { ?>
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
    <?php } ?>
</form>
