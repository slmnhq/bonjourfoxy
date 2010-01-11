bonjourfoxy.options = {
    initialize: function() {
        var strings = document.getElementById("string-bundle");
        var title = strings.getString('title') + " ";
        if (/Win/.test(navigator.platform)) {
            title += strings.getString('winPrefLabel');
        } else {
            title += strings.getString('unixPrefLabel');
        }
        document.getElementById('bonjourfoxy-prefpane').setAttribute('label', title);
        document.getElementById('bonjourfoxyPreferences').setAttribute('title', title);
        document.title = title;
    }
}

window.addEventListener("load", bonjourfoxy.options.initialize, false);
