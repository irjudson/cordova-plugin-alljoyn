var argscheck = require('cordova/argscheck'),
	utils = require('cordova/utils'),
	exec = require('cordova/exec'),
	cordova = require('cordova');

var registeredObjects = [];

var getSignature = function(indexList, objectsList) {
  var objects = objectsList[indexList[0]];
  var object = objects[indexList[1]];
  var interfaces = object.interfaces;
  var signature = interfaces[indexList[2]][indexList[3] + 1]
  return signature;
}

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
	  var successCallback = function(result) {
	    var session = {
	      sessionId: result[0],
	      sessionHost: result[1],
	      callMethod: function(callMethodSuccess, callMethodError, path, indexList, parameterType, parameters) {
	        var signature = getSignature(indexList, registeredObjects);
	        exec(callMethodSuccess, callMethodError, "AllJoyn", "invokeMember", [signature, path, indexList, parameterType, parameters]);
	      },
	      sendSignal: function(sendSignalSuccess, sendSignalError, path, indexList, parameterType, parameters) {
	        var signature = getSignature(indexList, registeredObjects);
	        exec(sendSignalSuccess, sendSignalError, "AllJoyn", "invokeMember", [signature, path, indexList, parameterType, parameters]);
	      }
	    };
	    success(session);
	  };

	  exec(successCallback, error, "AllJoyn", "joinSession", [service]);
	},
	registerObjects: function(success, error, applicationObjects, proxyObjects) {
	  exec(function() {
	    registeredObjects = [applicationObjects, proxyObjects];
	    success();
	  }, error, "AllJoyn", "registerObjects", [applicationObjects, proxyObjects]);
	},
	AJ_OK: 0
};

module.exports = AllJoyn;