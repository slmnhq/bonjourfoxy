bonjourfoxy.overlay = {
    initialize: function() {
        bonjourfoxy.lib.addUserPrefsObserver(bonjourfoxy.overlay);
        bonjourfoxy.lib.observerService().addObserver(bonjourfoxy.overlay, "BFServiceTracker_Change", false);
        bonjourfoxy.overlay.displayStatusBarIcon();
        bonjourfoxy.overlay.setStatusBarIcon();
        bonjourfoxy.overlay.displayWelcome();
        document.getElementById("mnuItemBonjourBrowser").addEventListener("command", function() {
            window.open('chrome://bonjourfoxy/content/browser.xul',
                        'bonjourfoxyBrowser',
                        'chrome,resizable=yes,width=600,height=600'
                        );
        }, false);
    },
    shutdown: function() {
        bonjourfoxy.lib.rmvUserPrefsObserver(bonjourfoxy.overlay);
        bonjourfoxy.lib.observerService().removeObserver(bonjourfoxy.overlay, "BFServiceTracker_Change");
    },
    setlabel: function(elid, label) {
        document.getElementById(elid).setAttribute('label', label);
    },
    uistring: function(string) {
        return document.getElementById("string-bundle").getString(string);
    },
    observe: function(subject, topic, data) {
        if (topic == "nsPref:changed")  {
            if (data == "statusbaricon") {
                bonjourfoxy.overlay.displayStatusBarIcon();
            }
        }
        if (topic == "BFServiceTracker_Change")  {
            bonjourfoxy.overlay.setStatusBarIcon();
        }
    },
    displayWelcome: function()  {
        var bfVer = bonjourfoxy.lib.version();
        if (bonjourfoxy.lib.userPrefs().getCharPref("rev") != bfVer)   {
            bonjourfoxy.lib.openLink("chrome://bonjourfoxy/content/welcome.html", "tab");
            bonjourfoxy.lib.userPrefs().setCharPref("rev", bfVer);
        }
    },
    displayStatusBarIcon: function() {
        var showstatusbar = bonjourfoxy.lib.userPrefs().getBoolPref("statusbaricon");
        var display = showstatusbar ? "display:block;" : "display:none;";
        document.getElementById('bonjourStatusIcon').setAttribute("style", display);
    },
    setStatusBarIcon: function() {
        var statusIcon = document.getElementById('bonjourStatusIcon');
        var serviceCount = bonjourfoxy.lib.ServiceTracker().countServices("__ALL__");
        if (serviceCount == 0) {
            statusIcon.setAttribute("image", "chrome://bonjourfoxy/content/status_bw.png");
            statusIcon.setAttribute("tooltiptext", "No services discovered");
        } else {
            statusIcon.setAttribute("image", "chrome://bonjourfoxy/content/status_color.png");
            if (serviceCount == 1) {
                statusIcon.setAttribute("tooltiptext", serviceCount + " service discovered");
            } else {
                statusIcon.setAttribute("tooltiptext", serviceCount + " services discovered");
            }
        }
    }
}
window.addEventListener("load", bonjourfoxy.overlay.initialize, false);
window.addEventListener("unload", bonjourfoxy.overlay.shutdown, false);