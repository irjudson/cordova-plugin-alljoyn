#include "pch.h"

#pragma once

using namespace Platform;

namespace AllJoynWinRTComponent
{
	public ref class BusAttachment sealed
	{
	public:
		BusAttachment(String^ applicationName, Boolean allowRemoteMessages, uint32_t concurrency);
		String^ CreateInterface(String^ name, Boolean secure);
	private:
		ajn::BusAttachment* _busAttachment;
		ajn::InterfaceDescription* _interfaceDescription;
	};
}