bonjourfoxy.list = {
    initialize: function()  {
        bonjourfoxy.lib.addUserPrefsObserver(bonjourfoxy.list);
        bonjourfoxy.lib.observerService().addObserver(bonjourfoxy.list, "BFServiceTracker_Change", false);
        document.getElementById("serviceListChildren").addEventListener("click", bonjourfoxy.list.listEventHandler, false);
        document.getElementById("targetmenu").addEventListener("command", bonjourfoxy.list.listEventHandler, false);
        bonjourfoxy.list.treeView.init();
        bonjourfoxy.list.displaySidebarsearch();
    },
    shutdown: function () {
        bonjourfoxy.lib.rmvUserPrefsObserver(bonjourfoxy.list);
        bonjourfoxy.lib.observerService().removeObserver(bonjourfoxy.list, "BFServiceTracker_Change");
        bonjourfoxy.list.persistContainers();
    },
    observe: function(subject, topic, data) {
         if (topic == "BFServiceTracker_Change")    {
             // subtypes potentially add/remove services from "Websites"
            if (data != "Websites") {
                this.treeView.update("Websites");
            }
            this.treeView.update(data);
         }
        if (topic == "nsPref:changed")  {
            if (data == "sidebarsearch") {
                bonjourfoxy.list.displaySidebarsearch();
            }
        }
    },
    persistContainers: function()   {
        var opensbc = [];
        for (i=0; i<bonjourfoxy.list.treeView.data.length; i++)  {
            if (bonjourfoxy.list.treeView.data[i].isOpen)    {
                opensbc.push(bonjourfoxy.list.treeView.data[i].label);
            }
        }
        bonjourfoxy.lib.userPrefs().setCharPref("sidebarcontainers", opensbc.join("|"));
    },
    displaySidebarsearch: function()    {
        var searchbox = document.getElementById("search-box");
        if (bonjourfoxy.lib.userPrefs().getBoolPref("sidebarsearch"))   {
            searchbox.style.display = "";
        } else {
            searchbox.style.display = "none";
            searchbox.value = "";
            bonjourfoxy.list.search();
        }
    },
    listEventHandler: function(event) {
        if (event.type=='click' && event.button == 2)   { return; } // context menu click
        var selected = document.getElementById('serviceList').currentIndex;
        if (selected==-1)   { return; }
        var service = bonjourfoxy.list.treeView.data[selected];
        if (service.isContainer)  { return; }
        var linkTarget = "default";
        if (event.type == 'command')  {
            linkTarget=event.target.value;
        } else {
            if (event.shiftKey) {
                linkTarget="window";
            }
            if (event.button=="1") {
                linkTarget="tab";
            }
        }
        var b = resolverContext = Object();
        resolverContext.resolved = false;
        resolverContext.label = service.label;
        resolverContext.target = linkTarget;
        resolverContext.resolver = bonjourfoxy.lib.DNSSDService().resolve(0, service.name, "_http._tcp.", service.domain, "path", function(service, interfaceIndex, error, fqdn, hostname, port, key, value) {
            var rc = resolverContext;
            if(!rc.resolved) {
                hostname = hostname.charAt(hostname.length-1) == "." ? hostname.substr(0, hostname.length-1) : hostname;
                var path = value.charAt(0) == '/' ? value : '/' + value;
                bonjourfoxy.lib.openLink(["http://", hostname, ':', port, path].join(""), rc.target);
            }
            rc.resolved = true;
            rc.resolver = null;
            service.stop();
        });
        window.setTimeout(function() {
            var rc = resolverContext;
            if (!rc.resolved)    {
                bonjourfoxy.lib.dialog("Timeout", "Timed out trying to resolve " + rc.label);
            }
            try { rc.resolver.stop(); }
            catch (e) {}
        }, 15000);
    },
    search: function() {
        categories = bonjourfoxy.lib.ServiceTracker().getCategories();
        for (i=0; i<categories.length; i++)    {
            var category = categories.queryElementAt(i, Components.interfaces.nsIVariant);
            bonjourfoxy.list.treeView.update(category);
        }
    },
    treeView: {
        getServices: function(category) {
            var services = [];
            var rawServices = bonjourfoxy.lib.ServiceTracker().getServices(category);
            var filter = document.getElementById("search-box").value;
            for (i = 0; i < rawServices.length; i++)   {
                rawService = rawServices.queryElementAt(i, Components.interfaces.nsIArray);
                var name = rawService.queryElementAt(0, Components.interfaces.nsIVariant);
                var domain = rawService.queryElementAt(1, Components.interfaces.nsIVariant);
                var label = name + " (" + domain + ")";
                if (label.indexOf(filter) !=-1)   {
                    services.push({
                        isContainer: false,
                        isOpen: false,
                        label: label,
                        name: name,
                        domain: domain,
                    });
                }
            }
            return services;
        },
        emptyContainer: function (idx) {
            if (!this.isContainer(idx))  {
                throw "Item " + idx + " is not a container";
            }
            var thisLevel = this.getLevel(idx);
            var deletecount = 0;
            for (var i = idx + 1; i < this.data.length; i++)    {
                if (this.getLevel(i) > thisLevel) {
                    deletecount++;
                } else {
                    break;
                }
            }
            if (deletecount) {
                this.data.splice(idx + 1, deletecount);
                this.treeBox.rowCountChanged(idx + 1, -deletecount);
            }
        },
        populateContainer: function(idx) {
            if (!this.isContainer(idx))  {
                throw "Item " + idx + " is not a container";
            }
            var category = this.data[idx].label;
            var toinsert = this.getServices(category);
            for (i=0; i<toinsert.length; i++)   {
                this.data.splice(idx + i + 1, 0, toinsert[i]);
            }
            this.treeBox.rowCountChanged(idx + 1, toinsert.length);
        },
        init: function()    {
            categories = bonjourfoxy.lib.ServiceTracker().getCategories();
            for (i=0; i < categories.length; i++)    {
                this.data.push({
                    label: categories.queryElementAt(i, Components.interfaces.nsIVariant),
                    isContainer: true,
                });
                document.getElementById("serviceList").view = bonjourfoxy.list.treeView;
            }
            var opensbcO = {};
            var opensbcA = bonjourfoxy.lib.userPrefs().getCharPref("sidebarcontainers").split("|");
            if (typeof(opensbcA)==typeof("")) { opensbcA = [opensbcA]; }
            for (i=0; i<opensbcA.length; i++)   {
                opensbcO[opensbcA[i]] = true;
            }
            for (i=0; i < this.data.length; i++)    {
                if (!this.data[i].isContainer) { continue; }
                if (opensbcO[this.data[i].label])   {
                    this.toggleOpenState(i);
                }
            }
        },
        update: function(category) {
            if (category == null) { return; }
            // find the idx
            var categoryIdx = -1;
            for (i=0; i<this.data.length; i++)  {
                var item = this.data[i];
                if (item.isContainer && item.label == category) {
                    if (!item.isOpen)   { return; }
                    categoryIdx = i;
                    break;
                }
            }
            if (categoryIdx == -1) { throw "Could not find idx of category " + category; }
            var firstVisibleRow = this.treeBox.getFirstVisibleRow();
            this.treeBox.beginUpdateBatch();
            this.emptyContainer(categoryIdx);
            this.populateContainer(categoryIdx);
            this.treeBox.invalidateRow(categoryIdx);
            this.treeBox.endUpdateBatch();
            this.treeBox.ensureRowIsVisible(this.data.length > firstVisibleRow ? firstVisibleRow : this.data.length);
        },
        data : [],
        treeBox: null,
        selection: null,
        get rowCount()                     { return this.data.length; },
        setTree: function(treeBox)         { this.treeBox = treeBox; },
        getCellText: function(idx, column) { return this.data[idx].label; },
        isContainer: function(idx)         { return this.data[idx].isContainer; },
        isContainerOpen: function(idx)     { return this.data[idx].isOpen; },
        isContainerEmpty: function(idx)    { return bonjourfoxy.lib.ServiceTracker().countServices(this.data[idx].label) == 0; },
        isSeparator: function(idx)         { return false; },
        isSorted: function()               { return false; },
        isEditable: function(idx, column)  { return false; },
        
        getParentIndex: function(idx) {
            if (this.isContainer(idx)) return -1;
            for (var t = idx - 1; t >= 0 ; t--) {
              if (this.isContainer(t)) return t;
            }
        },
        getLevel: function(idx) {
            return this.isContainer(idx) ? 0:1;
        },
        hasNextSibling: function(idx, after) {
            var thisLevel = this.getLevel(idx);
            for (var t = after + 1; t < this.data.length; t++) {
              var nextLevel = this.getLevel(t);
              if (nextLevel == thisLevel) return true;
              if (nextLevel < thisLevel) break;
            }
            return false;
        },
        toggleOpenState: function(idx) {
            if (!this.isContainer(idx)) return;
            if (this.isContainerOpen(idx))  {
                this.data[idx].isOpen = false;
                this.emptyContainer(idx);
            } else {
                this.data[idx].isOpen = true;
                this.populateContainer(idx);
            }
            this.treeBox.invalidateRow(idx);
            bonjourfoxy.list.persistContainers();
        },
        getImageSrc: function(idx, column) {},
        getProgressMode : function(idx,column) {},
        getCellValue: function(idx, column) {
        },
        cycleHeader: function(col, elem) {},
        selectionChanged: function() {},
        cycleCell: function(idx, column) {},
        performAction: function(action) {},
        performActionOnCell: function(action, index, column) {},
        getRowProperties: function(idx, column, prop) {},
        getCellProperties: function(idx, column, prop) {},
        getColumnProperties: function(column, element, prop) {},
    }
};
window.addEventListener("load", bonjourfoxy.list.initialize, false);
window.addEventListener("unload", bonjourfoxy.list.shutdown, false);