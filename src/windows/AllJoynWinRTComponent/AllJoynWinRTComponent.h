#pragma once

#include <ppltasks.h>

using namespace Platform;
using namespace Windows::Foundation;

namespace AllJoynWinRTComponent
{
    public ref class AllJoyn sealed
    {
    public:
		static IAsyncOperation<String^>^ Test();
		static IAsyncOperation<String^>^ GetVersion();
		static IAsyncAction^ Initialize();
    };
}