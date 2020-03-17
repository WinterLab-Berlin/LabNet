#pragma once

namespace GPIO
{
	class DigitalInput
	{
	public:
		DigitalInput(const char h, const char l, bool inverted)
			: pin_h(h)
			, pin_l(l)
			, is_inverted(inverted)
			, state(false)
		{
		}
		
		const char pin_h;
		const char pin_l;
		const bool is_inverted;
		bool state;
	};
}