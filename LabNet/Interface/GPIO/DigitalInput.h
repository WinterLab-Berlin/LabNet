#pragma once

namespace GPIO
{
	enum resistor
	{
		off = 0,
		pull_down = 1,
		pull_up = 2
	};
	
	struct DigitalInput
	{
		/// hardware pin number (wiringPi notation)
		char pin_h;
		/// logic pin number
		char pin_l;
		/// has the pin value to be inverted
		bool is_inverted {false};
		/// 
		bool available {false};
		/// current pin state
		char state {2};
		resistor resistor_state {resistor::off};
	};
}