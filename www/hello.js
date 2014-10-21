/*global cordova*/
cordova.define("cordova/plugin/hello",
    function (require, exports, module) {

        var exec = cordova.require('cordova/exec');

        function greet(name, successCallback, errorCallback) {
            exec(successCallback, errorCallback, "Hello", "greet", [name]);
        }

        module.exports = {
            greet: greet
        }
    }
);
