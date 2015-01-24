var argscheck = require('cordova/argscheck'),
	utils = require('cordova/utils'),
	exec = require('cordova/exec'),
	cordova = require('cordova');

var alljoyn = {
	// From my previous code -- IRJ
	version: function(success, error) {
		argscheck.checkArgs('', 'allseen.alljoyn.version', arguments);
		exec(success, error, "AllJoyn", "version", []);
	},
	// discover: function(success, error) {
	// 	argscheck.checkArgs('FF', 'allseen.alljoyn.discover', arguments);
	// 	exec(success, error, "AllJoyn", "discover", []);
	// From the combined implementation
	initialize: function(success, error, serviceName) {
		argscheck.checkArgs('FF', 'allseen.alljoyn.initialize', arguments);
		exec(success, error, "AllJoyn", "initialize", [serviceName]);
	}
};

module.exports = alljoyn;
