import * as network from 'JS/network.js';

$(function() {
    network.withGraphData(function(data) {
        network.setupNetwork();
        network.drawNetwork(data);
    }, true);
});
