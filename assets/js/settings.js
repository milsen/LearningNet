import css from 'CSS/settings.css';

/**
 * Hides all value inputs belonging to the given select element except the one
 * that is selected.
 *
 * @param {jQuery} select element
 */
function selectSectionInput(jquerySelect) {
    let field = jquerySelect.attr('id').split("_")[0];
    let section = jquerySelect.val();
    $(`.valueinput[data-field="${field}"]`).hide();
    $(`.valueinput[name="valueinput[${field}][${section}]"]`).show();
}

$(function() {
    // Show the correct value inputs initially.
    $("select").each(function() {
        selectSectionInput($(this));
    });

    // Update which value input should be shown if the selection changes.
    $("select").change(function() {
        selectSectionInput($(this));
    });
});
