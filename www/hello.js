var exec = require('cordova/exec');

var alljoyn = {
    hello: function(name, successCallback, errorCallback) {
        exec(successCallback, errorCallback, "Hello", "greet", [name]);
    }
};

module.exports = alljoyn;
