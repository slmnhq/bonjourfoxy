Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");

function BFServiceTracker() {};

BFServiceTracker.prototype = {
    classDescription: "BFServiceTracker",
    classID:          Components.ID("2a0884a8-40d8-4e12-afd6-26530b2e47c2"),
    contractID:       "@bonjourfoxy.net/BFServiceTracker;1",
    _xpcom_categories: [
        {category: "xpcom-startup"},
        {category: "xpcom-shutdown"},
    ],
    QueryInterface: XPCOMUtils.generateQI([
        Components.interfaces.nsISupports,
        Components.interfaces.nsIObserver,
        Components.interfaces.nsISupportsWeakReference,
        Components.interfaces.IBFServiceTracker,
    ]),
    _observerService: null,
    _dnssdSvc: null,
    _consoleService: null,
    _alertsService: null,
    _prefs: null,
    _dnssdSvcEnum: null,
    _initCalled: false,
    _initTimer: null,
    _serviceCategories: [
        {label: "website", regtype: "_http._tcp"},
        {label: "wiki", regtype: "_http._tcp,_wiki"},
        {label: "printer", regtype: "_http._tcp,_printer"},
    ],
    _browseDomains: Object(),
    _services: Object(),
    _sortedServices: [],
    _allServices: [],
    callInContext: function(fn) {
        var context = this;
        return function() { fn.apply(context, arguments); }
    },
    consoleService: function()  {
        if (!this._consoleService)  {
            this._consoleService = Components.classes["@mozilla.org/consoleservice;1"]
                                   .getService(Components.interfaces.nsIConsoleService);
        }
        return this._consoleService;
    },
    observerService: function()  {
        if (!this._observerService) {
            this._observerService = Components.classes["@mozilla.org/observer-service;1"]
                                    .getService(Components.interfaces.nsIObserverService);
        }
        return this._observerService;
    },
    alertsService: function() {
        if (!this._alertsService)   {
            this._alertsService = Components.classes["@mozilla.org/alerts-service;1"]
                                  .getService(Components.interfaces.nsIAlertsService);
        }
        return this._alertsService;
    },
    prefs: function() {
        if (!this._prefs)    {
            this._prefs = Components.classes["@mozilla.org/preferences-service;1"]
                         .getService(Components.interfaces.nsIPrefService)
                         .getBranch("extensions.bonjourfoxy.");
        }
        return this._prefs;
    },
    dnssdSvc: function()   {
        if (!this._dnssdSvc) {
            this._dnssdSvc = Components.classes["@bonjourfoxy.net/BFDNSSDService;1"]
                            .createInstance(Components.interfaces.IBFDNSSDService);
        }
        return this._dnssdSvc;
    },
    getTmrInst: function()  {
        return Components.classes["@mozilla.org/timer;1"].createInstance(Components.interfaces.nsITimer);
    },
    log: function(text) {
        this.consoleService().logStringMessage(text);
    },
    _alertsPref: false,
    _updateAlertsPref: function(update)  {
        this._alertPref = this.prefs().getBoolPref("alerts");
    },
    _sendAlerts: function()  {
        if (!this._alertPref)   {
            this._updateAlertsPref();
        }
        return this._alertPref;
    },
    alert: function(title, body) {
        this.alertsService().showAlertNotification(null, title, body, null, null, null);
    },
    _newWritable: function() {
        return Components.classes["@mozilla.org/variant;1"]
               .createInstance(Components.interfaces.nsIWritableVariant);
    },
    _newArray: function() {
        return Components.classes["@mozilla.org/array;1"]
               .createInstance(Components.interfaces.nsIMutableArray);
    },
    observe: function(subject, topic, data) {
        switch(topic)   {
            case "xpcom-startup":
                // this is run very early, right after XPCOM is initialized,
                // but before user profile information is applied.
                this.observerService().addObserver(this, "toplevel-window-ready", true);
            break;
            case "toplevel-window-ready":
                if (!this._initCalled)  {
                    this._initCalled = true;
                    this.observerService().removeObserver(this, "toplevel-window-ready");
                    // still a little early - some UI weirdness is often
                    // manifested under X11 if the alert service is used.
                    // so come back in half a second...
                    this._initTimer = this.getTmrInst();
                    var tCallback = this.callInContext(function()    {
                        var dnssdSvc = this.dnssdSvc();
                        var eCallback = this.callInContext(this.eListener);
                        this._dnssdSvcEnum = dnssdSvc.enumerate(0, true, eCallback);
                        this._updateAlertsPref();
                    });
                    this._initTimer.initWithCallback({notify: tCallback}, 500, Components.interfaces.nsITimer.TYPE_ONE_SHOT);
                }
            break;
        }
    },
    getCategories: function() {
        var outCategories = this._newArray();
        for (i in this._serviceCategories) {
            var category = this._newWritable();
            category.setFromVariant(this._serviceCategories[i].label);
            outCategories.appendElement(category, 0);
        }
        return outCategories;
    },
    countServices: function(category) {
        if (category.toUpperCase() == "__ALL__")  {
            return this._allServices.length;
        } else {
            return this._sortedServices[category] ? this._sortedServices[category].length : 0;
        }
    },
    getServices: function(category) {
        var services = this._newArray();
        var returnArray = [];
        if (category.toUpperCase() == "__ALL__")  {
            returnArray = this._allServices;
        } else {
            if (this._sortedServices[category]) {
                returnArray = this._sortedServices[category];
            }
        }
        for (i in returnArray) {
            var service = this._newWritable();
            service.setFromVariant(returnArray[i].label);
            var domain = this._newWritable();
            domain.setFromVariant(returnArray[i].domain);
            var pair = this._newArray();
            pair.appendElement(service, 0);
            pair.appendElement(domain, 0);
            services.appendElement(pair, 0);
        }
        return services;
    },
    _organiseServices: function()    {
        var sortFn = function(a,b) { return a.label == b.label ? 0 : (a.label < b.label ? -1 : 1); }
        for (i in this._serviceCategories)   {
            var subtype = this._serviceCategories[i].label;
            this._sortedServices[subtype] = [];
        }
        this._allServices = [];
        for (item in this._services)    {
            for (domain in this._services[item]) {
                var serviceObj = {label: item, domain: domain};
                var subtypes = this._services[item][domain].subtypes;
                if (subtypes.__count__ == 1)    {
                    for (subtype in subtypes)  {
                        this._sortedServices[subtype].push(serviceObj);
                    }
                } else {
                    for (subtype in subtypes)  {
                        if (subtype != "Websites")  {
                            this._sortedServices[subtype].push(serviceObj);
                        }
                    }
                }
                this._allServices.push(serviceObj);
            }
        }
        for (i in this._serviceCategories)   {
            var subtype = this._serviceCategories[i].label;
            this._sortedServices[subtype].sort(sortFn);
        }
        this._allServices.sort(sortFn);
    },
    eListener: function(service, add, interfaceIndex, error, domainType, domain) {
        if (error) return;
        if (!this._browseDomains[domain])   {
            this._browseDomains[domain] = Object();
            this._browseDomains[domain].count = 0;
        }
        var domainObj = this._browseDomains[domain];
        add ? domainObj.count++ : domainObj.count--;
        if (domainObj.count == 1 && add) {
            for (i in this._serviceCategories) {
                var serviceObj = this._serviceCategories[i];
                var serviceID = serviceObj.label + serviceObj.regtype;
                var callback = this.callInContext(this.bListener);
                domainObj[serviceID] = this.dnssdSvc().browse(0, serviceObj.regtype, domain, callback);
            }
        }
        if (domainObj.count > 1)    {
            for (i in this._serviceCategories) {
                var serviceObj = this._serviceCategories[i];
                var serviceID = serviceObj.label + serviceObj.type;
                try { domainObj[serviceId].stop(); }
                catch (e) {};
            }
        }
    },
    bListener: function(service, add, interfaceIndex, error, serviceName, regtype, domain) {
        if (error) return;
        var stateHierarchy = [serviceName, domain, "subtypes"];
        var statePosition = this._services;
        var reorganise = false;
        var label = null;
        for (i=0; i<this._serviceCategories.length; i++)    {
            if (this._serviceCategories[i].regtype == regtype)  {
                label = this._serviceCategories[i].label;
                break;
            }
        }
        for (i=0; i<stateHierarchy.length; i++) {
            var property = stateHierarchy[i];
            if (!statePosition[property]) {
                statePosition[property] = Object();
            }
            statePosition = statePosition[property];
        }
        if (typeof(this._services[serviceName][domain].total) != "number") {
            this._services[serviceName][domain].total = 0;
        }
        if (typeof(this._services[serviceName][domain].subtypes[label]) != "number") {
            this._services[serviceName][domain].subtypes[label] = 0;
        }
        if (add) {
            this._services[serviceName][domain].total++;
            this._services[serviceName][domain].subtypes[label]++;
        } else {
            this._services[serviceName][domain].total--;
            this._services[serviceName][domain].subtypes[label]--;
        }
        if (this._services[serviceName][domain].subtypes[label] < 1) {
            delete this._services[serviceName][domain].subtypes[label];
            reorganise = true;
        }
        if (this._services[serviceName][domain].total < 1) {
            delete this._services[serviceName][domain];
            reorganise = true;
        }
        if (add && this._services[serviceName][domain].subtypes[label] == 1) {
            if (this._sendAlerts()) {
                this.alert('Service Discovered', serviceName);
            }
            reorganise = true;
        }
        if (!add && this._services[serviceName].__count__ == 0)  {
            delete this._services[serviceName];
            reorganise = true;
        }
        if (reorganise) {
            this._organiseServices();
            this._notifyChange(label);
        }
    },
    _notifyChange: function(category) {
        this.observerService().notifyObservers(null, "BFServiceTracker_Change", category);
    },
}

var components =[BFServiceTracker];

function NSGetModule(compMgr, fileSpec)  {
    return XPCOMUtils.generateModule(components);
}