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
    private AJ_BusAttachment bus;

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
		bus = new AJ_BusAttachment();
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
		if (action.equals("connect")) {
			String serviceName = data.getString(0);
			if (serviceName.length() == 0) {
				serviceName = null;
			}
			long timeout = data.getLong(1);
			AJ_Status status = null;
			Log.i(TAG, "AllJoyn.connect("+bus+","+serviceName+","+timeout+")");
			try {
				status = alljoyn.AJ_FindBusAndConnect(bus, serviceName, timeout);					
			} catch (Exception e) {
				Log.i(TAG, "Exception finding and connecting to bus: " + e.toString());
			}
			Log.i(TAG, "Called AJ_FindBusAndConnect, status = " + status);
			if( status == AJ_Status.AJ_OK) {
				callbackContext.success("Connected to router!");
				return true;
			} else {
				callbackContext.error("Error connecting to router: " + status.toString());
				return false;
			}
		}
		if (action.equals("registerObjects")) {
			Log.i(TAG, "AllJoyn.registerObjects");

			AJ_Status status;
			JSONArray localObjects = data.getJSONArray(0);
			JSONArray remoteObjects = data.getJSONArray(1);
			AJ_Object local = null;
			AJ_Object remote = null;

			Log.i(TAG, "AllJoyn.registerObjects("+localObjects+","+remoteObjects+")");

			alljoyn.AJ_RegisterObjects(local, remote);

			Log.i(TAG, "AllJoyn.registerObjects succeeded.");

			callbackContext.success("Registered objects!");
			return true;
		}
		if (action.equals("joinSession")) {
			Log.i(TAG, "AllJoyn.joinSession");
			AJ_Status status = AJ_Status.AJ_OK;

			if( status == AJ_Status.AJ_OK) {
				callbackContext.success("Yay!");
				return true;
			} else {
				callbackContext.error("Error: " + status.toString());
				return false;
			}
		}
		if (action.equals("leaveSession")) {
			Log.i(TAG, "AllJoyn.leaveSession");
			AJ_Status status = AJ_Status.AJ_OK;
			
			if( status == AJ_Status.AJ_OK) {
				callbackContext.success("Yay!");
				return true;
			} else {
				callbackContext.error("Error: " + status.toString());
				return false;
			}
		}		
		if (action.equals("invokeMember")) {
			Log.i(TAG, "AllJoyn.invokeMember");
			AJ_Status status = AJ_Status.AJ_OK;
			
			if( status == AJ_Status.AJ_OK) {
				callbackContext.success("Yay!");
				return true;
			} else {
				callbackContext.error("Error: " + status.toString());
				return false;
			}
		}		
		if (action.equals("addInterfacesListener")) {
			Log.i(TAG, "AllJoyn.addInterfacesListener");
			AJ_Status status = AJ_Status.AJ_OK;
			
			if( status == AJ_Status.AJ_OK) {
				callbackContext.success("Yay!");
				return true;
			} else {
				callbackContext.error("Error: " + status.toString());
				return false;
			}
		}		
		if (action.equals("addAdvertisedNameListener")) {
			Log.i(TAG, "AllJoyn.addAdvertisedNameListener");
			AJ_Status status = AJ_Status.AJ_OK;
			
			if( status == AJ_Status.AJ_OK) {
				callbackContext.success("Yay!");
				return true;
			} else {
				callbackContext.error("Error: " + status.toString());
				return false;
			}
		}		
		if (action.equals("addListener")) {
			Log.i(TAG, "AllJoyn.addListener");
			AJ_Status status = AJ_Status.AJ_OK;
			
			if( status == AJ_Status.AJ_OK) {
				callbackContext.success("Yay!");
				return true;
			} else {
				callbackContext.error("Error: " + status.toString());
				return false;
			}
		}		
		return false;
	}
}
