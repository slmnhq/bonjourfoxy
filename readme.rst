BonjourFoxy
===========

This repo should contain everything needed to build a BonjourFoxy release from scratch. If you redistribute BonjourFoxy please change all the identifiers in the to be unique. That includes but is no limited to the names and UUID's used by the binary components. If you do not want to rebuild the binary components then specify BonjourFoxy as a dependency in your extension (see 'Extension Dependencies' on the mozilla wiki for details). 

Folder Structure
----------------

 - ``ext`` - Chrome skeleton (component, content, etc)
 - ``src`` - XPCOM component source code
 - ``src/#.#`` - Component intermediary/output folder
 - ``scratch`` - Staging folder
 - ``sdk/archive`` - XULRunner SDK archive
 - ``sdk/#.#`` - The active XULRunner SDK for a given release

Obtaining the XULRunner SDK
---------------------------

You can populate ``sdk/archive`` using the ``get-xr.sh`` shell script which takes a XULRunner release as it sole argument. You will likely want to alter the script to point at your local Mozilla mirror unless you have good connectivity to Internode (in Australia). Example use:

 - ./get-xr.sh 1.9.0.15
 - ./get-xr.sh 1.9.1.4
 - ./get-xr.sh 1.9.2b1

``switch-xr.sh`` is used to activate an SDK for use with a given Firefox release. It takes a Firefox release and a XULRunner SDK release as it's arguments. Example use:

 - ./switch-xr.sh 3.0 1.9.0.15
 - ./switch-xr.sh 3.5 1.9.1.4
 - ./switch-xr.sh 3.6 1.9.2b1

Prerequisites on OS X
---------------------

If you've installed the 'Developer Tools' and the 10.5 SDK (you can build against 10.6 only if you want, just update ``src/darwin.inc``) you should be right to go.

Prerequisites on Windows
------------------------

 - Visual Studio (I've used 2008, I assume it'd work with other releases)
 - Cygwin install including Perl and GNU Make
 - Bonjour SDK (in default directory, otherwise you'll need to update ``src/cygwin.inc``)

Building the Extension
----------------------

Make targets:

 - ``xpcom`` - build components for the current platform
 - ``dir`` - copies the extension skeleton and any built XPCOM components into the ``scratch`` folder and updates the install.rdf
 - ``xpi`` - creates an XPI from the contents of the ``scratch`` directory
 - ``clean`` - clean up

Variables:
 - ``FF_RELS`` - the releases to build against
 - ``BF_REL`` - the BonjourFoxy release being built
 - ``MIN_VER`` - the minimum supported Firefox release (used to update install.rdf)
 - ``MAX_VER`` - the maximum supported Firefox release (used to update install.rdf)

Testing the Extension
---------------------

Create a file called 'bonjourfoxy@bonjourfoxy.net' in your Firefox Profile's extension folder that contains the path to the ``scratch`` directory, eg: ``/Users/atj/Projects/bonjourfoxy/scratch`` or ``B:\scratch``.

What about Linux/BSD?
---------------------

BonjourFoxy can be built against Avahi's Bonjour compatibility layer though at present no build script is included. If you'd like to package BonjourFoxy for a particular OS feel free to contact me.
