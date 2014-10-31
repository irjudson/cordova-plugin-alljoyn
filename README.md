cordova-plugin-alljoyn
======================

Cordova plugin for [AllJoyn](https://allseenalliance.org/alljoyn-framework-tutorial).

Implementation Notes
--------------------
Here are my current implementation plans:
* Integrating with AllJoyn Core SDK 14.06.00a
* Add Windows native support (x64) by building WinRT component port of the necessary libraries 
* Requires Windows SDK 8.0

Caveats
-------
Work is still in progress. Requires Cordova < 3.6 (Version 3.6 added Windows Universal App Support which breaks some project referencing. [Bug Filing](https://issues.apache.org/jira/browse/CB-7911)).
