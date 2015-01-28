package org.allseen.alljoyn.plugin;

import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.net.wifi.WifiManager;
import android.util.Log;

import org.apache.cordova.CallbackContext;
import org.apache.cordova.CordovaPlugin;
import org.apache.cordova.CordovaInterface;
import org.apache.cordova.CordovaWebView;

import org.json.JSONArray;
import org.json.JSONException;

import org.allseen.alljoyn.alljoyn;
import org.allseen.alljoyn.AJ_BusAttachment;
import org.allseen.alljoyn.AJ_Status;

public class AllJoyn extends CordovaPlugin {
	private static final String TAG = "AllJoyn";

	/* Load the native alljoyn_java library. */
	static {
		Log.i(TAG, "Loading AllJoyn Thin Library.");
		System.loadLibrary("alljoyn");
	}

	// Native methods
	public native String getVersion();
	public native void initialize();

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
		AJ_Status status = null;
		AJ_BusAttachment bus = new AJ_BusAttachment();
		long timeout = 5000L;

		Log.i(TAG, "Initialization running.");

		alljoyn.AJ_Initialize();

    Log.i(TAG, "Registering Objects");    
    alljoyn.AJ_RegisterObjects(null, null);

    Log.i(TAG, "Setting Bus Auth Password Callback");    
    // alljoyn.SetBusAuthPwdCallback(MyBusAuthPwdCB);
    alljoyn.SetBusAuthPwdCallback(null);

    Log.i(TAG, "Calling Find Bus and Connect");    
    status = alljoyn.AJ_FindBusAndConnect(bus, "org.alljoyn.BusNode", timeout);
    if (status != null) {
    	Log.i(TAG, "Discover Find Bus and Connect: " + status.toString());
    }

		// Log.i(TAG, "Setting Authentication.");
		// Status status = PasswordManager.setCredentials(DAEMON_AUTH, DAEMON_PWD);
  //       if (status == Status.OK) {
  //       	Log.i(TAG, "AUTH set successfully.");
  //       } else {
  //       	Log.i(TAG, "AUTH set failed: " + status.getErrorCode());
  //       }

		// Log.i(TAG, "Creating BusAttachment.");
		// mBus = new BusAttachment(getClass().getName(), BusAttachment.RemoteMessage.Receive);
		
		// Log.i(TAG, "Registering mBusBusListener.");
		// mBus.registerBusListener(new BusListener() {
		// 	@Override
		// 	public void foundAdvertisedName(String name, short transport, String namePrefix) {
		// 		mBus.enableConcurrentCallbacks();
		// 		Log.i(TAG, "Service Found: " + name + " " + namePrefix);
		// 		short contactPort = CONTACT_PORT;
		// 		SessionOpts sessionOpts = new SessionOpts();
		// 		Mutable.IntegerValue sessionId = new Mutable.IntegerValue();
		// 		Status status = mBus.joinSession(name, contactPort, sessionId, sessionOpts, new SessionListener());
		// 	}
		// });
		
		// Log.i(TAG, "Connecting to mBus.");
		// status = mBus.connect();
		// if (status == Status.OK) {
		// 	Log.i(TAG, "mBus Connect Success.");
		// } else {
		// 	Log.i(TAG, "mBus Connect Error: " + status.getErrorCode());
		// }

		// Log.i(TAG, "Finding Router Daemon.");
		// status = mBus.findAdvertisedName("org.alljoyn.BusNode");
		// if (status == Status.OK) {
		// 	Log.i(TAG, "Find Router Daemon Success.");
		// } else {
		// 	Log.i(TAG, "Find Router Daemon Error: " + status.getErrorCode());
		// }

		Log.i(TAG, "Initialization completed.");
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
		} else if (action.equals("version")) {
			Log.i(TAG, "Calling version");
			// String version = null;
			String version = alljoyn.AJ_GetVersion();
			if (version == null) {
				callbackContext.error("Error retrieving version.");
				return false;
			} else {
				callbackContext.success(version);
				return true;
			}
		}
		return false;
	}
}
