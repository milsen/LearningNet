import graphlib from 'graphlib';

function assign(elem, columns, headers) {
    // if (columns.length !== headers.length) {
        //TODO error?
    // }
    for (var i = 0; i < columns.length; i++) {
        elem[headers[i]] = columns[i];
    }
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
            let columns = str.split(/\s+/);
            switch (mode) {
                case '@nodes':
                    if (readingHeader) {
                        nodeHeaders = columns;
                    } else {
                        let obj = {};
                        assign(obj, columns, nodeHeaders);
                        if (!('label' in obj)) {
                            console.log("read(lgfInput): No node label found.");
                            return null;
                        }
                        g.setNode(obj.label, obj);
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
