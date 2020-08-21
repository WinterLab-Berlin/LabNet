#pragma once

namespace gpio_wiring
{
	enum resistor
	{
		off       = 0,
		pull_down = 1,
		pull_up   = 2
	};
	
	struct DigitalInput
	{
		/// hardware pin number (wiringPi notation)
		char pin;
		/// has the pin value to be inverted
		bool is_inverted {false};
		/// current pin state
		char state {2};
		resistor resistor_state {resistor::off};
	};
}