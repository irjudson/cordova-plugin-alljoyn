var argscheck = require('cordova/argscheck'),
    utils = require('cordova/utils'),
    exec = require('cordova/exec'),
    cordova = require('cordova');

function AllJoyn() {
    AllJoyn.prototype.hello = function(success, error, name) {
    	argscheck.checkArgs('FFS', 'allseen.alljoyn.hello', arguments);
        exec(success, error, "AllJoyn", "hello", [name]);
    }
}

modele.exports = new AllJoyn();