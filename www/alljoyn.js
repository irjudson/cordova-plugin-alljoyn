var argscheck = require('cordova/argscheck'),
	utils = require('cordova/utils'),
	exec = require('cordova/exec'),
	cordova = require('cordova');

var alljoyn = {
	version: function(success, error) {
		argscheck.checkArgs('', 'allseen.alljoyn.version', arguments);
		exec(success, error, "AllJoyn", "version", []);
	},
	discover: function(success, error) {
		argscheck.checkArgs('FF', 'allseen.alljoyn.discover', arguments);
		exec(success, error, "AllJoyn", "discover", []);
	}
};

module.exports = alljoyn;
