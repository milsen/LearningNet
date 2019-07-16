import css from 'CSS/network.css';
import dagreD3 from 'dagre-d3';
import dot from 'graphlib-dot';
import * as d3 from 'd3';

let oldInputGraphValue = "";

// Draw network us if inputGraph changes.
function drawNetwork(dotInput) {
    let g;
    try {
        g = dot.read(inputGraph.value);
    } catch (e) {
        throw e;
    }

    // Round the corners of the nodes.
    g.nodes().forEach(function(v) {
        let node = g.node(v);
        node.rx = node.ry = 5;
    });

    // Set margin if not present already.
    if (!g.graph().hasOwnProperty("marginx") &&
        !g.graph().hasOwnProperty("marginy")) {
        g.graph().marginx = 20;
        g.graph().marginy = 20;
    }

    // Set transition.
    g.graph().transition = function(selection) {
        return selection.transition().duration(500);
    };

    // Create the renderer.
    let render = new dagreD3.render();

    // Run the renderer. This is what draws the final graph.
    render(d3.select("svg g"), g);

    // Center the graph.
    svg.attr("height", g.graph().height + 5);
    svg.attr("width", g.graph().width + 5);
}

// Draw network again if #inputGraph changes.
function tryDraw() {
    let inputGraph = document.querySelector("#inputGraph");
    if (oldInputGraphValue !== inputGraph.value) {
        oldInputGraphValue = inputGraph.value;
        drawNetwork(oldInputGraphValue);
    }
}

$(function() {
    // Set up an SVG group so that we can translate the final graph.
    let svg = d3.select("svg");
    let svgGroup = svg.append("g");

    // Add zoom.
    let zoom = d3.zoom().on("zoom", function() {
        svgGroup.attr("transform", d3.event.transform);
    });
    svg.call(zoom);

    // Set drawing handler for keyup-event in input form.
    $('#inputGraph').keyup(function() { tryDraw(); });

    // Draw network once initially.
    tryDraw();
});
