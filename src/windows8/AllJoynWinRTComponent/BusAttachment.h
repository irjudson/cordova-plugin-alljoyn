#include "pch.h"

#pragma once

using namespace Platform;

namespace AllJoynWinRTComponent
{
	public ref class BusAttachment sealed
	{
	public:
		BusAttachment(String^ applicationName, Boolean allowRemoteMessages);
		String^ BusAttachment::CreateInterface(String^ name);
		String^ BusAttachment::Start();
		String^ BusAttachment::Connect();
		void BusAttachment::RegisterBusListener();
		String^ BusAttachment::FindAdvertisedName(String^ serviceName);
	private:
		ajn::BusAttachment* _busAttachment;
		ajn::InterfaceDescription* _interfaceDescription;
		ajn::BusListener _busListener;
	};
}