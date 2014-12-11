#include "pch.h"

#include "AllJoynWinRTComponent.h"

#include "aj_init.h"
#include "aj_util.h"
#include "aj_target_util.h"
#include "aj_helper.h"

#include <ppltasks.h>

using namespace concurrency;

using namespace Platform;
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

#define MAX_STR_LENGTH 1024

#define WCS2MBS(string)														\
	char __ ## string[MAX_STR_LENGTH];										\
	wcstombs(__ ## string, string->Data(), strlen(__ ## string));			\
	char* _ ## string = (string == nullptr) ? NULL : __ ## string			\


AllJoynWinRTComponent::AllJoyn::AllJoyn()
{
}


AllJoynWinRTComponent::AllJoyn::~AllJoyn()
{
	// Phong TODO: free mem
}


void AllJoynWinRTComponent::AllJoyn::AJ_Initialize()
{
	::AJ_Initialize();
}


::AJ_Object* AllJoynWinRTComponent::AllJoyn::RegisterObject(const Array<AJ_Object^>^ objects)
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
					if (objects[i]->interfaces->GetAt(j))
					{
						int nEntries = objects[i]->interfaces->GetAt(j)->Size;
						interfaces[j] = new char*[nEntries + 1];
						ZeroMemory(interfaces[j], sizeof(interfaces[j]));
						for (int k = 0; k < nEntries; k++)
						{
							char* entry = new char[MAX_STR_LENGTH];
							AJ_StringToChars(objects[i]->interfaces->GetAt(j)->GetAt(k), entry);
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


void AllJoynWinRTComponent::AllJoyn::AJ_RegisterObjects(const Array<AJ_Object^>^ localObjects, const Array<AJ_Object^>^ proxyObjects)
{
	::AJ_Object* _localObjects = AllJoynWinRTComponent::AllJoyn::RegisterObject(localObjects);
	::AJ_Object* _proxyObjects = AllJoynWinRTComponent::AllJoyn::RegisterObject(proxyObjects);
	::AJ_RegisterObjects(_localObjects, _proxyObjects);
}


AllJoynWinRTComponent::AJ_Status AllJoynWinRTComponent::AllJoyn::AJ_StartClient
(
	AllJoynWinRTComponent::AJ_BusAttachment^ bus,
	String^ daemonName,
	uint32_t timeout,
	uint8_t connected,
	String^ name,
	uint16_t port,
	uint32_t* sessionId,
	AllJoynWinRTComponent::AJ_SessionOpts^ opts)
{
	::AJ_BusAttachment _bus;

	WCS2MBS(daemonName);
	WCS2MBS(name);

	::AJ_SessionOpts* _opts = NULL;

	if (opts)
	{
		_opts = new ::AJ_SessionOpts();
		ZeroMemory(_opts, sizeof(_opts));

		STRUCT_COPY(opts, isMultipoint);
		STRUCT_COPY(opts, proximity);
		STRUCT_COPY(opts, traffic);
		STRUCT_COPY(opts, transports);
	}

	::AJ_Status status = ::AJ_StartClient(&_bus, _daemonName, timeout, connected, _name, port, sessionId, _opts);

	return (static_cast<AJ_Status>(status));
}


//////////////////////////////////////////////////////////////////////////////////////////
// Testing
//////////////////////////////////////////////////////////////////////////////////////////

extern int AJ_Main(void);

IAsyncOperation<String^>^ AllJoynWinRTComponent::AllJoyn::Test() {
	return create_async([]() -> String^ 
	{
		AJ_Main();
		return "To be implemented";
	});
}


IAsyncOperation<String^>^ AllJoynWinRTComponent::AllJoyn::GetVersion() {
	return create_async([]() -> String^ {
		return AJ_CharsToString(AJ_GetVersion());
	});
}


IAsyncAction^ AllJoynWinRTComponent::AllJoyn::Initialize() {
	return create_async([]() {
		AJ_Initialize();
	});
}