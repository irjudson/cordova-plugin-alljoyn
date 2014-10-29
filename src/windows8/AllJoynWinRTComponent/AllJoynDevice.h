#include "pch.h"

#pragma once

using namespace Platform;

namespace AllJoynWinRTComponent
{
	public ref class AllJoynDevice sealed
	{
	public:
		property String^ InterfaceName;
		property String^ ObjectName;
		property String^ ObjectPath;
		property String^ Signature;
		property int PortNumber;

	};
}