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
    var successCallback = function() {
      var bus = {
        addListener: function(indexList, responseType, listener) {
          var signature = getSignature(indexList, registeredObjects);
          // We are passing the listener function to the exec call as its success callback, but in this case,
          // it is expected that the callback can be called multiple times. The error callback is passed just because
          // exec requires it, but it is not used for anything.
          // The listener is also passed as a parameter, because in the Windows implementation, the success callback
          // can't be called multiple times.
          exec(listener, function() { }, "AllJoyn", "addListener", [indexList, responseType, listener]);
        }
      };
      success(bus);
    }
    exec(successCallback, error, "AllJoyn", "connect");
	},
  /*
   * When name found, success callback is called with parameter { name: "the.name.found" }
   */
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
	      callMethod: function(callMethodSuccess, callMethodError, destination, path, indexList, inParameterType, parameters, outParameterType) {
	        var signature = getSignature(indexList, registeredObjects);
	        exec(callMethodSuccess, callMethodError, "AllJoyn", "invokeMember", [sessionId, destination, signature, path, indexList, inParameterType, parameters, outParameterType]);
	      },
	      sendSignal: function(sendSignalSuccess, sendSignalError, destination, path, indexList, inParameterType, parameters) {
	        var signature = getSignature(indexList, registeredObjects);
	        exec(sendSignalSuccess, sendSignalError, "AllJoyn", "invokeMember", [sessionId, destination, signature, path, indexList, inParameterType, parameters]);
	      },
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