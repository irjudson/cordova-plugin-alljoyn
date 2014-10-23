var exec = require('cordova/exec');

var alljoyn = {
    hello: function(name, successCallback, errorCallback) {
        exec(successCallback, errorCallback, "AllJoyn", "hello", [name]);
    }
};

module.exports = alljoyn;
