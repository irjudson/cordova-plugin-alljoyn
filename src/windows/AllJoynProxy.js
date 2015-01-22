var AJ_CONNECT_TIMEOUT = 1000 * 5;
var AJ_BUS_START_FINDING = 0;
var AJ_BUS_STOP_FINDING = 1;

var busAttachment = new AllJoynWinRTComponent.AJ_BusAttachment();

cordova.commandProxy.add("AllJoyn", {
  connect: function(success, error) {
    var daemonName = "";
    var status = AllJoynWinRTComponent.AllJoyn.aj_FindBusAndConnect(busAttachment, daemonName, AJ_CONNECT_TIMEOUT);
    if (status == AllJoynWinRTComponent.AJ_Status.aj_OK) {
      messageHandler.start(busAttachment);
      success();
    } else {
      error(status);
    }
  },
  registerObjects: function(success, error, parameters) {
    // This function turns the JavaScript objects
    // into format that the AllJoyn Windows Runtime Component expects.
    // The complexity of the conversion comes from the fact that
    // we can't pass null JavaScript values to functions that expect
    // strings, but they have to be first converted into empty strings.
    var getAllJoynObjects = function(objects) {
      if (objects == null || objects.length == 0) return [null];
      var allJoynObjects = [];
      for (var i = 0; i < objects.length; i++) {
        if (objects[i] == null) {
          allJoynObjects.push(null);
          break;
        }
        var allJoynObject = new AllJoynWinRTComponent.AJ_Object();
        allJoynObject.path = objects[i].path;
        var interfaces = objects[i].interfaces;
        for (var j = 0; j < interfaces.length; j++) {
          var interface = interfaces[j];
          if (interface === null) break;
          var lastIndex = interface.length - 1;
          if (interface[lastIndex] === null) interface[lastIndex] = "";
        }
        allJoynObject.interfaces = interfaces;
        allJoynObjects.push(allJoynObject);
      }
      return allJoynObjects;
    }
    var applicationObjects = getAllJoynObjects(parameters[0]);
    var proxyObjects = getAllJoynObjects(parameters[1]);
    AllJoynWinRTComponent.AllJoyn.aj_RegisterObjects(applicationObjects, proxyObjects);
    success();
  },
  startFindingAdvertisedName: function(success, error, parameters) {
    var name = parameters[0];

    var foundAdvertisedNameMessageId = AllJoynWinRTComponent.AllJoyn.aj_Bus_Message_ID(1, 0, 1);
    messageHandler.addHandler(
      foundAdvertisedNameMessageId, 's',
      function(messageObject, messageBody) {
        console.log("Received message: ", messageObject, messageBody);
        messageHandler.removeHandler(foundAdvertisedNameMessageId, this[1]);
        if (messageBody[0] == AllJoynWinRTComponent.AJ_Status.aj_OK) {
          success(messageBody[1]);
        } else {
          error(messageBody[0])
        }
      }
    );

    var status = AllJoynWinRTComponent.AllJoyn.aj_BusFindAdvertisedName(busAttachment, name, AJ_BUS_START_FINDING);
    if (status != AllJoynWinRTComponent.AJ_Status.aj_OK) {
      error(status);
    }
  },
  joinSession: function(success, error, parameters) {
    var service = parameters[0];

    // Use null value as session options, which means that AllJoyn will use the default options
    var sessionOptions = null;
    var status = AllJoynWinRTComponent.AllJoyn.aj_BusJoinSession(busAttachment, service.name, service.port, sessionOptions);
    if (status == AllJoynWinRTComponent.AJ_Status.aj_OK) {
      var joinSessionReplyId = AllJoynWinRTComponent.AllJoyn.aj_Reply_ID(AllJoynWinRTComponent.AllJoyn.aj_Bus_Message_ID(1, 0, 10));
      messageHandler.addHandler(
        joinSessionReplyId, 'uu',
        function(messageObject, messageBody) {
          console.log("Received message: ", messageObject, messageBody);
          var sessionId = messageBody[2];
          var sessionHost = messageObject.sender;
          success([sessionId, sessionHost]);
          messageHandler.removeHandler(joinSessionReplyId, this[1]);
        }
      );
    } else {
      error(status);
    }
  }
});

