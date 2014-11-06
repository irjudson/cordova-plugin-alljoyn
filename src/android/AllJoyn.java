package org.allseen.alljoyn;

import org.alljoyn.bus.BusAttachment;
import org.alljoyn.bus.BusException;
import org.alljoyn.bus.BusListener;
import org.alljoyn.bus.Mutable;
import org.alljoyn.bus.ProxyBusObject;
import org.alljoyn.bus.SessionListener;
import org.alljoyn.bus.SessionOpts;
import org.alljoyn.bus.Status;

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
	/* Load the native alljoyn_java library. */
	static {
		System.loadLibrary("alljoyn_java");
	}

	private static final String TAG = "AllJoyn";
	private static final short CONTACT_PORT=42;

	BusAttachment mBus;

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

		Log.i(TAG, "AllJoyn: initialization");

		mBus = new BusAttachment(getClass().getName(), BusAttachment.RemoteMessage.Receive);

		Log.i(TAG, "AllJoyn: created bus attachment");

		mBus.registerBusListener(new BusListener() {
			@Override
			public void foundAdvertisedName(String name, short transport, String namePrefix) {
				mBus.enableConcurrentCallbacks();
				Log.i(TAG, "Found: " + name);
				short contactPort = CONTACT_PORT;
				SessionOpts sessionOpts = new SessionOpts();
				Mutable.IntegerValue sessionId = new Mutable.IntegerValue();
				Status status = mBus.joinSession(name, contactPort, sessionId, sessionOpts, new SessionListener());
			}
		});

		Log.i(TAG, "AllJoyn: registered bus listener");
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

		if (action.equals("hello")) {
			Log.i(TAG, "Calling hello");
			String name = data.getString(0);
			String message = "Hello, " + name;
			callbackContext.success(message);
			return true;
		}

		if (action.equals("start")) {
			Log.i(TAG, "Calling start");
			Status status = mBus.connect();
			if (status != Status.OK) {
				callbackContext.error("AllJoyn Connect Error: " + status.getErrorCode());
				return false;
			} else {
				status = mBus.findAdvertisedName("org.alljoyn.BusNode");
				if (status != Status.OK) {
    				callbackContext.error("AllJoyn Find Daemon Error: " + status.getErrorCode());
					return false;
				}
				callbackContext.success("Success: " + status.getErrorCode());
				return true;
			}
		}

		return false;
	}
}
