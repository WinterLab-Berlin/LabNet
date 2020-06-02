#pragma once

#include <../Interface/Interfaces.h>
#include <map>
#include <so_5/all.hpp>

namespace DigitalOut
{
	struct PinId
	{
		Interface::Interfaces interface;
		unsigned int pin;
		
		bool operator <(const PinId& a) const
		{
			return std::tie(interface, pin) < std::tie(a.interface, a.pin); 
		}
	};
}