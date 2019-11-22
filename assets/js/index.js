import * as network from 'JS/network.js';

// Get the learning net and draw it.
$(function() {
    network.withGraphData(function(data) {
        network.setupNetwork();
        network.drawNetwork(data);
    }, true);
});
