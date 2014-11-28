#include "pch.h"
#include "AllJoynWinRTComponent.h"

#include "aj_init.h"
#include "aj_util.h"
#include "aj_target_util.h"

#include <ppltasks.h>

using namespace concurrency;

using namespace AllJoynWinRTComponent;
using namespace Platform;
using namespace Windows::Foundation;


IAsyncOperation<String^>^ AllJoyn::Test() {
	return create_async([]() -> String^ 
	{
		return "To be implemented";
	});
}

IAsyncOperation<String^>^ AllJoyn::GetVersion() {
	return create_async([]() -> String^ {
		return AJ_CharsToString(AJ_GetVersion());
	});
}

IAsyncAction^ AllJoyn::Initialize() {
	return create_async([]() {
		AJ_Initialize();
	});
}