import graphlib from 'graphlib';

function assign(elem, columns, headers) {
    // if (columns.length !== headers.length) {
        //TODO error?
    // }
    for (var i = 0; i < columns.length; i++) {
        let attrName = headers[i];
        // LGF labels are node ids, graphlib labels are text labels.
        attrName = attrName === "label" ? "id" : attrName;
        elem[attrName] = columns[i];
    }
}

function parseRow(row) {
    // Split at white space unless you're in a quoted string.
    let columns = row.match(/[^\s]+|"(?:\\."|[^"])*"/g);
    // Remove outer quotes of string values.
    columns.forEach(function(val, index, arr) {
        if (val[0] === "\"" && val.slice(-1) === "\"") {
            arr[index] = JSON.parse(val); //val.slice(1,-1);
        }
    });
    return columns;
}

export function read(lgfInput) {
    // Initialize directed simple graph.
    let g = new graphlib.Graph({
        directed: true,
        compound: false,
        multigraph: false
    });

    // Needed such that g.graph() is defined.
    g.setGraph({});

    let mode = '';
    let readingHeader = false;
    let nodeHeaders = [];
    let edgeHeaders = [];

    // For each line in LGF.
    lgfInput.split('\n').forEach(function(str) {
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
                        assign(obj, columns, nodeHeaders);
                        if (!('id' in obj)) {
                            console.log('read(lgfInput): No node label found.');
                            return null;
                        }
                        g.setNode(obj.id, obj);
                    }
                    break;
                case '@arcs':
                    if (readingHeader) {
                        edgeHeaders = columns;
                        // The first two entries do not have headers
                        edgeHeaders = ['src', 'tgt'].concat(edgeHeaders);
                    } else {
                        let obj = {};
                        assign(obj, columns, edgeHeaders);
                        g.setEdge(obj.src, obj.tgt, obj);
                    }
                    break;
                case '@attributes':
                    if (columns[0] === 'target') {
                        g.graph().target = columns[1];
                    }
                    // Ignore attributes other than target.
                    break;
                default:
                    console.log('Unknown section in LGF data.')
            }

            readingHeader = false;
        }
    });

    return g;
}
