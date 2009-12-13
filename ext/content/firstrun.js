window.addEventListener("load", FirstRunCheck, false);
 
function FirstRunCheck()
{
    var prefs = Components.classes["@mozilla.org/preferences-service;1"]
                .getService(Components.interfaces.nsIPrefService)
                .getBranch("extensions.bonjourfoxy.");
    var curRev = prefs.getCharPref("rev");
    var newRev = Components.classes["@mozilla.org/extensions/manager;1"]
                .getService(Components.interfaces.nsIExtensionManager)
                .getItemForID("bonjourfoxy@bonjourfoxy.net")
                .version;
    var now=new Date();
    if(curRev != newRev) {
        prefs.setCharPref("rev", newRev);
        setTimeout(AddToolbarItem, 1);
        setTimeout(function() { toggleSidebar("viewBonjourServices"); }, 2);
        setTimeout(OpenWelcome, 1);
    }
}

function AddToolbarItem()
{
    try {
        var firefoxnav = window.parent.document.getElementById("nav-bar");
        var curSet = firefoxnav.currentSet;
        if (curSet.indexOf("bonjourfoxy-toolbar-button") == -1)
        {
            var set;
            if (curSet.indexOf("home-button") != -1)
            {
                set = curSet.replace(/home-button/, "home-button,bonjourfoxy-toolbar-button");
            }
            else
            {
                set = curSet + ",bonjourfoxy-toolbar-button";
            }
            firefoxnav.setAttribute("currentset", set);
            firefoxnav.currentSet = set;
            document.persist("nav-bar", "currentset");
            // If you don't do the following call, funny things happen
            try {
                BrowserToolboxCustomizeDone(true);
            }
            catch (e) {}
        }
    }
    catch(e) { }
}

function OpenWelcome()
{
    var win = Components.classes['@mozilla.org/appshell/window-mediator;1'].getService(Components.interfaces.nsIWindowMediator).getMostRecentWindow('navigator:browser');
    win.openUILinkIn('chrome://bonjourfoxy/content/firstrun.html', 'tab');
}