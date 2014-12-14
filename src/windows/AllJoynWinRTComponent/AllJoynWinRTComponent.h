#pragma once

#include <ppltasks.h>

#include "aj_introspect.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;


#ifdef AJ_PrintXML(objs)
#undef AJ_PrintXML(objs)
#endif // AJ_PrintXML(objs)


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
	private public:
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
	* Type for a message argument
	*/
	public ref struct AJ_Arg sealed
	{
		property uint8_t typeId;								/**< the argument type */
		property uint8_t flags;									/**< non-zero if the value is a variant - values > 1 indicate variant-of-variant etc. */
		property uint16_t len;									/**< length of a string or array in bytes */

		// Phong TODO
		///*
		//* Union of the various argument values.
		//*/
		//union {
		//	uint8_t*     v_byte;        /**< byte type field value in the message */
		//	int16_t*     v_int16;       /**< int16 type field value in the message */
		//	uint16_t*    v_uint16;      /**< uint16 type field value in the message */
		//	uint32_t*    v_bool;        /**< boolean type field value in the message */
		//	uint32_t*    v_uint32;      /**< uint32 type field value in the message */
		//	int32_t*     v_int32;       /**< int32 type field value in the message */
		//	int64_t*     v_int64;       /**< int64 type field value in the message */
		//	uint64_t*    v_uint64;      /**< uint64 type field value in the message */
		//	double*      v_double;      /**< double type field value in the message */
		//	const char*  v_string;      /**< string(char *) type field value in the message */
		//	const char*  v_objPath;     /**< objPath(char *) type field value in the message */
		//	const char*  v_signature;   /**< signature(char *) type field value in the message */
		//	const void*  v_data;        /**< data(void *) type field value in the message */
		//} val;                          /**< union of the field value in the message */

		property String^ sigPtr;								/**< pointer to the signature */
		property AJ_Arg^ container;								/**< container argument */
	};

	/**
	* AllJoyn Message Header
	*/
	public ref struct AJ_MsgHeader sealed
	{
		property uint8_t endianess;								/**< The endianness of this message */
		property uint8_t msgType;								/**< Indicates if the message is method call, signal, etc. */
		property uint8_t flags;									/**< Flag bits */
		property uint8_t majorVersion;							/**< Major version of this message */
		property uint32_t bodyLen;								/**< Length of the body data */
		property uint32_t serialNum;							/**< serial of this message */
		property uint32_t headerLen;							/**< Length of the header data */
	};

	/**
	* AllJoyn Message
	*/
	public ref struct AJ_Message sealed
	{
		property uint32_t msgId;								/**< Identifies the message to the application */
		property AJ_MsgHeader^ hdr;								/**< The message header */
		// Phong TODO
		//union {
		//	const char* objPath;   /**< The nul terminated object path string or NULL */
		//	uint32_t replySerial;  /**< The reply serial number */
		//};
		//union {
		//	const char* member;    /**< The nul terminated member name string or NULL */
		//	const char* error;     /**< The nul terminated error name string or NULL */
		//};
		property String^ iface;									/**< The nul terminated interface string or NULL */
		property String^ sender;								/**< The nul terminated sender string or NULL */
		property String^ destination;							/**< The nul terminated destination string or NULL */
		property String^ signature;								/**< The nul terminated signature string or NULL */
		property uint32_t sessionId;							/**< Session id */
		property uint32_t timestamp;							/**< Timestamp */
		property uint32_t ttl;									/**< Time to live */
		/*
		* Private message state - the application should not touch this data
		*/
		property uint8_t sigOffset;								/**< Offset to current position in the signature */
		property uint8_t varOffset;								/**< For variant marshalling/unmarshalling - Offset to start of variant signature */
		property uint16_t bodyBytes;							/**< Running count of the number body bytes written */
		property AJ_BusAttachment^ bus;							/**< Bus attachment for this message */
		property AJ_Arg^ outer;									/**< Container arg current being marshaled */
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
		static AJ_Status AJ_MarshalMethodCall(AJ_BusAttachment^ bus, AJ_Message^* msg, uint32_t msgId, String^ destination, AJ_SessionId sessionId, uint8_t flags, uint32_t timeout);


		// Testing
		static IAsyncOperation<String^>^ Test();
		static IAsyncOperation<String^>^ GetVersion();
		static IAsyncAction^ Initialize();

	private:
		static ::AJ_Object* RegisterObjects(const Array<AJ_Object^>^);
		static void ReleaseObjects(::AJ_Object*, const Array<AJ_Object^>^);

		// Helper functions
		static AJ_Message^ CPPCX2C(::AJ_Message* msg);
		static AJ_MsgHeader^ CPPCX2C(::AJ_MsgHeader* hdr);
		static AJ_Arg^ CPPCX2C(::AJ_Arg* arg);
    };
}
