var argscheck = require('cordova/argscheck'),
	utils = require('cordova/utils'),
	exec = require('cordova/exec'),
	cordova = require('cordova');

var AllJoyn = {
	connect: function(success, error) {
	  exec(success, error, "AllJoyn", "connect");
	},
	startFindingAdvertisedName: function(success, error, name) {
	  exec(success, error, "AllJoyn", "startFindingAdvertisedName", [name]);
	},
	startFindingInterfaces: function(success, error, names) {
	  exec(success, error, "AllJoyn", "startFindingInterfaces", [names]);
	},
  /*
        var service = {
          name: "name.of.the.service",
          port: 12
        };
   */
	joinSession: function(success, error, service) {
	  var successCallback = function(sessionId, sessionTarget) {
	    var session = {
	      sessionId: sessionId,
	      sessionTarget: sessionTarget,
	      callMethod: function(callMethodSuccess, callMethodError, path, indexList, parameters) {
	        exec(callMethodSuccess, callMethodError, "AllJoyn", "invokeMember", [path, indexList, parameters]);
	      },
	      sendSignal: function(sendSignalSuccess, sendSignalError, path, indexList, parameters) {
	        exec(sendSignalSuccess, sendSignalError, "AllJoyn", "invokeMember", [path, indexList, parameters]);
	      }
	    };
	    success(session);
	  };

	  exec(successCallback, error, "AllJoyn", "joinSession", [service]);
	},
	registerObjects: function(success, error, applicationObjects, proxyObjects) {
	  exec(success, error, "AllJoyn", "registerObjects", [applicationObjects, proxyObjects]);
	},
	AJ_OK: 0
};

module.exports = AllJoyn;