#include "PulseHelper.h"
#include "../io_board/Messages.h"
#include "../Interface/DigitalMessages.h"

struct turn_on final : public so_5::signal_t {};
struct turn_off final : public so_5::signal_t {};

DigitalOut::PulseHelper::PulseHelper(context_t ctx,
	Logger logger,
	so_5::mbox_t dig_out_box,
	so_5::mbox_t lab_net_box,
	so_5::mbox_t interface_box,
	Interface::Interfaces interface,
	char pin)
	: so_5::agent_t(ctx)
	, _logger(logger)
	, _digOutBox(dig_out_box)
	, _labNetBox(lab_net_box)
	, _interfaceBox(interface_box)
	, _interface(interface)
	, _pin(pin)
{
}

void DigitalOut::PulseHelper::so_define_agent()
{
	this >>= _waitState;
	
	_waitState
		.event([this](mhood_t<start_pulse> mes)
		{
			_highDuration = mes->high_duration;
			_lowDuration = mes->low_duration;
			_pulses = mes->pulses;
			
			so_5::send<DigitalMessages::set_digital_out>(_interfaceBox, _interface, _pin, true, so_direct_mbox());
			this >>= _startingState;
		})
		.event([this](mhood_t<stop_helper> mes)
		{
			so_5::send<DigitalMessages::set_digital_out>(_interfaceBox, _interface, _pin, false, _labNetBox);
			so_deregister_agent_coop_normally();
		})
		.event([this](mhood_t<just_switch> mes)
		{
			so_5::send<DigitalMessages::set_digital_out>(_interfaceBox, _interface, _pin, mes->state, _labNetBox);
		});
	
	_startingState
		.event([this](mhood_t<DigitalMessages::return_digital_out_state> mes)
		{
			this >>= _runningState;
			
			so_5::send<DigitalMessages::return_digital_out_state>(_labNetBox, _interface, mes->pin, mes->state, std::chrono::high_resolution_clock::now());
			
			_turnOffTimer = so_5::send_periodic<turn_off>(
				so_direct_mbox(),
				std::chrono::milliseconds(_highDuration),
				std::chrono::milliseconds(_highDuration + _lowDuration));
			
			_turnOnTimer = so_5::send_periodic<turn_on>(
				so_direct_mbox(),
				std::chrono::milliseconds(_highDuration + _lowDuration),
				std::chrono::milliseconds(_highDuration + _lowDuration));
		})
		.event([this](mhood_t<DigitalMessages::invalid_digital_out_pin> mes)
		{
			so_5::send<DigitalMessages::return_digital_out_state>(_labNetBox, _interface, mes->pin, false, std::chrono::high_resolution_clock::now());
		})
		.event([this](mhood_t<stop_helper> mes)
		{
			so_5::send<DigitalMessages::set_digital_out>(_interfaceBox, _interface, _pin, false, _labNetBox);
			so_deregister_agent_coop_normally();
		}).event([this](mhood_t<just_switch> mes)
		{
			so_5::send<DigitalMessages::set_digital_out>(_interfaceBox, _interface, _pin, mes->state, _labNetBox);
			this >>= _waitState;
		});
	
	_runningState
		.event([this](mhood_t<turn_off> mes)
		{
			if (_pulses < 255)
				_pulses--;
			
			if (_pulses <= 0)
			{
				so_5::send<DigitalMessages::set_digital_out>(_interfaceBox, _interface, _pin, false, _labNetBox);
				
				_turnOffTimer.release();
				_turnOnTimer.release();
			}
			else
			{
				so_5::send<DigitalMessages::set_digital_out>(_interfaceBox, _interface, _pin, false, so_direct_mbox());
			}
		})
		.event([this](mhood_t<turn_on> mes)
		{
			so_5::send<DigitalMessages::set_digital_out>(_interfaceBox, _interface, _pin, true, so_direct_mbox());
		})
		.event([this](mhood_t<stop_helper> mes)
		{
			so_5::send<DigitalMessages::set_digital_out>(_interfaceBox, _interface, _pin, false, _labNetBox);
			_turnOffTimer.release();
			_turnOnTimer.release();
			so_deregister_agent_coop_normally();
		})
		.event([this](mhood_t<start_pulse> mes)
		{
			_turnOffTimer.release();
			_turnOnTimer.release();
			
			_highDuration = mes->high_duration;
			_lowDuration = mes->low_duration;
			_pulses = mes->pulses;
			
			so_5::send<DigitalMessages::set_digital_out>(_interfaceBox, _interface, _pin, true, so_direct_mbox());
			this >>= _startingState;
		}).event([this](mhood_t<just_switch> mes)
		{
			so_5::send<DigitalMessages::set_digital_out>(_interfaceBox, _interface, _pin, mes->state, _labNetBox);
			_turnOffTimer.release();
			_turnOnTimer.release();
			
			this >>= _waitState;
		});;
}