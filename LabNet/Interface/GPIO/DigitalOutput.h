#pragma once

namespace GPIO
{
	class DigitalOutput
	{
	public:
		DigitalOutput(const char h, const char l, bool inverted)
			: pin_h(h)
			, pin_l(l)
			, is_inverted(inverted)
		{
		}
		
		const char pin_h;
		const char pin_l;
		const bool is_inverted;
	};
}