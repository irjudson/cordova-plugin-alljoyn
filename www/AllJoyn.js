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
	    var sessionId = result[0];
	    var sessionHost = result[1];
	    var session = {
	      sessionId: sessionId,
	      sessionHost: sessionHost,
	      callMethod: function(callMethodSuccess, callMethodError, destination, path, indexList, parameterType, parameters) {
	        var signature = getSignature(indexList, registeredObjects);
	        exec(callMethodSuccess, callMethodError, "AllJoyn", "invokeMember", [sessionId, destination, signature, path, indexList, parameterType, parameters]);
	      },
	      sendSignal: function(sendSignalSuccess, sendSignalError, destination, path, indexList, parameterType, parameters) {
	        var signature = getSignature(indexList, registeredObjects);
	        exec(sendSignalSuccess, sendSignalError, "AllJoyn", "invokeMember", [sessionId, destination, signature, path, indexList, parameterType, parameters]);
	      }
	    };
	    success(session);
	  };

	  exec(successCallback, error, "AllJoyn", "joinSession", [service]);
	},
	registerObjects: function(success, error, applicationObjects, proxyObjects) {
	  exec(function() {
	    registeredObjects = [null, applicationObjects, proxyObjects];
	    success();
	  }, error, "AllJoyn", "registerObjects", [applicationObjects, proxyObjects]);
	},
	AJ_OK: 0
};

module.exports = AllJoyn;