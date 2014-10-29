var exec = require('cordova/exec');

module.exports = {

    busAttachment: function(applicationName, allowRemoteMessages, concurrency, successCallback, errorCallback) {
    	AllJoynWinRTComponent.BusAttachment(applicationName, allowRemoteMessages, concurrency);
    	successCallback();
    }
    
};
