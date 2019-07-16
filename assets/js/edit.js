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

    const STORE_ROUTE = "store";

    // TODO replace console.log by messages on page
    $('#submit').click(function() {
        if (network.checkNetwork(inputGraph.value)) {
            $.ajax({
                url: network.ajaxURL(STORE_ROUTE),
                type: 'POST',
                data: { network : inputGraph.value },
                success: function(msg) {
                    console.log(msg);
                }
            });
        } else {
            console.log("input graph not correct")
        }
    });
});

// TODO
// * add operator nodes
// * delete operator nodes
// * add edges (check acyclicity?)
// * delete edges
// * change conditions
// * change join numbers