// This initialization call is required once before doing
// other AllJoyn operations.
AllJoynWinRTComponent.AllJoyn.aj_Initialize();

/*
 * To use the handler, you must have first connected to an AllJoyn bus. The handler is then started
 * by passing a reference to your bus attachment.
 *
 * AllJoynMessageHandler.start(<your bus attachment>);
 *
 * To register handlers, you call the addHandler by passing the message ids you are interested,
 * the signature of your message and the callback function that gets called if such messages arrive.
 * The signature is an AllJoyn-specific value that is used to unmarshal the body of the message.
 *
 * AllJoynMessageHandler.addHandler(<interesting message id>,
 *                                  <return value signature>
 *                                  function(messageObject, messageBody) {
 *                                    // handle the received message
 *                                  });
 */
var messageHandler = (function() {
  // Use 0 as unmarshal timeout so that we don't end up blocking
  // the UI while waiting for new messages
  var AJ_UNMARSHAL_TIMEOUT = 0;
  var AJ_METHOD_TIMEOUT = 1000 * 1;
  var AJ_MESSAGE_SLOW_LOOP_INTERVAL = 500;
  var AJ_MESSAGE_FAST_LOOP_INTERVAL = 50;

  var messageHandler = {};
  var messageListeners = {};
  var interval = null;

  messageHandler.start = function(busAttachment) {
    // Flag to store current interval pace
    var runningFast;
    // This function can be called to update the interval based on if
    // the bus attachment had new messages or now. The idea is that if there are
    // messages, we run the loop faster to "flush the bus" and slower if there is
    // nothing new.
    var updateInterval = function(unmarshalStatus) {
      if (unmarshalStatus == AllJoynWinRTComponent.AJ_Status.aj_OK) {
        if (!runningFast) {
          clearInterval(interval);
          runningFast = true;
          interval = setInterval(handlerFunction, AJ_MESSAGE_FAST_LOOP_INTERVAL);
        }
      }
      if (unmarshalStatus == AllJoynWinRTComponent.AJ_Status.aj_ERR_TIMEOUT) {
        if (runningFast) {
          clearInterval(interval);
          runningFast = false;
          interval = setInterval(handlerFunction, AJ_MESSAGE_SLOW_LOOP_INTERVAL);
        }
      }
    }
    var handlerFunction = function() {
      var aj_message = new AllJoynWinRTComponent.AJ_Message();
      AllJoynWinRTComponent.AllJoyn.aj_UnmarshalMsg(busAttachment, aj_message, AJ_UNMARSHAL_TIMEOUT).done(function(status) {
        if (status == AllJoynWinRTComponent.AJ_Status.aj_OK) {
          var messageObject = aj_message.get();
          var receivedMessageId = messageObject.msgId;
          // Check if we have listeners for this message id
          if (messageListeners[receivedMessageId]) {
            // Pass the value to listeners
            var callbacks = messageListeners[receivedMessageId];
            for (var i = 0; i < callbacks.length; i++) {
              // Unmarshal the message body
              var messageBody = AllJoynWinRTComponent.AllJoyn.aj_UnmarshalArgs(aj_message, callbacks[i][0]);
              callbacks[i][1](messageObject, messageBody);
            }
          }
        }
        AllJoynWinRTComponent.AllJoyn.aj_CloseMsg(aj_message);
        updateInterval(status);
      });
    }
    // Initially start with slower interval
    runningFast = false;
    interval = setInterval(handlerFunction, AJ_MESSAGE_SLOW_LOOP_INTERVAL);
  }

  messageHandler.stop = function() {
    clearInterval(interval);
  }

  messageHandler.addHandler = function(messageId, signature, callback) {
    // Create a list of handlers for this message id if it doesn't exist yet
    if (typeof messageListeners[messageId] != "object") {
      messageListeners[messageId] = [];
    }
    messageListeners[messageId].push([signature, callback]);
  }

  messageHandler.removeHandler = function(messageId, callback) {
    messageListeners[messageId] = messageListeners[messageId].filter(
      function(element) {
        // Filter out the given callback function
        return (element[1] !== callback);
      }
    );
  }

  return messageHandler;
})();
