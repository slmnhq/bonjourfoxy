var bonjourfoxy = {};
(function() {
// Registration
var namespaces = [];
this.ns = function(fn)  {
    var ns = {};
    namespaces.push(fn, ns);
    return ns;
};

// Initialization
this.initialize = function()    {
    for (var i=0; i<namespaces.length; i+=2)    {
        var fn = namespaces[i];
        var ns = namespaces[i+1];
        fn.apply(ns);
    }
};

// Clean up
this.shutdown = function()  {
    window.removeEventListener("load", bonjourfoxy.initialize, false);
    window.removeEventListener("unload", bonjourfoxy.shutdown, false);
};

// Register handlers to maintain extension life cycle
window.addEventListener("load", bonjourfoxy.initialize, false);
window.addEventListener("unload", bonjourfoxy.shutdown, false);
}).apply(bonjourfoxy);
