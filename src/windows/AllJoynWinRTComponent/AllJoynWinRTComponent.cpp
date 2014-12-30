#include "pch.h"

#include "aj_init.h"
#include "aj_util.h"
#include "aj_target_util.h"
#include "aj_helper.h"
#include "aj_msg.h"
#include "aj_connect.h"
#include "AllJoynWinRTComponent.h"
#include "aj_debug.h"
#include <ppltasks.h>

using namespace concurrency;

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;


#define AJ_BUS_ID_FLAG   0x00  /**< Identifies that a message belongs to the set of builtin bus object messages */
#define AJ_APP_ID_FLAG   0x01  /**< Identifies that a message belongs to the set of objects implemented by the application */
#define AJ_PRX_ID_FLAG   0x02  /**< Identifies that a message belongs to the set of objects implemented by remote peers */


#define STRUCT_COPY(name, attrib) _ ## name->attrib = name->attrib

#define NULLABLE_TYPE_COPY(type, name)										\
	type* _ ## name = NULL;													\
	if (name != nullptr)													\
	{																		\
		*_ ## name = name->Value;											\
	}																		\


#define WCS2MBS(string)														\
	char __ ## string[MAX_STR_LENGTH];										\
	wcstombs(__ ## string, string->Data(), MAX_STR_LENGTH);					\
	char* _ ## string = (string == nullptr) ? NULL : __ ## string			\


#define SAFE_DEL(p)															\
	if (p)																	\
	{																		\
		delete p;															\
		p = NULL;															\
	}


#define SAFE_DEL_ARRAY(p)													\
	if (p)																	\
	{																		\
		delete[] p;															\
		p = NULL;															\
	}


static ::AJ_Object* _s_cachedLocalObjects = NULL;
static ::AJ_Object* _s_cachedProxyObjects = NULL;
static const Array<AllJoynWinRTComponent::AJ_Object^>^ s_cachedLocalObjects;
static const Array<AllJoynWinRTComponent::AJ_Object^>^ s_cachedProxyObjects;
static ::AJ_SessionOpts* _s_cachedSessionOpts = NULL;


AllJoynWinRTComponent::AllJoyn::AllJoyn()
{
}


AllJoynWinRTComponent::AllJoyn::~AllJoyn()
{
}


void AllJoynWinRTComponent::AllJoyn::AJ_Initialize()
{
	::AJ_Initialize();
}


void AllJoynWinRTComponent::AllJoyn::ReleaseObjects(::AJ_Object* _objects, const Array<AJ_Object^>^ objects)
{
	if (_objects == NULL)
	{
		return;
	}

	int nObjects = objects->Length;

	for (int j = 0; j < nObjects; j++)
	{
		if (_objects[j].path)
		{
			// Free path
			SAFE_DEL(_objects[j].path);

			// Free interfaces
			int nInterfaces = objects[j]->interfaces->Size;

			for (int k = 0; k < nInterfaces; k++)
			{
				if (_objects[j].interfaces[k])
				{
					int nEntries = objects[j]->interfaces->GetAt(k)->Size;

					for (int m = 0; m < nEntries; m++)
					{
						if (_objects[j].interfaces[k][m])
						{
							delete _objects[j].interfaces[k][m];
						}
					}

					delete[] _objects[j].interfaces[k];
				}
			}

			SAFE_DEL_ARRAY(_objects[j].interfaces);
		}
	}

	SAFE_DEL_ARRAY(_objects);
}


void AllJoynWinRTComponent::AllJoyn::AJ_ReleaseObjects()
{
	ReleaseObjects(_s_cachedLocalObjects, s_cachedLocalObjects);
	ReleaseObjects(_s_cachedProxyObjects, s_cachedProxyObjects);
}


::AJ_Object* AllJoynWinRTComponent::AllJoyn::RegisterObjects(const Array<AJ_Object^>^ objects)
{
	::AJ_Object* _objects = NULL;

	if (objects->Length != 0)
	{
		_objects = new ::AJ_Object[objects->Length];
		ZeroMemory(_objects, sizeof(_objects));

		for (int i = 0; i < objects->Length; i++)
		{
			ZeroMemory(&_objects[i], sizeof(_objects[i]));

			if (objects[i])
			{
				// Copy path
				char* _path = new char[MAX_STR_LENGTH];
				AJ_StringToChars(objects[i]->path, _path);
				_objects[i].path = _path;

				// Copy interface
				int nInterfaces = objects[i]->interfaces->Size;
				::AJ_InterfaceDescription* _interfaces = new ::AJ_InterfaceDescription[nInterfaces];
				ZeroMemory(_interfaces, sizeof(_interfaces));
				char*** interfaces = new char**[nInterfaces];
				ZeroMemory(interfaces, sizeof(interfaces));
				for (int j = 0; j < nInterfaces; j++)
				{
					_interfaces[j] = NULL;
					if (objects[i]->interfaces->GetAt(j))
					{
						int nEntries = objects[i]->interfaces->GetAt(j)->Size;
						interfaces[j] = new char*[nEntries + 1];
						ZeroMemory(interfaces[j], sizeof(interfaces[j]));
						for (int k = 0; k < nEntries; k++)
						{
							char* entry = NULL;
							if (objects[i]->interfaces->GetAt(j)->GetAt(k))
							{
								entry = new char[MAX_STR_LENGTH];
								AJ_StringToChars(objects[i]->interfaces->GetAt(j)->GetAt(k), entry);
							}
							interfaces[j][k] = entry;
						}
						_interfaces[j] = interfaces[j];
						_interfaces[nEntries] = NULL;
					}
				}
				_objects[i].interfaces = _interfaces;

				// Copy flag
				_objects[i].flags = objects[i]->flags;
			}
		}
	}

	return _objects;
}


void AllJoynWinRTComponent::AllJoyn::AJ_PrintXML(const Array<AJ_Object^>^ objects)
{
	::AJ_Object* _objects = AllJoynWinRTComponent::AllJoyn::RegisterObjects(objects);
#if _DEBUG
	::AJ_PrintXML(_objects);
#endif
	ReleaseObjects(_objects, objects);
}


void AllJoynWinRTComponent::AllJoyn::AJ_RegisterObjects(const Array<AJ_Object^>^ localObjects, const Array<AJ_Object^>^ proxyObjects)
{
	// Free the old objects first
	if (_s_cachedLocalObjects)
	{
		ReleaseObjects(_s_cachedLocalObjects, s_cachedLocalObjects);
		ReleaseObjects(_s_cachedProxyObjects, s_cachedProxyObjects);
	}

	// Cache the objects
	_s_cachedLocalObjects = AllJoynWinRTComponent::AllJoyn::RegisterObjects(localObjects);
	_s_cachedProxyObjects = AllJoynWinRTComponent::AllJoyn::RegisterObjects(proxyObjects);
	s_cachedLocalObjects = localObjects;
	s_cachedProxyObjects = proxyObjects;

	// Register the objects
	::AJ_RegisterObjects(_s_cachedLocalObjects, _s_cachedProxyObjects);
}


IAsyncOperation<AllJoynWinRTComponent::AJ_Session>^ AllJoynWinRTComponent::AllJoyn::AJ_StartClient
(
	AllJoynWinRTComponent::AJ_BusAttachment^ bus,
	String^ daemonName,
	uint32_t timeout,
	uint8_t connected,
	String^ name,
	uint16_t port,
	AllJoynWinRTComponent::AJ_SessionOpts^ opts)
{
	return create_async([bus, daemonName, timeout, connected, name, port, opts]() -> AllJoynWinRTComponent::AJ_Session
	{
		::AJ_BusAttachment* _bus = new ::AJ_BusAttachment();
		::AJ_SessionOpts* _opts = NULL;

		WCS2MBS(daemonName);
		WCS2MBS(name);

		if (opts)
		{
			SAFE_DEL(_s_cachedSessionOpts);
			_opts = new ::AJ_SessionOpts();
			ZeroMemory(_opts, sizeof(_opts));

			STRUCT_COPY(opts, isMultipoint);
			STRUCT_COPY(opts, proximity);
			STRUCT_COPY(opts, traffic);
			STRUCT_COPY(opts, transports);

			_s_cachedSessionOpts = _opts;
		}

		uint32_t _sessionId;
		::AJ_Status _status = ::AJ_StartClient(_bus, _daemonName, timeout, connected, _name, port, &_sessionId, _opts);
		bus->_bus = _bus;
		AJ_Session retObj;
		retObj.sessionId = _sessionId;
		retObj.status = static_cast<uint8_t>(_status);

		return retObj;
	});
}


AllJoynWinRTComponent::AJ_Status AllJoynWinRTComponent::AllJoyn::AJ_MarshalMethodCall(AJ_BusAttachment^ bus, AJ_Message^ msg, uint32_t msgId, String^ destination, AJ_SessionId sessionId, uint8_t flags, uint32_t timeout)
{
	if (msg->_msg)
	{
		SAFE_DEL(msg->_msg->destination);
	}

	SAFE_DEL(msg->_msg);

	msg->_msg = new ::AJ_Message();
	char* _destination = new char[MAX_STR_LENGTH];
	wcstombs(_destination, destination->Data(), MAX_STR_LENGTH);

	::AJ_Status _status = ::AJ_MarshalMethodCall(bus->_bus, msg->_msg, msgId, _destination, sessionId, flags, timeout);

	return (static_cast<AJ_Status>(_status));
}


AllJoynWinRTComponent::AJ_Status AllJoynWinRTComponent::AllJoyn::AJ_MarshalArgs(AJ_Message^ msg, String^ signature, const Array<String^>^ args)
{
	::AJ_Status _status = ::AJ_Status::AJ_ERR_INVALID;

	WCS2MBS(signature);

	for (int i = 0; i < args->Length; i++)
	{
		::AJ_Arg arg;
		uint8_t u8;
		uint16_t u16;
		uint32_t u32;
		uint64_t u64;
		void* val = NULL;
		uint8_t typeId = (uint8_t)_signature[i];

		switch (_signature[i])
		{
			/**< AllJoyn 64-bit unsigned integer basic type */
			case 't':
				u64 = _wtoi64(args[i]->Data());
				val = &u64;
				break;

			/**< AllJoyn 32-bit unsigned integer basic type */
			case 'u':
				u32 = _wtoi(args[i]->Data());
				val = &u32;
				break;

			/**< AllJoyn 16-bit unsigned integer basic type */
			case 'q':
				u16 = _wtoi(args[i]->Data());
				val = &u16;
				break;

			/**< AllJoyn 8-bit unsigned integer basic type */
			case 'y':
				u8 = _wtoi(args[i]->Data());
				val = &u8;
				break;

			/**< AllJoyn UTF-8 NULL terminated string basic type */
			case 's':
				char str[MAX_STR_LENGTH];
				wcstombs(str, args[i]->Data(), MAX_STR_LENGTH);
				val = &str;
				break;

			default:
				// Unsupported type
				break;
		}

		if (val)
		{
			arg.typeId = typeId;
			arg.flags = 0;
			arg.len = 0;
			arg.val.v_data = (void*)val;
			arg.sigPtr = NULL;
			arg.container = NULL;
			_status = ::AJ_MarshalArg(msg->_msg, &arg);

			if (_status != AJ_OK)
			{
				AJ_ErrPrintf(("AJ_MarshalArgs(): status=%s\n", AJ_StatusText(_status)));
				break;
			}
		}
		else
		{
			_status = ::AJ_Status::AJ_ERR_INVALID;
			break;
		}
	}

	return (static_cast<AJ_Status>(_status));
}


AllJoynWinRTComponent::AJ_Status AllJoynWinRTComponent::AllJoyn::AJ_DeliverMsg(AJ_Message^ msg)
{
	::AJ_Status _status = ::AJ_DeliverMsg(msg->_msg);

	return (static_cast<AJ_Status>(_status));
}


AllJoynWinRTComponent::AJ_Status AllJoynWinRTComponent::AllJoyn::AJ_CloseMsg(AJ_Message^ msg)
{
	::AJ_Status _status = ::AJ_CloseMsg(msg->_msg);
	SAFE_DEL(msg->_msg->destination);
	SAFE_DEL(msg->_msg);

	return (static_cast<AJ_Status>(_status));
}


IAsyncOperation<AllJoynWinRTComponent::AJ_Status>^ AllJoynWinRTComponent::AllJoyn::AJ_UnmarshalMsg(AJ_BusAttachment^ bus, AJ_Message^ msg, uint32_t timeout)
{
	return create_async([bus, msg, timeout]() -> AllJoynWinRTComponent::AJ_Status
	{
		SAFE_DEL(msg->_msg);
		msg->_msg = new ::AJ_Message();
		::AJ_Status _status = ::AJ_UnmarshalMsg(bus->_bus, msg->_msg, timeout);

		return (static_cast<AJ_Status>(_status));
	});
}


AllJoynWinRTComponent::AJ_Status AllJoynWinRTComponent::AllJoyn::AJ_UnmarshalArg(AJ_Message^ msg, AJ_Arg^ arg)
{
	SAFE_DEL(arg->_arg);
	arg->_arg = new ::AJ_Arg();
	::AJ_Status _status = ::AJ_UnmarshalArg(msg->_msg, arg->_arg);

	return (static_cast<AJ_Status>(_status));
}


AllJoynWinRTComponent::AJ_Status AllJoynWinRTComponent::AllJoyn::AJ_CloseArg(AJ_Arg^ arg)
{
	SAFE_DEL(arg->_arg);

	return AJ_Status::AJ_OK;
}


AllJoynWinRTComponent::AJ_Status AllJoynWinRTComponent::AllJoyn::AJ_BusHandleBusMessage(AJ_Message^ msg)
{
	::AJ_Status _status = ::AJ_BusHandleBusMessage(msg->_msg);

	return (static_cast<AJ_Status>(_status));
}


AllJoynWinRTComponent::AJ_Status AllJoynWinRTComponent::AllJoyn::AJ_BusFindAdvertisedName(AJ_BusAttachment^ bus, String^ namePrefix, uint8_t op)
{
	WCS2MBS(namePrefix);
	::AJ_Status _status = ::AJ_BusFindAdvertisedName(bus->_bus, _namePrefix, op);

	return (static_cast<AJ_Status>(_status));
}


AllJoynWinRTComponent::AJ_Status AllJoynWinRTComponent::AllJoyn::AJ_FindBusAndConnect(AJ_BusAttachment^ bus, String^ serviceName, uint32_t timeout)
{
	SAFE_DEL(bus->_bus);
	::AJ_BusAttachment* _bus = new ::AJ_BusAttachment();
	WCS2MBS(serviceName);
	::AJ_Status _status = ::AJ_FindBusAndConnect(_bus, _serviceName, timeout);
	bus->_bus = _bus;

	return (static_cast<AJ_Status>(_status));
}


AllJoynWinRTComponent::AJ_Status AllJoynWinRTComponent::AllJoyn::AJ_BusSetSignalRule(AJ_BusAttachment^ bus, String^ ruleString, uint8_t rule)
{
	WCS2MBS(ruleString);
	::AJ_Status _status = ::AJ_BusSetSignalRule(bus->_bus, _ruleString, rule);

	return (static_cast<AJ_Status>(_status));
}


void AllJoynWinRTComponent::AllJoyn::AJ_Disconnect(AJ_BusAttachment^ bus)
{
	::AJ_Disconnect(bus->_bus);
}


AllJoynWinRTComponent::AJ_Status AllJoynWinRTComponent::AllJoyn::AJ_BusJoinSession(AJ_BusAttachment^ bus, String^ sessionHost, uint16_t port, AJ_SessionOpts^ opts)
{
	WCS2MBS(sessionHost);
	::AJ_SessionOpts* _opts = NULL;

	if (opts)
	{
		SAFE_DEL(_s_cachedSessionOpts);
		_opts = new ::AJ_SessionOpts();
		ZeroMemory(_opts, sizeof(_opts));

		STRUCT_COPY(opts, isMultipoint);
		STRUCT_COPY(opts, proximity);
		STRUCT_COPY(opts, traffic);
		STRUCT_COPY(opts, transports);

		_s_cachedSessionOpts = _opts;
	}

	::AJ_Status _status = ::AJ_BusJoinSession(bus->_bus, _sessionHost, port, _opts);

	return (static_cast<AJ_Status>(_status));
}


AllJoynWinRTComponent::AJ_Status AllJoynWinRTComponent::AllJoyn::AJ_BusLeaveSession(AJ_BusAttachment^ bus, uint32_t sessionId)
{
	::AJ_Status _status = ::AJ_BusLeaveSession(bus->_bus, sessionId);

	return (static_cast<AJ_Status>(_status));
}


AllJoynWinRTComponent::AJ_Status AllJoynWinRTComponent::AllJoyn::AJ_MarshalSignal(AJ_BusAttachment^ bus, AJ_Message^ msg, uint32_t msgId, String^ destination, AJ_SessionId sessionId, uint8_t flags, uint32_t ttl)
{
	if (msg->_msg)
	{
		SAFE_DEL(msg->_msg->destination);
	}

	SAFE_DEL(msg->_msg);

	msg->_msg = new ::AJ_Message();
	char* _destination = new char[MAX_STR_LENGTH];
	wcstombs(_destination, destination->Data(), MAX_STR_LENGTH);

	::AJ_Status _status = ::AJ_MarshalSignal(bus->_bus, msg->_msg, msgId, _destination, sessionId, flags, ttl);

	return (static_cast<AJ_Status>(_status));
}


Array<Object^>^ AllJoynWinRTComponent::AllJoyn::AJ_UnmarshalArgs(AJ_Message^ msg, String^ signature)
{
	if (signature->Length() == 0)
	{
		return nullptr;
	}

	Array<Object^>^ args = ref new Array<Object^>(signature->Length() + 1);
	::AJ_Status _status = ::AJ_Status::AJ_ERR_INVALID;

	WCS2MBS(signature);

	for (int i = 0; i < signature->Length(); i++)
	{
		::AJ_Arg arg;
		uint8_t typeId = (uint8_t)_signature[i];
		_status = ::AJ_UnmarshalArg(msg->_msg, &arg);
		args[i + 1] = nullptr;

		if (_status != AJ_OK)
		{
			break;
		}

		switch (_signature[i])
		{
			/**< AllJoyn 64-bit unsigned integer basic type */
		case 't':
			args[i + 1] = static_cast<uint64_t>(*arg.val.v_uint64);
			break;

			/**< AllJoyn 32-bit unsigned integer basic type */
		case 'u':
			args[i + 1] = static_cast<uint32_t>(*arg.val.v_uint32);
			break;

			/**< AllJoyn 16-bit unsigned integer basic type */
		case 'q':
			args[i + 1] = static_cast<uint16_t>(*arg.val.v_uint16);
			break;

			/**< AllJoyn 8-bit unsigned integer basic type */
		case 'y':
			args[i + 1] = static_cast<uint8_t>(*arg.val.v_byte);
			break;

			/**< AllJoyn UTF-8 NULL terminated string basic type */
		case 's':
			String^ val = AJ_CharsToString(arg.val.v_string);
			args[i + 1] = val;
			break;
		}

		if (args[i + 1] == nullptr)
		{
			_status = ::AJ_Status::AJ_ERR_INVALID;
			break;
		}
	}

	args[0] = static_cast<AJ_Status>(_status);

	return args;
}


void AllJoynWinRTComponent::AllJoyn::AJ_BusSetPasswordCallback(AJ_BusAttachment^ bus, AJ_AuthPwdFunc^ pwdCallback)
{
	::AJ_BusSetPasswordCallback(bus->_bus, pwdCallback);
}


char _peerBusName[MAX_STR_LENGTH];

AllJoynWinRTComponent::AJ_Status AllJoynWinRTComponent::AllJoyn::AJ_BusAuthenticatePeer(AJ_BusAttachment^ bus, String^ peerBusName, AJ_PeerAuthenticateCallback^ pwdCallback)
{
	wcstombs(_peerBusName, peerBusName->Data(), MAX_STR_LENGTH);
	AJ_Status status = static_cast<AJ_Status>(::AJ_BusAuthenticatePeer(bus->_bus, _peerBusName, pwdCallback));
	return status;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Helper functions
//////////////////////////////////////////////////////////////////////////////////////////

uint32_t AllJoynWinRTComponent::AllJoyn::Get_AJ_Message_msgId(AJ_Message^ msg)
{
	return msg->_msg->msgId;
}


String^ AllJoynWinRTComponent::AllJoyn::Get_AJ_Arg_v_string(AJ_Arg^ arg)
{
	return AJ_CharsToString(arg->_arg->val.v_string);
}


uint32_t AllJoynWinRTComponent::AllJoyn::AJ_Description_ID(uint32_t o, uint32_t i, uint32_t m, uint32_t a)
{
	return (((uint32_t)(o) << 24) | (((uint32_t)(i)) << 16) | (((uint32_t)(m)) << 8) | (a));
}


uint32_t AllJoynWinRTComponent::AllJoyn::AJ_Encode_Message_ID(uint32_t o, uint32_t p, uint32_t i, uint32_t m)
{
	return (((uint32_t)(o) << 24) | (((uint32_t)(p)) << 16) | (((uint32_t)(i)) << 8) | (m));
}


uint32_t AllJoynWinRTComponent::AllJoyn::AJ_Encode_Property_ID(uint32_t o, uint32_t p, uint32_t i, uint32_t m)
{
	return (((uint32_t)(o) << 24) | (((uint32_t)(p)) << 16) | (((uint32_t)(i)) << 8) | (m));
}


uint32_t AllJoynWinRTComponent::AllJoyn::AJ_Bus_Message_ID(uint32_t p, uint32_t i, uint32_t m)
{
	return (((uint32_t)(AllJoynWinRTComponent::AJ_Introspect::AJ_Bus_ID_Flag) << 24) | (((uint32_t)(p)) << 16) | (((uint32_t)(i)) << 8) | (m));
}


uint32_t AllJoynWinRTComponent::AllJoyn::AJ_App_Message_ID(uint32_t p, uint32_t i, uint32_t m)
{
	return (((uint32_t)(AllJoynWinRTComponent::AJ_Introspect::AJ_App_ID_Flag) << 24) | (((uint32_t)(p)) << 16) | (((uint32_t)(i)) << 8) | (m));
}


uint32_t AllJoynWinRTComponent::AllJoyn::AJ_Prx_Message_ID(uint32_t p, uint32_t i, uint32_t m)
{
	return (((uint32_t)(AllJoynWinRTComponent::AJ_Introspect::AJ_Prx_ID_Flag) << 24) | (((uint32_t)(p)) << 16) | (((uint32_t)(i)) << 8) | (m));
}


uint32_t AllJoynWinRTComponent::AllJoyn::AJ_Bus_Property_ID(uint32_t p, uint32_t i, uint32_t m)
{
	return (((uint32_t)(AllJoynWinRTComponent::AJ_Introspect::AJ_Bus_ID_Flag) << 24) | (((uint32_t)(p)) << 16) | (((uint32_t)(i)) << 8) | (m));
}


uint32_t AllJoynWinRTComponent::AllJoyn::AJ_App_Property_ID(uint32_t p, uint32_t i, uint32_t m)
{
	return (((uint32_t)(AllJoynWinRTComponent::AJ_Introspect::AJ_App_ID_Flag) << 24) | (((uint32_t)(p)) << 16) | (((uint32_t)(i)) << 8) | (m));
}


uint32_t AllJoynWinRTComponent::AllJoyn::AJ_Prx_Property_ID(uint32_t p, uint32_t i, uint32_t m)
{
	return (((uint32_t)(AllJoynWinRTComponent::AJ_Introspect::AJ_Prx_ID_Flag) << 24) | (((uint32_t)(p)) << 16) | (((uint32_t)(i)) << 8) | (m));
}


uint32_t AllJoynWinRTComponent::AllJoyn::AJ_Reply_ID(uint32_t id)
{
	return ((id) | (uint32_t)((uint32_t)(AllJoynWinRTComponent::AJ_Introspect::AJ_Rep_ID_Flag) << 24));
}
