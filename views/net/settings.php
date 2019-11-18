<form method="post" name="cost_form" >
    <h2>Gewichtungen von Kostenfunktionen:</h2>
    <table>
    <?php foreach ($costFunctions as $costFunc => $weight) { ?>
        <tr>
        <td><?=$costFunc ?>:</td>
        <td>
        <input type="number"
            name="weightinput[<?=$costFunc ?>]"
            class="weightinput"
            min="0" max="1" step="0.01"
            value="<?=$weight ?>"
            oninput="this.form['weightrange[<?=$costFunc ?>]'].value=this.value"
            />
        <input type="range"
            name="weightrange[<?=$costFunc ?>]"
            class="weightrange"
            min="0" max="1" step="0.01"
            value="<?=$weight ?>"
            oninput="this.form['weightinput[<?=$costFunc ?>]'].value=this.value"
            />
        </td>
        </tr>
    <?php }?>
    </table>
    <br/>

    <h2>Zusätzlich benötigte Informationen:</h2>
    <table>
    <?php foreach ($inputFields as $field => $arr) { ?>
        <tr>
        <td>
        <?=$field ?> für
        </td>
        <td>
        <select id="<?=$field ?>_sectionselect">
        <?php foreach (array_keys($arr) as $section) { ?>
            <option value="<?=$section ?>">
                <?=$sectionTitles[$section] ?>
            </option>
        <?php } ?>
        </select>
        </td>
        <td>
        <?php foreach ($arr as $section => $val) { ?>
            <input type="number"
                style="display: none"
                data-field="<?=$field ?>"
                name="valueinput[<?=$field ?>][<?=$section ?>]"
                class="valueinput"
                min="0" max="10" step="1"
                value="<?=$val ?>"
                />
        <?php } ?>
        </td>
        </tr>
    <?php } ?>
    </table>
    <br/>
    <br/>

    <button type="submit" name="save_settings" value="1">Einstellungen speichern</button>
</form>
