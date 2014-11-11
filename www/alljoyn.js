var exec = require('cordova/exec');

module.exports = {

    hello: function(callback, name) {
        exec(callback, null, "AllJoyn", "hello", [name]);
    },

    discover: function(success, error) {
		argscheck.checkArgs('FF', 'allseen.alljoyn.discover', arguments);
			exec(success, error, "AllJoyn", "discover", []);
	}

};
