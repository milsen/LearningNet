import css from 'CSS/network.css';
import * as lgf from 'JS/lgf.js';
import dagreD3 from 'dagre-d3';
import graphlib from 'graphlib';
import * as d3 from 'd3';

const NETWORK_ROUTE = 'network';
const LABELS_ROUTE = 'labels';
const CONDITION_BRANCH_ELSE_KEYWORD = 'SONST';

/**
 * @param {number} node type represented by an int value
 * @return {string} name of the node type, used as a css class
 */
function typeToClass(type) {
    type = parseInt(type);
    if (type === 0) {
        return "inactive";
    } else if (type === 1) {
        return "active";
    } else if (type === 2) {
        return "completed";
    } else if (type === 10) {
        return "split";
    } else if (type === 11) {
        return "condition";
    } else if (type === 12) {
        return "test";
    } else if (type >= 20) {
        return "join";
    }
}

/**
 * @param {Object} node for which a label is created
 * @param {string} name title of the node
 * @return {Element} svg label with a hyperlink redirecting to the corresponding
 * Courseware section
 */
function createNodeLabel(node, name) {
    node.labelType = 'svg';
    let section = node.ref;

    // Build label of node: link to courseware section.
    let url = window.STUDIP.ABSOLUTE_URI_STUDIP +
        'plugins.php/courseware/courseware' +
        window.location.search + '&selected=' + section;

    // Create the SVG label to pass in, must create in SVG namespace
    // http://stackoverflow.com/questions/7547117/add-a-new-line-in-svg-bug-cannot-see-the-line
    // This mimics the same way string labels get added in Dagre-D3
    let svg_label = document.createElementNS('http://www.w3.org/2000/svg', 'text');
    let tspan = document.createElementNS('http://www.w3.org/2000/svg','tspan');
    tspan.setAttributeNS('http://www.w3.org/XML/1998/namespace', 'xml:space', 'preserve');
    tspan.setAttribute('dy', '1em');
    tspan.setAttribute('x', '1');
    let link = document.createElementNS('http://www.w3.org/2000/svg', 'a');
    link.setAttributeNS('http://www.w3.org/1999/xlink', 'xlink:href', url);
    link.setAttribute('target', '_self');
    link.textContent = node.pathIndex ? node.pathIndex + ": " + name : name;
    tspan.appendChild(link);
    svg_label.appendChild(tspan);
    return svg_label;
}

/**
 * Adds a css class to the given node/edge.
 *
 * @param {Object} nodeOrEdge node or edge
 * @param {string} className css class that should be added to the node/edge
 */
function classListAdd(nodeOrEdge, className) {
    nodeOrEdge.class = nodeOrEdge.class ?
        nodeOrEdge.class + " " + className :
        className;
}

/**
 * Adds a css class to the given node/edge.
 *
 * @param {Object} nodeOrEdge node or edge
 * @param {string} className css class that the node/edge might already have
 * @return {boolean} whether the given node/edge already has the given class in
 * its class list
 */
function classListHas(nodeOrEdge, className) {
    return nodeOrEdge.class.split(" ").includes(className);
}

/**
 * @param {string} route route of learningnet/net/ for AJAX requests
 * @return {string} full stud ip url with the given route and the current
 * params (i.e. course id)
 */
export function ajaxURL(route) {
    return window.STUDIP.ABSOLUTE_URI_STUDIP + 'plugins.php/learningnet/net/' +
        route + window.location.search;
}

/**
 * Uses AJAX to get the LGF representation of the current learning net and
 * passes this representation to a given callback function.
 *
 * @param {function(string) : void} func function which is supplied with the LGF
 * representation of the current learning net
 * @param {boolean} getUserData whether the graph data should contain
 * information specific to the current user such as completed and active nodes,
 * a recommended learning path etc.
 */
export function withGraphData(func, getUserData = false) {
    $.ajax({
        url: ajaxURL(NETWORK_ROUTE),
        data: { getUserData : getUserData },
        datatype: 'json',
        success: function(data) {
            console.log(data);
            if (data.succeeded) {
                func(data.message);
            } else {
                $("#layout_content").prepend(data.message);
            }
        }
    });
}

