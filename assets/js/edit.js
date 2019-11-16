import css from 'CSS/edit.css';
import * as lgf from 'JS/lgf.js';
import * as network from 'JS/network.js';

let g;

/**
 * Draws the network given by the text field with the id 'inputGraph' if the
 * content of that text field has changed.
 * TODO
 */
function doAndRedraw(func) {
    func();
    network.drawNetwork(g);
}

$(function() {
    network.setupNetwork();

    network.withGraphData(function(data) {
        g = lgf.read(data);
        if (g === null) {
            console.log("Reading network from data failed.");
            return;
        }

        // Draw network once initially.
        network.drawNetwork(g);
    });

    // TODO Setup event handlers for gui controls.
    // New connective.
    $('#newnode').click(function() {
        let obj = {
            'id' : ++g.maxId,
            'type' : translate(document.form.nodetype.value),
            'ref' : 0
        };
        doAndRedraw(function() {
            g.setNode(obj.id, obj);
        });
    });

    // Remove connective.
    $('#delnode').click(function() {
        doAndRedraw(function() {
            g.removeNode(obj.id);
        });
    });

    // New edge.
    $('#newedge').click(function() {
        doAndRedraw(function() {
            g.setEdge(obj.src, obj.tgt, obj);
        });
    });

    // Remove edge.
    $('#deledge').click(function() {
        doAndRedraw(function() {
            g.removeEdge(obj.src, obj.tgt);
        });
    });

    const STORE_ROUTE = "store";

    $('#submit').click(function() {
        $.ajax({
            url: network.ajaxURL(STORE_ROUTE),
            type: 'POST',
            data: { network : lgf.write(g) },
            success: function(htmlMsg) {
                console.log(htmlMsg)
                $("#layout_content").prepend(htmlMsg);
            }
        });
    });
});

// TODO
// * add operator nodes
// * delete operator nodes
// * add edges (check acyclicity?)
// * delete edges
// * change conditions
// * change join numbers
