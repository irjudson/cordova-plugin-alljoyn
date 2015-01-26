package org.allseen.alljoyn;

import alljoyn.*;

import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.os.Looper;
import android.os.Message;

import org.apache.cordova.CallbackContext;
import org.apache.cordova.CordovaPlugin;
import org.apache.cordova.CordovaInterface;
import org.apache.cordova.CordovaWebView;

import org.json.JSONArray;
import org.json.JSONException;

public class AllJoyn extends CordovaPlugin {
	/* Load the native alljoyn library. */
	static {
		System.loadLibrary("alljoyn");
	}

	private static final String TAG = "AllJoyn";
	private static final short  CONTACT_PORT=42;
    private static final String DAEMON_AUTH = "ALLJOYN_PIN_KEYX";
    private static final String DAEMON_PWD = "1234"; // 000000 or 1234

    // Hard coded bits that need to be dynamic later
    AJ_Object[] appObjects;

	/**
	 * Sets the context of the Command. This can then be used to do things like
	 * get file paths associated with the Activity.
	 *
	 * @param cordova The context of the main Activity.
	 * @param webView The CordovaWebView Cordova is running in.
	 */
	@Override
	public void initialize(final CordovaInterface cordova, CordovaWebView webView) {
		super.initialize(cordova, webView);
		Log.i(TAG, "Initialization running.");		
		alljoyn.AJ_Initialize();
		Log.i(TAG, "Initialization completed.");
	}

	/**
	 * Executes the application level initialization.
	 * 
	 * @param serviceName The name of the service to connect to.
	 * @return AJ_Status string
	**/
	private AJ_Status init(String serviceName)
	{
		AJ_Status status = AJ_Status.AJ_OK;

		SWIGTYPE_p_p_char interfaceDescription = alljoyn.AJ_InterfaceDescriptionCreate("org.alljoyn.bus.samples.chat");
		interfaceDescription = alljoyn.AJ_InterfaceDescriptionAdd(interfaceDescription, "!Chat str>s");
		SWIGTYPE_p_p_p_char chatInterfaces = alljoyn.AJ_InterfacesCreate();
		chatInterfaces = alljoyn.AJ_InterfacesAdd(chatInterfaces, interfaceDescription);
		AJ_Object chatObject = alljoyn.AJ_Object();
		chatObject.setPath("/chatService");
		chatObject.setInterfaces(chatInterfaces);
		
		return status;
	}

	/**
	 * Executes the request and returns PluginResult.
	 *
	 * @param action            The action to execute.
	 * @param args              JSONArray of arguments for the plugin.
	 * @param callbackContext   The callback context used when calling back into JavaScript.
	 * @return                  True when the action was valid, false otherwise.
	 */    
	@Override
	public boolean execute(String action, JSONArray data, CallbackContext callbackContext) throws JSONException {

		if (action.equals("initialize")) {
        	String serviceName = data.getString(0);
			Log.i(TAG, "AllJoyn.Initialize("+serviceName+") called.");
			AJ_Status status = init(serviceName);
			if (status == AJ_Status.AJ_OK) {
				callbackContext.success("Initialized AllJoyn.");
				return true;
			} else {
				callbackContext.error("Error Initializing AllJoyn: " + status.toString());
				return false;
			}
		}
		if (action.equals("discover")) {
			Log.i(TAG, "Calling discover");
			// Status status = mBus.findAdvertisedName("org.alljoyn.BusNode.*");
			// if (status == Status.OK) {
			// 	callbackContext.success("Find Devices Success.");
			// 	return true;
			// } else {
			// 	callbackContext.error("Find Devices Error: " + status.getErrorCode());
			// 	return false;
			// }
			    
		}
		return false;
	}
}
