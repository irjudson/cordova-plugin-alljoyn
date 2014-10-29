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

BusAttachment::BusAttachment(String^ applicationName, Boolean allowRemoteMessages, uint32_t concurrency)
{
	_busAttachment = NULL;
	_interfaceDescription = NULL;

	wstring w_appName(applicationName->Begin());
	string s_appName(w_appName.begin(), w_appName.end());

	_busAttachment = new ajn::BusAttachment(s_appName.c_str(), allowRemoteMessages, concurrency);
}

String^ BusAttachment::CreateInterface(String^ name, Boolean secure)
{
	wstring w_name(name->Begin());
	string s_name(w_name.begin(), w_name.end());
	printf("Interface '%s' created.\n", s_name);

	QStatus status = _busAttachment->CreateInterface(s_name.c_str(), _interfaceDescription, secure);

	return MultibyteToPlatformString(QCC_StatusText(status));
}