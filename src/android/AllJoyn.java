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

	private static final String TAG = "AllJoyn";

	private BusHandler mBusHandler;

	/* Load the native alljoyn_java library. */
	static {
		System.loadLibrary("alljoyn_java");
	}

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

		/* Make all AllJoyn calls through a separate handler thread to prevent blocking the UI. */
		// HandlerThread busThread = new HandlerThread("BusHandler");
		Log.i(TAG, "AllJoyn: initialization");

		busThread.start();
		mBusHandler = new BusHandler(busThread.getLooper());

		/* Connect to an AllJoyn object. */
		mBusHandler.sendEmptyMessage(BusHandler.CONNECT);
		// mHandler.sendEmptyMessage(MESSAGE_START_PROGRESS_DIALOG);
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
			Log.i(TAG, message);
			callbackContext.success(message);
			return true;
		}

		return false;
	}


   // private Handler mHandler = new Handler() {
   //      @Override
   //      public void handleMessage(Message msg) {
   //          switch (msg.what) {
   //          case MESSAGE_PING:
   //              String ping = (String) msg.obj;
   //              break;
   //          case MESSAGE_PING_REPLY:
   //              String ret = (String) msg.obj;
   //              break;
   //          case MESSAGE_POST_TOAST:
   //              // Toast.makeText(getApplicationContext(), (String) msg.obj, Toast.LENGTH_LONG).show();
   //              break;
   //          case MESSAGE_START_PROGRESS_DIALOG:
   //              break;
   //          case MESSAGE_STOP_PROGRESS_DIALOG:
   //              break;
   //          default:
   //              break;
   //          }
   //      }
   //  };


	class BusHandler extends Handler {
	    /*
	     * Name used as the well-known name and the advertised name of the service this client is
	     * interested in.  This name must be a unique name both to the bus and to the network as a
	     * whole.
	     *
	     * The name uses reverse URL style of naming, and matches the name used by the service.
	     */
	    private static final String SERVICE_NAME = "org.alljoyn.bus.samples.simple";
	    private static final short CONTACT_PORT=42;

	    private BusAttachment mBus;
	    private ProxyBusObject mProxyObj;
	    // private SimpleInterface mSimpleInterface;

	    private int     mSessionId;
	    private boolean mIsInASession;
	    private boolean mIsConnected;
	    private boolean mIsStoppingDiscovery;

	    /* These are the messages sent to the BusHandler from the UI. */
	    public static final int CONNECT = 1;
	    public static final int JOIN_SESSION = 2;
	    public static final int DISCONNECT = 3;
	    public static final int PING = 4;

	    public BusHandler(Looper looper) {
	        super(looper);

	        mIsInASession = false;
	        mIsConnected = false;
	        mIsStoppingDiscovery = false;
	    }

	    @Override
	    public void handleMessage(Message msg) {
	        switch(msg.what) {
	        /* Connect to a remote instance of an object implementing the SimpleInterface. */
	        case CONNECT: {
	        	Log.i("Got Connect");
	            // org.alljoyn.bus.alljoyn.DaemonInit.PrepareDaemon(getApplicationContext());
	            /*
	             * All communication through AllJoyn begins with a BusAttachment.
	             *
	             * A BusAttachment needs a name. The actual name is unimportant except for internal
	             * security. As a default we use the class name as the name.
	             *
	             * By default AllJoyn does not allow communication between devices (i.e. bus to bus
	             * communication). The second argument must be set to Receive to allow communication
	             * between devices.
	             */
	            // mBus = new BusAttachment(getPackageName(), BusAttachment.RemoteMessage.Receive);

	            /*
	             * Create a bus listener class
	             */
	            // mBus.registerBusListener(new BusListener() {
	            //     @Override
	            //     public void foundAdvertisedName(String name, short transport, String namePrefix) {
	            //         Log.i(String.format("MyBusListener.foundAdvertisedName(%s, 0x%04x, %s)", name, transport, namePrefix));
	                    /*
	                     * This client will only join the first service that it sees advertising
	                     * the indicated well-known name.  If the program is already a member of
	                     * a session (i.e. connected to a service) we will not attempt to join
	                     * another session.
	                     * It is possible to join multiple session however joining multiple
	                     * sessions is not shown in this sample.
	                     */
	            //         if(!mIsConnected) {
	            //             Message msg = obtainMessage(JOIN_SESSION);
	            //             msg.arg1 = transport;
	            //             msg.obj = name;
	            //             sendMessage(msg);
	            //         }
	            //     }
	            // });

	            /* To communicate with AllJoyn objects, we must connect the BusAttachment to the bus. */
	            // Status status = mBus.connect();
	            // Log.i("BusAttachment.connect()", status);
	            // if (Status.OK != status) {
	            //     finish();
	            //     return;
	            // }

	            /*
	             * Now find an instance of the AllJoyn object we want to call.  We start by looking for
	             * a name, then connecting to the device that is advertising that name.
	             *
	             * In this case, we are looking for the well-known SERVICE_NAME.
	             */
	            // status = mBus.findAdvertisedName(SERVICE_NAME);
	            // logStatus(String.format("BusAttachement.findAdvertisedName(%s)", SERVICE_NAME), status);
	            // if (Status.OK != status) {
	            //     finish();
	            //     return;
	            // }

	            break;
	        }
	        case (JOIN_SESSION): {
	        	Log.i("Got JOIN_SESSION");
	            /*
	             * If discovery is currently being stopped don't join to any other sessions.
	             */
	            // if (mIsStoppingDiscovery) {
	            //     break;
	            // }

	            /*
	             * In order to join the session, we need to provide the well-known
	             * contact port.  This is pre-arranged between both sides as part
	             * of the definition of the chat service.  As a result of joining
	             * the session, we get a session identifier which we must use to
	             * identify the created session communication channel whenever we
	             * talk to the remote side.
	             */
	            // short contactPort = CONTACT_PORT;
	            // SessionOpts sessionOpts = new SessionOpts();
	            // sessionOpts.transports = (short)msg.arg1;
	            // Mutable.IntegerValue sessionId = new Mutable.IntegerValue();

	            // Status status = mBus.joinSession((String) msg.obj, contactPort, sessionId, sessionOpts, new SessionListener() {
	            //     @Override
	            //     public void sessionLost(int sessionId, int reason) {
	            //         mIsConnected = false;
	            //         logInfo(String.format("MyBusListener.sessionLost(sessionId = %d, reason = %d)", sessionId,reason));
	            //         mHandler.sendEmptyMessage(MESSAGE_START_PROGRESS_DIALOG);
	            //     }
	            // });
	            // logStatus("BusAttachment.joinSession() - sessionId: " + sessionId.value, status);

	            // if (status == Status.OK) {
	                /*
	                 * To communicate with an AllJoyn object, we create a ProxyBusObject.
	                 * A ProxyBusObject is composed of a name, path, sessionID and interfaces.
	                 *
	                 * This ProxyBusObject is located at the well-known SERVICE_NAME, under path
	                 * "/SimpleService", uses sessionID of CONTACT_PORT, and implements the SimpleInterface.
	                 */
	                // mProxyObj =  mBus.getProxyBusObject(SERVICE_NAME,
	                //                                     "/SimpleService",
	                //                                     sessionId.value,
	                //                                     new Class<?>[] { SimpleInterface.class });

	                //  We make calls to the methods of the AllJoyn object through one of its interfaces. 
	                // mSimpleInterface =  mProxyObj.getInterface(SimpleInterface.class);

	                // mSessionId = sessionId.value;
	                // mIsConnected = true;
	                // mHandler.sendEmptyMessage(MESSAGE_STOP_PROGRESS_DIALOG);
	            // }
	            break;
	        }

	        /* Release all resources acquired in the connect. */
	        case DISCONNECT: {
	            mIsStoppingDiscovery = true;
	            if (mIsConnected) {
	                Status status = mBus.leaveSession(mSessionId);
	                logStatus("BusAttachment.leaveSession()", status);
	            }
	            mBus.disconnect();
	            getLooper().quit();
	            break;
	        }

	        /*
	         * Call the service's Ping method through the ProxyBusObject.
	         *
	         * This will also print the String that was sent to the service and the String that was
	         * received from the service to the user interface.
	         */
	        case PING: {
	            // try {
	            //     if (mSimpleInterface != null) {
	            //         sendUiMessage(MESSAGE_PING, msg.obj);
	            //         String reply = mSimpleInterface.Ping((String) msg.obj);
	            //         sendUiMessage(MESSAGE_PING_REPLY, reply);
	            //     }
	            // } catch (BusException ex) {
	            //     logException("SimpleInterface.Ping()", ex);
	            // }
	            break;
	        }
	        default:
	            break;
	        }
	    }

	    /* Helper function to send a message to the UI thread. */
	    // private void sendUiMessage(int what, Object obj) {
	    //     mHandler.sendMessage(mHandler.obtainMessage(what, obj));
	    // }
	}
}
