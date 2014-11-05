var exec = require('cordova/exec');

module.exports = {

    hello: function(success, error, name) {
        exec(success, error, "AllJoyn", "hello", [name]);
    }

};
