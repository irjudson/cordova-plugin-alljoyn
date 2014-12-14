#pragma once

#include <ppltasks.h>

#include "aj_introspect.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;


#ifdef AJ_PrintXML
#undef AJ_PrintXML
#endif // AJ_PrintXML


namespace AllJoynWinRTComponent
{
	/**
	* Type for an interface description - NULL terminated array of strings.
	*/
	typedef IVector<IVector<String^>^>^ AJ_InterfaceDescription;

	/**
	* Prototype for a function provided by the property store for getting ANNOUNCE and ABOUT properties
	*
	* @param reply     The message to marshal the property values into. The getter can also figure out
	*                  from the msgId in the reply message if the reply is for ANNOUNCE or ABOUT.
	*
	* @param language  The language to use to return the string properties. If this is NULL the default
	*                  language will be used.
	*
	* @return   Return AJ_OK if the properties were succesfully marshaled into the reply.
	*
	*/
	public enum class AJ_Status
	{
		AJ_OK = 0,  /**< Success status */
		AJ_ERR_NULL = 1,  /**< Unexpected NULL pointer */
		AJ_ERR_UNEXPECTED = 2,  /**< An operation was unexpected at this time */
		AJ_ERR_INVALID = 3,  /**< A value was invalid */
		AJ_ERR_IO_BUFFER = 4,  /**< An I/O buffer was invalid or in the wrong state */
		AJ_ERR_READ = 5,  /**< An error while reading data from the network */
		AJ_ERR_WRITE = 6,  /**< An error while writing data to the network */
		AJ_ERR_TIMEOUT = 7,  /**< A timeout occurred */
		AJ_ERR_MARSHAL = 8,  /**< Marshaling failed due to badly constructed message argument */
		AJ_ERR_UNMARSHAL = 9,  /**< Unmarshaling failed due to a corrupt or invalid message */
		AJ_ERR_END_OF_DATA = 10, /**< Not enough data */
		AJ_ERR_RESOURCES = 11, /**< Insufficient memory to perform the operation */
		AJ_ERR_NO_MORE = 12, /**< Attempt to unmarshal off the end of an array */
		AJ_ERR_SECURITY = 13, /**< Authentication or decryption failed */
		AJ_ERR_CONNECT = 14, /**< Network connect failed */
		AJ_ERR_UNKNOWN = 15, /**< A unknown value */
		AJ_ERR_NO_MATCH = 16, /**< Something didn't match */
		AJ_ERR_SIGNATURE = 17, /**< Signature is not what was expected */
		AJ_ERR_DISALLOWED = 18, /**< An operation was not allowed */
		AJ_ERR_FAILURE = 19, /**< A failure has occurred */
		AJ_ERR_RESTART = 20, /**< The OEM event loop must restart */
		AJ_ERR_LINK_TIMEOUT = 21, /**< The bus link is inactive too long */
		AJ_ERR_DRIVER = 22, /**< An error communicating with a lower-layer driver */
		AJ_ERR_OBJECT_PATH = 23, /**< Object path was not specified */
		AJ_ERR_BUSY = 24, /**< An operation failed and should be retried later */
		AJ_ERR_DHCP = 25, /**< A DHCP operation has failed */
		AJ_ERR_ACCESS = 26, /**< The operation specified is not allowed */
		AJ_ERR_SESSION_LOST = 27, /**< The session was lost */
		AJ_ERR_LINK_DEAD = 28, /**< The network link is now dead */
		AJ_ERR_HDR_CORRUPT = 29, /**< The message header was corrupt */
		AJ_ERR_RESTART_APP = 30, /**< The application must cleanup and restart */
		AJ_ERR_INTERRUPTED = 31, /**< An I/O operation (READ) was interrupted */
		AJ_ERR_REJECTED = 32, /**< The connection was rejected */
		AJ_ERR_RANGE = 33, /**< Value provided was out of range */
		AJ_ERR_ACCESS_ROUTING_NODE = 34, /**< Access defined by routing node */
		AJ_ERR_KEY_EXPIRED = 35, /**< The key has expired */
		AJ_ERR_SPI_NO_SPACE = 36, /**< Out of space error */
		AJ_ERR_SPI_READ = 37, /**< Read error */
		AJ_ERR_SPI_WRITE = 38, /**< Write error */
		AJ_ERR_OLD_VERSION = 39, /**< Router you connected to is old and unsupported */
		AJ_ERR_NVRAM_READ = 40, /**< Error while reading from NVRAM */
		AJ_ERR_NVRAM_WRITE = 41, /**< Error while writing to NVRAM */
		/*
		* REMINDER: Update AJ_StatusText in aj_debug.c if adding a new status code.
		*/
		AJ_STATUS_LAST = 41  /**< The last error status code */

	};

