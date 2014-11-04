var exec = require('cordova/exec');

var alljoyn = {
    hello: function(name) {
        exec(null, null, "AllJoyn", "hello", [name]);
    }
};

module.exports = alljoyn;
