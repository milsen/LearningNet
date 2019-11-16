import * as network from 'JS/network.js';
import * as lgf from 'JS/lgf.js';

// Get the learning net and draw it.
$(function() {
    network.withGraphData(function(data) {
        network.setupNetwork();

        let g = lgf.read(data);
        if (g === null) {
            console.log("Reading network from data failed.");
            return;
        }
        network.drawNetwork(g);
    }, true);
});
