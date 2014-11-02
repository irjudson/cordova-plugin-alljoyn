#include "pch.h"
#include "BusAttachment.h"
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace AllJoynWinRTComponent;
using namespace Platform;
using namespace Platform::Collections;
using namespace std;
using namespace Windows::Foundation::Collections;

const char* PlatformStringToMultibyte(String^ str)
{
	wstring w_str(str->Begin());
	string s_str(w_str.begin(), w_str.end());

	char* temp = (char*)malloc(256 * sizeof(char));
	strcpy(temp, s_str.c_str());
	return temp;
}

Platform::String^ MultibyteToPlatformString(const char* str)
{
	auto s_str = std::string(str);
	std::wstring w_str;
	w_str.assign(s_str.begin(), s_str.end());

	return ref new Platform::String(w_str.c_str());
}

 void Log(const char* str, ...)
 {
	 char buf[2048];

	 va_list ptr;
	 va_start(ptr, str);
	 vsprintf(buf, str, ptr);

	 auto s_str = std::string(buf);

	 std::wstring w_str;
	 w_str.assign(s_str.begin(), s_str.end());

	 OutputDebugString(w_str.c_str());
 }


BusAttachment::BusAttachment(String^ applicationName, Boolean allowRemoteMessages)
{
	Log("AllJoyn Library:\n");
	Log("	version: %s.\n", ajn::GetVersion());
	Log("	build info: %s.\n", ajn::GetBuildInfo());

	QStatus status = ER_OK;

	/* Create message bus. */
	ajn::BusAttachment* g_msgBus = new ajn::BusAttachment("myApp", true);

	/* This test for NULL is only required if new() behavior is to return NULL
	* instead of throwing an exception upon an out of memory failure.
	*/
	if (!g_msgBus) {
		status = ER_OUT_OF_MEMORY;
	}

	if (ER_OK == status) {
		/* Add org.alljoyn.Bus.method_sample interface */
		ajn::InterfaceDescription* testIntf = NULL;
		status = g_msgBus->CreateInterface("org.alljoyn.Bus.sample", testIntf);
	}

	if (ER_OK == status) {
		status = g_msgBus->Start();
	}

	if (ER_OK == status) {
		status = g_msgBus->Connect();
	}

	_busAttachment = NULL;
	_interfaceDescription = NULL;
	_busAttachment = NULL;

	wstring w_appName(applicationName->Begin());
	string s_appName(w_appName.begin(), w_appName.end());

	_busAttachment = new ajn::BusAttachment(s_appName.c_str(), allowRemoteMessages);
}

String^ BusAttachment::CreateInterface(String^ name)
{
	wstring w_name(name->Begin());
	string s_name(w_name.begin(), w_name.end());

	QStatus status = _busAttachment->CreateInterface(s_name.c_str(), _interfaceDescription);

	if (status == ER_OK) {
		Log("BusAttachment::CreateInterface - '%s' created.\n", PlatformStringToMultibyte(name));

		_interfaceDescription->AddMethod("cat", "ss", "s", "inStr1,inStr2,outStr", 0);
		_interfaceDescription->Activate();
	}
	else {
		Log("BusAttachment::CreateInterface - Failed. '%s'.\n", QCC_StatusText(status));
	}

	return MultibyteToPlatformString(QCC_StatusText(status));
}

String^ BusAttachment::Start()
{
	QStatus status = _busAttachment->Start();

	if (ER_OK == status) {
		Log("BusAttachment::Start - Started.\n");
	}
	else {
		Log("BusAttachment::Start - Failed. '%s'.\n", QCC_StatusText(status));;
	}

	return MultibyteToPlatformString(QCC_StatusText(status));
}

String^	BusAttachment::Connect(void)
{
	QStatus status = _busAttachment->Connect();

	if (ER_OK == status) {
		Log("BusAttachment::Connect connected to '%s'.\n", _busAttachment->GetConnectSpec().c_str());
	}
	else {
		Log("BusAttachment::Start - Failed. '%s'. Connection Specs: '%s'.\n", QCC_StatusText(status), _busAttachment->GetConnectSpec().c_str());
	}

	return MultibyteToPlatformString(_busAttachment->GetConnectSpec().c_str());
}

void BusAttachment::RegisterBusListener(void)
{
	_busAttachment->RegisterBusListener(_busListener);
	printf("BusListener Registered.\n");
}

String^ BusAttachment::FindAdvertisedName(String^ serviceName)
{
	wstring w_serviceName(serviceName->Begin());
	string s_serviceName(w_serviceName.begin(), w_serviceName.end());

	QStatus status = _busAttachment->FindAdvertisedName(s_serviceName.c_str());

	if (status == ER_OK) {
		Log("org.alljoyn.Bus.FindAdvertisedName ('%s') succeeded.\n", s_serviceName.c_str());
	}
	else {
		Log("org.alljoyn.Bus.FindAdvertisedName ('%s') failed (%s).\n", s_serviceName.c_str(), QCC_StatusText(status));
	}

	return MultibyteToPlatformString(QCC_StatusText(status));
}
