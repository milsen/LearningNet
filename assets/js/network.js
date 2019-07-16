import css from 'CSS/network.css';
import dagreD3 from 'dagre-d3';
import graphlib from 'graphlib';
import dot from 'graphlib-dot';
import * as d3 from 'd3';

const DATA_ROUTE = 'data';

export function ajaxURL(route) {
    return window.STUDIP.ABSOLUTE_URI_STUDIP + 'plugins.php/learningnet/net/' +
        route + window.location.search;
}

export function withGraphData(func) {
    $.get(ajaxURL(DATA_ROUTE), func);
}

export function setupNetwork() {
    // Set up an SVG group so that we can translate the final graph.
    let svg = d3.select("svg");
    let svgGroup = svg.append("g");

    // Add zoom.
    let zoom = d3.zoom().on("zoom", function() {
        svgGroup.attr("transform", d3.event.transform);
    });
    svg.call(zoom);
}

export function drawNetwork(dotInput) {
    console.log("draw: " + dotInput);
    let g;
    try {
        g = dot.read(dotInput);
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
    // TODO fix graph size, just zoom.
    let svg = d3.select("svg");
    svg.attr("height", g.graph().height + 5);
    svg.attr("width", g.graph().width + 5);
}

export function checkNetwork(dotInput) {
    let graph;
    try {
        graph = dot.read(dotInput);
    } catch (e) {
        return false;
    }

    return graphlib.alg.isAcyclic(graph);
}
