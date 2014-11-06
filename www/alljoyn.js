var argscheck = require('cordova/argscheck'),
	utils = require('cordova/utils'),
	exec = require('cordova/exec'),
	cordova = require('cordova');

var alljoyn = {
	hello: function(name, success, error) {
		argscheck.checkArgs('SFF', 'allseen.alljoyn.hello', arguments);
		exec(success, error, "AllJoyn", "hello", [name]);
	},
	start: function(success, error) {
		// argscheck.checkArgs('FF', 'allseen.alljoyn.start', arguments);
		exec(success, error, "AllJoyn", "start");
	}
};

module.exports = alljoyn;