	/**
	* Type for a bus attachment
	*/
	public ref struct AJ_BusAttachment sealed
	{
	internal:
		::AJ_BusAttachment* _bus;
	};

	/**
	* Type for describing session options
	*/
	public ref struct AJ_SessionOpts sealed
	{
		property uint8_t traffic;								/**< traffic type */
		property uint8_t proximity;								/**< proximity */
		property uint16_t transports;							/**< allowed transports */
		property uint32_t isMultipoint;							/**< multi-point session capable */
	};

	/**
	* Type for an AllJoyn object description
	*/
	public ref struct AJ_Object sealed
	{
		property String^ path;									/**< object path */
		property AJ_InterfaceDescription interfaces;			/**< interface descriptor */
		property uint8_t flags;                                 /**< flags for the object */
	};

	/**
	* AllJoyn Message
	*/
	public ref struct AJ_Message sealed
	{
	internal:
		::AJ_Message* _msg;
	};

	/**
	* Type for a message argument
	*/
	public ref struct AJ_Arg sealed
	{
	internal:
		::AJ_Arg* _arg;
	};

	/**
	* AllJoyn Windows Runtime
	*/
    public ref class AllJoyn sealed
    {
    public:
		AllJoyn();
		virtual ~AllJoyn();

		static void AJ_Initialize();
		static void AJ_PrintXML(const Array<AJ_Object^>^ localObjects);
		static void AJ_RegisterObjects(const Array<AJ_Object^>^ localObjects, const Array<AJ_Object^>^ proxyObjects);
		static AJ_Status AJ_StartClient(AJ_BusAttachment^ bus,
										String^ daemonName,
										uint32_t timeout,
										uint8_t connected,
										String^ name,
										uint16_t port,
										uint32_t* sessionId,
										AJ_SessionOpts^ opts);
		static void AJ_ReleaseObjects();
		static AJ_Status AJ_MarshalMethodCall(AJ_BusAttachment^ bus, AJ_Message^ msg, uint32_t msgId, String^ destination, AJ_SessionId sessionId, uint8_t flags, uint32_t timeout);
		static AJ_Status AJ_MarshalArgs(AJ_Message^ msg, String^ signature, const Array<String^>^ args);
		static AJ_Status AJ_DeliverMsg(AJ_Message^ msg);
		static AJ_Status AJ_CloseMsg(AJ_Message^ msg);
		static AJ_Status AJ_UnmarshalMsg(AJ_BusAttachment^ bus, AJ_Message^ msg, uint32_t timeout);
		static AJ_Status AJ_UnmarshalArg(AJ_Message^ bus, AJ_Arg^ msg);
		static AJ_Status AJ_CloseArg(AJ_Arg^ arg);

		// Helper functions
		static uint32_t Get_AJ_Message_msgId(AJ_Message^ msg);
		static String^ Get_AJ_Arg_v_string(AJ_Arg^ arg);

		// Testing
		static IAsyncOperation<String^>^ Test();
		static IAsyncOperation<String^>^ GetVersion();
		static IAsyncAction^ Initialize();

	private:
		static ::AJ_Object* RegisterObjects(const Array<AJ_Object^>^);
		static void ReleaseObjects(::AJ_Object*, const Array<AJ_Object^>^);
    };
}
