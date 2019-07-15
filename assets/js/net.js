import css from 'CSS/network.css';
import dagreD3 from 'dagre-d3';
import dot from 'graphlib-dot';
import * as d3 from 'd3';

$(function() {
    const AJAX_ROUTE = "ajaxdata.php";
    let path = window.location.pathname;
    // let basePath = path.substring(0, path.lastIndexOf('/') + 1);
    let ajaxUrl = window.location.origin + path + "/" + AJAX_ROUTE + window.location.search;
    console.log(window.location);
    console.log(ajaxUrl);

    $.get(ajaxUrl, function(data, status) {
        console.log("DATA: " + data);

        // Create the input graph
        // TODO try / catch for wrong input data
        let g = dot.read(data);

        // Round the corners of the nodes
        g.nodes().forEach(function(v) {
            let node = g.node(v);
            node.rx = node.ry = 5;
        });

        // Create the renderer
        let render = new dagreD3.render();

        // Set up an SVG group so that we can translate the final graph.
        let svg = d3.select("svg");
        let svgGroup = svg.append("g");

        // Run the renderer. This is what draws the final graph.
        render(d3.select("svg g"), g);

        // Center the graph
        svg.attr("height", g.graph().height + 5);
        svg.attr("width", g.graph().width + 5);
    });
});
