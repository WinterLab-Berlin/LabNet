#pragma once

namespace GPIO
{
	struct DigitalOutput
	{
		char pin_h;
		char pin_l;
		bool is_inverted {false};
		bool available {false};
	};
}