/**
 * Uses AJAX to get the information needed to add labels to the nodes and edges
 * of the current learning net.
 *
 * @param {Array} conditionBranchesByID mapping from condition ids to condition
 * values
 * @param {function(Object) : void} func function which is supplied with the
 * label information, i.e. an object with the entries
 *  'section_titles': mapping from section ids to section titles
 *  'test_titles': mapping from test ids to test titles
 *  'condition_titles': mapping from condition ids to condition titles
 *  'condition_branches': mapping of condition ids to (mappings of condition
 *  values to condition value titles)
 */
export function withLabels(conditionBranchesByID, func) {
    $.ajax({
        url: ajaxURL(LABELS_ROUTE),
        data: { conditionBranchesByID : conditionBranchesByID },
        datatype: 'json',
        success: func
    });
}

/**
 * Initializes the learning net svg, adds zoom capability.
 */
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

/**
 * Draws the given learning net as an svg,
 *
 * setupNetwork() must be called beforehand!
 *
 * @param {string} data LGF representation of a learning net
 */
export function drawNetwork(data) {
    let g = lgf.read(data);
    if (g === null) {
        console.log("drawNetwork(): Reading network from data failed.");
        return;
    }

    // TODO Can performance be improved by doing this elsewhere?
    let conditionBranchesByID = {};
    g.nodes().forEach(function(v) {
        let node = g.node(v);
        if (typeToClass(node.type) === "condition") {
            let conditionId = node.ref;
            if (conditionBranchesByID[conditionId] === undefined) {
                conditionBranchesByID[conditionId] = [];
            }

            g.outEdges(v).forEach(function(e) {
                let branch = g.edge(e).condition;
                if (branch != CONDITION_BRANCH_ELSE_KEYWORD) {
                    conditionBranchesByID[conditionId].push(branch);
                }
            });
        }
    });

    // Set styles of nodes: CSS classes, labels etc.
    withLabels(conditionBranchesByID, function(labels) {
        let sectionTitles = labels['section_titles'];
        let testTitles = labels['test_titles'];
        let conditionTitles = labels['condition_titles'];
        let conditionBranches = labels['condition_branches'];

        g.nodes().forEach(function(v) {
            let node = g.node(v);
            node.rx = node.ry = 5;

            classListAdd(node, typeToClass(node.type));
            if (classListHas(node, "split")) {
                node.label = "";
                node.shape = "diamond";
            } else if (classListHas(node, "condition")) {
                node.label = conditionTitles[node.ref];
                node.shape = "diamond";
                g.outEdges(v).forEach(function(e) {
                    let edge = g.edge(e);
                    if (edge.condition == CONDITION_BRANCH_ELSE_KEYWORD) {
                        edge.label = CONDITION_BRANCH_ELSE_KEYWORD;
                    } else {
                        edge.label = conditionBranches &&
                            conditionBranches[node.ref] &&
                            conditionBranches[node.ref][edge.condition] ?
                            conditionBranches[node.ref][edge.condition] :
                            "";
                    }
                });
            } else if (classListHas(node, "test")) {
                node.label = createNodeLabel(node, testTitles[node.ref]);
                node.shape = "diamond";
                g.outEdges(v).forEach(function(e) {
                    let edge = g.edge(e);
                    edge.label = "≥ " + edge.condition + "p";
                });
            } else if (classListHas(node, "join")) {
                // Show how many predecessors have to be completed for the join.
                let activatedInArcs = (parseInt(node.type) - 20).toString();
                node.label = activatedInArcs + "/" + node.ref;
                node.shape = "diamond";
            } else {
                // Label with link.
                node.label = createNodeLabel(node, sectionTitles[node.ref]);
            }

            // Mark visited edges as such.
            g.outEdges(v).forEach(function(e) {
                let edge = g.edge(e);
                if (edge.visited === "1") {
                    classListAdd(edge, "visited");
                }
            });

            // Mark nodes that are part of the learning path.
            if (node.pathIndex) {
                classListAdd(node, "learningpath");
            }
       });

        // Set style of target node.
        let tgtNode = g.node(g.graph().target);
        if (tgtNode) {
            classListAdd(tgtNode, "target");
        }

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
        let svg = d3.select("svg");
        let scaleWidth = g.graph().width + 300;
        let scaleHeight = g.graph().height + 5;
        svg.attr("viewBox", `0 0 ${scaleWidth} ${scaleHeight}`)
           .attr("preserveAspectRatio", "xMidYMid meet");
    });
}
