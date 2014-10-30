#include "pch.h"
#include "BusAttachment.h"

using namespace AllJoynWinRTComponent;
using namespace Platform;
using namespace Platform::Collections;
using namespace std;
using namespace Windows::Foundation::Collections;

Platform::String^ MultibyteToPlatformString(const char* str)
{
	auto s_str = std::string(str);

	std::wstring w_str;
	w_str.assign(s_str.begin(), s_str.end());

	return ref new Platform::String(w_str.c_str());
}

BusAttachment::BusAttachment(String^ applicationName, Boolean allowRemoteMessages)
{
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
		printf("BusAttachment::CreateInterface - '%s' created.\n", s_name);

		_interfaceDescription->AddMethod("cat", "ss", "s", "inStr1,inStr2,outStr", 0);
		_interfaceDescription->Activate();
	}
	else {
		printf("BusAttachment::CreateInterface - Failed. '%s'.\n", QCC_StatusText(status));
	}

	return MultibyteToPlatformString(QCC_StatusText(status));
}

String^ BusAttachment::Start()
{
	QStatus status = _busAttachment->Start();

	if (ER_OK == status) {
		printf("BusAttachment::Start - Started.\n");
	}
	else {
		printf("BusAttachment::Start - Failed. '%s'.\n", QCC_StatusText(status));;
	}

	return MultibyteToPlatformString(QCC_StatusText(status));
}

String^	BusAttachment::Connect(void)
{
	const char* connectArgs = "tcp:addr=127.0.0.1,port=9955";

	QStatus status = _busAttachment->Connect(connectArgs);

	if (ER_OK == status) {
		printf("BusAttachment::Connect connected to '%s'.\n", _busAttachment->GetConnectSpec().c_str());
	}
	else {
		printf("BusAttachment::Start - Failed. '%s'. Connection Specs: '%s'.\n", QCC_StatusText(status), _busAttachment->GetConnectSpec().c_str());
	}

	return MultibyteToPlatformString(QCC_StatusText(status));
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
		printf("org.alljoyn.Bus.FindAdvertisedName ('%s') succeeded.\n", s_serviceName.c_str());
	}
	else {
		printf("org.alljoyn.Bus.FindAdvertisedName ('%s') failed (%s).\n", s_serviceName.c_str(), QCC_StatusText(status));
	}

	return MultibyteToPlatformString(QCC_StatusText(status));
}
