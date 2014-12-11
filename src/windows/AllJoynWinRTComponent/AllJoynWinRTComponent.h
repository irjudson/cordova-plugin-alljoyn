﻿#pragma once

#include <ppltasks.h>

#include "aj_introspect.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;



namespace AllJoynWinRTComponent
{
	typedef IVector<IVector<String^>^>^ AJ_InterfaceDescription;

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

	public ref struct AJ_IOBuffer sealed
	{
		property uint8_t direction;								/**< I/O buffer is either a Tx buffer or an Rx buffer */
		property uint16_t bufSize;								/**< Size of the data buffer */
		property IBox<uint8_t>^ bufStart;						/**< Start for the data buffer */
		property IBox<uint8_t>^ readPtr;						/**< Current position in buf for reading data */
		property IBox<uint8_t>^ writePtr;						/**< Current position in buf for writing data */

		// Phong TODO
		///*
		//* Function pointer to send or recv function
		//*/
		//union 
		//{
		//	AJ_TxFunc send;
		//	AJ_RxFunc recv;
		//};
		//void* context;										/**< Abstracted context for managing I/O */
	};

	public ref struct AJ_NetSocket sealed
	{
		property AJ_IOBuffer^ tx;								/**< transmit network socket */
		property AJ_IOBuffer^ rx;								/**< receive network socket */
	};

	public ref struct AJ_BusAttachment sealed
	{
		property uint16_t aboutPort;							/**< The port to use in announcements */
		property String^ uniqueName;							/**< The unique name returned by the hello message */
		property AJ_NetSocket^ sock;							/**< Abstracts a network socket */
		property uint32_t serial;								/**< Next outgoing message serial number */
		// Phong TODO
		//AJ_AuthPwdFunc pwdCallback;							/**< Callback for obtaining passwords */
		//AJ_AuthListenerFunc authListenerCallback;				/**< Callback for obtaining passwords */
		property IBox<uint32_t>^ suites;						/**< Supported cipher suites */
		property size_t numsuites;								/**< Number of supported cipher suites */
	};

	public ref struct AJ_SessionOpts sealed
	{
		property uint8_t traffic;								/**< traffic type */
		property uint8_t proximity;								/**< proximity */
		property uint16_t transports;							/**< allowed transports */
		property uint32_t isMultipoint;							/**< multi-point session capable */
	};

	public ref struct AJ_Object sealed
	{
		property String^ path;									/**< object path */
		property AJ_InterfaceDescription interfaces;			/**< interface descriptor */
		property uint8_t flags;                                 /**< flags for the object */

		// Phong TODO
		//void* context;										/**< an application provided context pointer for this object */
	};

    public ref class AllJoyn sealed
    {
    public:
		AllJoyn();
		virtual ~AllJoyn();

		static void AJ_Initialize();
		static void AJ_RegisterObjects(const Array<AJ_Object^>^ localObjects, const Array<AJ_Object^>^ proxyObjects);
		static AJ_Status AJ_StartClient(AJ_BusAttachment^ bus,
										String^ daemonName,
										uint32_t timeout,
										uint8_t connected,
										String^ name,
										uint16_t port,
										uint32_t* sessionId,
										AJ_SessionOpts^ opts);

		// Testing
		static IAsyncOperation<String^>^ Test();
		static IAsyncOperation<String^>^ GetVersion();
		static IAsyncAction^ Initialize();

	private:
		static ::AJ_Object* RegisterObject(const Array<AJ_Object^>^ objects);
    };
}

#undef AJ_InterfaceDescription