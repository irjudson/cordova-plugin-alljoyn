var argscheck = require('cordova/argscheck'),
	utils = require('cordova/utils'),
	exec = require('cordova/exec'),
	cordova = require('cordova');

var alljoyn = {
	initialize: function(success, error, serviceName) {
		argscheck.checkArgs('FF', 'allseen.alljoyn.initialize', arguments);
		exec(success, error, "AllJoyn", "initialize", [serviceName]);
	}
};

module.exports = alljoyn;
