<form method="post" name="cost_form" >
    <?php foreach ($costFunctions as $name => $weight) { ?>
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
    <?php } ?>
    <button type="submit" name="save_settings" value="1">Einstellungen speichern</button>
</form>
