import * as network from 'JS/network.js';

let oldInputGraphValue = "";

// Draw network again if #inputGraph changes.
function tryDraw() {
    let inputGraph = document.querySelector("#inputGraph");
    if (oldInputGraphValue !== inputGraph.value) {
        oldInputGraphValue = inputGraph.value;
        network.drawNetwork(oldInputGraphValue);
    }
}

$(function() {
    network.setupNetwork();

    network.withGraphData(function(data) {
        // Set initial input for form.
        $('#inputGraph').val(data);

        // Draw network once initially.
        tryDraw();
    });

    // Set drawing handler for keyup-event in input form.
    $('#inputGraph').keyup(function() { tryDraw(); });
});
