const AJAX_ROUTE = "ajaxdata.php";
let path = window.location.pathname;
// let basePath = path.substring(0, path.lastIndexOf('/') + 1);
let ajaxUrl = window.location.origin + path + "/" + AJAX_ROUTE + window.location.search;
console.log(window.location);
console.log(ajaxUrl);

$.get(ajaxUrl, function(data, status) {
    console.log("DATA: " + data);

    // TODO later: replaceWith
    $("#layout_content").append("<br>" + data);
});
