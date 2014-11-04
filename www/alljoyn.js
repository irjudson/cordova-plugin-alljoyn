var exec = require('cordova/exec');

var alljoyn = {
    hello: function(callback, name) {
        exec(callback, callback, "AllJoyn", "hello", [name]);
    }
};

module.exports = alljoyn;
