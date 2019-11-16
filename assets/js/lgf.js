import graphlib from 'graphlib';

/**
 * Assigns column values to the given elem.
 *
 * @param {Object} elem node or edge
 * @param {string[]} columns column values
 * @param {string[]} headers column headers of the currently processed columns
 * @return {boolean} whether the assignment succeeded
 */
function assign(elem, columns, headers) {
    if (columns.length !== headers.length) {
        console.log(`read(lgfInput): Column has length ${columns.length} ` +
            ` but only ${headers.length} headers exist.`);
        return false;
    }
    for (var i = 0; i < columns.length; i++) {
        let attrName = headers[i];
        // LGF labels are node ids, graphlib labels are text labels.
        attrName = attrName === "label" ? "id" : attrName;
        elem[attrName] = columns[i];
    }
    return true;
}

/**
 * Splits a row at spaces, respecting double-quoted strings.
 *
 * @param {string} row row to split
 * @return {string[]} column values of the given row
 */
function parseRow(row) {
    // Split at white space unless you're in a quoted string.
    let columns = row.match(/"(?:\\."|[^"])*"|[^\s]+/g);
    // Remove outer quotes of string values.
    columns.forEach(function(val, index, arr) {
        if (val[0] === "\"" && val.slice(-1) === "\"") {
            arr[index] = JSON.parse(val); //val.slice(1,-1);
        }
    });
    return columns;
}


/**
 * Parses a learning net given in LGF representation and creates the
 * corresponding graph.
 *
 * @param {string} lgfInput LGF representation of a learning net
 * @return {graphlib.Graph} graph corresponding to the given LGF input
 */
export function read(lgfInput) {
    // Initialize directed simple graph.
    let g = new graphlib.Graph({
        directed: true,
        compound: false,
        multigraph: true
    });

    // Needed such that g.graph() is defined.
    g.setGraph({});

    let mode = '';
    let readingHeader = false;
    let nodeHeaders = [];
    let edgeHeaders = [];
    let maxId = -1;

    // For each line in LGF.
    for (let str of lgfInput.split('\n')) {
        str = str.trim();
        if (str[0] === '@') {
            // Section beginning.
            mode = str;
            readingHeader = true;
        } else if (str !== '' && str[0] !== '#') {
            // Not empty, not a comment: Column of data.
            let columns = parseRow(str);
            switch (mode) {
                case '@nodes':
                    if (readingHeader) {
                        nodeHeaders = columns;
                    } else {
                        let obj = {};
                        if (!assign(obj, columns, nodeHeaders)) {
                            return null;
                        }
                        if (!('id' in obj)) {
                            console.log('read(lgfInput): No node label found.');
                            return null;
                        }
                        g.setNode(obj.id, obj);
                        maxId = Math.max(maxId, obj.id);
                    }
                    break;
                case '@arcs':
                    if (readingHeader) {
                        edgeHeaders = columns;
                        // The first two entries do not have headers
                        edgeHeaders = ['src', 'tgt'].concat(edgeHeaders);
                    } else {
                        let obj = {};
                        if (!assign(obj, columns, edgeHeaders)) {
                            return null;
                        }
                        if (!g.node(obj.src) || !g.node(obj.tgt)) {
                            console.log(`read(lgfInput): Nodes ${obj.src} and` +
                                ` ${obj.tgt} not both available for edge creation.`);
                            return null;
                        }
                        g.setEdge(obj.src, obj.tgt, obj);
                    }
                    break;
                case '@attributes':
                    if (columns[0] === 'target') {
                        g.graph().target = columns[1];
                    } else if (columns[0] === 'recommended') {
                        // Extract learning path, set pathIndex for each node.
                        let learningPath = columns[1] === "" ?
                            [] : columns[1].split(" ");
                        g.graph().recommended = learningPath;
                        let index = 1;
                        learningPath.forEach(function(id) {
                            g.node(id).pathIndex = index;
                            index++;
                        });
                    }
                    // Ignore attributes other than target and path.
                    break;
                case '':
                    console.log('read(lgfInput): Found data outside of section.');
                    return null;
                default:
                    console.log('Unknown section in LGF data.')
            }

            readingHeader = false;
        }
    }

    g.maxId = maxId;

    return g;
}

export function write(g) {
    let lgfOutput = [];

    // Nodes.
    lgfOutput.push('@nodes');
    lgfOutput.push('label type ref');
    g.nodes().forEach(function(v) {
        let node = g.node(v);
        let type = node.type === undefined ? 0 : node.type;
        let ref = node.ref === undefined ? 0 : node.ref;
        // It should hold that v == node.id.
        lgfOutput.push(`${v} ${type} ${ref}`);
    });

    // Edges.
    lgfOutput.push('@edges');
    lgfOutput.push('    condition');
    g.outEdges(v).forEach(function(e) {
        let edge = g.edge(e);
        let condition = edge.condition === undefined ? "" : edge.condition;
        condition = `"${condition}"`;
        lgfOutput.push(`${e.v} ${e.w} ${condition}`);
    });

    // Target.
    lgfOutput.push('@attributes');
    lgfOutput.push('target ${g.target}');
    // Recommended path is not exported since it is user-specific.

    return lgfOutput.join('\n');
}
