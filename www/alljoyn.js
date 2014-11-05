var exec = require('cordova/exec');

module.exports = {

    hello: function(callback, name) {
        exec(callback, null, "AllJoyn", "hello", [name]);
    }

};
