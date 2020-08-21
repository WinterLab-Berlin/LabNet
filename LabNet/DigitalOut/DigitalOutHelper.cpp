#include "DigitalOutHelper.h"
#include "../../Network/ProtocolAll.h"
#include "../InterfaceMessages.h"
#include "PulseHelper.h"
#include "LoopHelper.h"
#include "../io_board/Messages.h"
#include "LoopMessages.h"
#include "../Interface/DigitalMessages.h"

DigitalOut::DigitalOutHelper::DigitalOutHelper(context_t ctx, Logger logger, so_5::mbox_t selfBox, so_5::mbox_t lab_net_box)
	: so_5::agent_t(ctx)
	, _selfBox(selfBox)
	, _gpioBox(ctx.environment().create_mbox("gpio"))
	, _uartBox(ctx.environment().create_mbox("uart"))
	, _gpioWiringBox(ctx.environment().create_mbox("gpioWiring"))
	, _labNetBox(lab_net_box)
	, _logger(logger)
{
	
}

DigitalOut::DigitalOutHelper::~DigitalOutHelper()
{
}

void DigitalOut::DigitalOutHelper::so_define_agent()
{
	this >>= _runningState;
	
	_runningState
		.event(_selfBox,
		[this](mhood_t<Interface::pause_interface> msg) {
			for (auto& loop : _loopHelper)
			{
				so_5::send<pause_loop_helper>(loop.second);
			}
				
			for (auto& out : _pulseHelper)
			{
				so_5::send<stop_helper>(out.second);
			}
			_pulseHelper.clear();
				
			this >>= _pausedState;
		})
		.event(_selfBox,
		[this](mhood_t<Interface::reset_interface> msg) {
			for (auto& loop : _loopHelper)
			{
				so_5::send<abort_loop_helper>(loop.second);
			}
			_loopHelper.clear();
				
			for (auto& pulse : _pulseHelper)
			{
				so_5::send<stop_helper>(pulse.second);
			}
			_pulseHelper.clear();
		})
		.event(_selfBox,
		[this](mhood_t<LabNetProt::Client::DigitalOutSet> msg) {
			auto interface = msg->id().interface();
			
			if (interface == LabNetProt::INTERFACE_IO_BOARD)
			{
				PinId id{Interface::IO_BOARD, msg->id().pin()}
				;
				
				if (_pulseHelper.count(id) > 0)
				{
					so_5::send<just_switch>(_pulseHelper[id], msg->state());
				}
				else
				{
					so_5::send<DigitalMessages::set_digital_out>(_gpioBox, Interface::IO_BOARD, msg->id().pin(), msg->state(), _labNetBox);
				}
			}
			else if (interface == LabNetProt::INTERFACE_GPIO_WIRINGPI)
			{
				PinId id{Interface::GPIO_WIRING, msg->id().pin()};
				
				if (_pulseHelper.count(id) > 0)
				{
					so_5::send<just_switch>(_pulseHelper[id], msg->state());
				}
				else
				{
					so_5::send<DigitalMessages::set_digital_out>(_gpioWiringBox, Interface::GPIO_WIRING, msg->id().pin(), msg->state(), _labNetBox);
				}
			}
			else if (interface == LabNetProt::INTERFACE_UART0)
			{
				PinId id{Interface::UART0, msg->id().pin()};
				
				if (_pulseHelper.count(id) > 0)
				{
					so_5::send<just_switch>(_pulseHelper[id], msg->state());
				}
				else
				{
					so_5::send<DigitalMessages::set_digital_out>(_uartBox, Interface::UART0, msg->id().pin(), msg->state(), _labNetBox);
				}
			}
			else if (interface == LabNetProt::INTERFACE_UART1)
			{
				PinId id{Interface::UART1, msg->id().pin()};
				
				if (_pulseHelper.count(id) > 0)
				{
					so_5::send<just_switch>(_pulseHelper[id], msg->state());
				}
				else
				{
					so_5::send<DigitalMessages::set_digital_out>(_uartBox, Interface::UART1, msg->id().pin(), msg->state(), _labNetBox);
				}
			}
			else if (interface == LabNetProt::INTERFACE_UART2)
			{
				PinId id{Interface::UART2, msg->id().pin()};
				
				if (_pulseHelper.count(id) > 0)
				{
					so_5::send<just_switch>(_pulseHelper[id], msg->state());
				}
				else
				{
					so_5::send<DigitalMessages::set_digital_out>(_uartBox, Interface::UART2, msg->id().pin(), msg->state(), _labNetBox);
				}
			}
			else if (interface == LabNetProt::INTERFACE_UART3)
			{
				PinId id{Interface::UART3, msg->id().pin()};
				
				if (_pulseHelper.count(id) > 0)
				{
					so_5::send<just_switch>(_pulseHelper[id], msg->state());
				}
				else
				{
					so_5::send<DigitalMessages::set_digital_out>(_uartBox, Interface::UART3, msg->id().pin(), msg->state(), _labNetBox);
				}
			}
			else if (interface == LabNetProt::INTERFACE_UART4)
			{
				PinId id{Interface::UART4, msg->id().pin()};
				
				if (_pulseHelper.count(id) > 0)
				{
					so_5::send<just_switch>(_pulseHelper[id], msg->state());
				}
				else
				{
					so_5::send<DigitalMessages::set_digital_out>(_uartBox, Interface::UART4, msg->id().pin(), msg->state(), _labNetBox);
				}
			}
		})
		.event(_selfBox,
		[this](mhood_t<LabNetProt::Client::DigitalOutPulse> msg) {
			auto interface = msg->id().interface();
			if (interface == LabNetProt::INTERFACE_IO_BOARD)
			{
				PinId id{Interface::IO_BOARD, msg->id().pin()}
				;
				
				if (_pulseHelper.count(id) == 0)
				{
					auto coop = so_5::create_child_coop(*this);
					auto a = coop->make_agent<PulseHelper>(_logger, _selfBox, _labNetBox, _gpioBox, id.interface, id.pin);
					
					_pulseHelper[id] = a->so_direct_mbox();
					
					so_environment().register_coop(std::move(coop));
				}
				
				so_5::send<start_pulse>(_pulseHelper[id], msg->high_duration(), msg->low_duration(), msg->pulses());
			}
			else if (interface == LabNetProt::INTERFACE_GPIO_WIRINGPI)
			{
				PinId id{Interface::GPIO_WIRING, msg->id().pin()};
				
				if (_pulseHelper.count(id) == 0)
				{
					auto coop = so_5::create_child_coop(*this);
					auto a = coop->make_agent<PulseHelper>(_logger, _selfBox, _labNetBox, _gpioWiringBox, id.interface, id.pin);
					
					_pulseHelper[id] = a->so_direct_mbox();
					
					so_environment().register_coop(std::move(coop));
				}
				
				so_5::send<start_pulse>(_pulseHelper[id], msg->high_duration(), msg->low_duration(), msg->pulses());
			}
			else if (interface == LabNetProt::INTERFACE_UART0
				|| interface == LabNetProt::INTERFACE_UART1
				|| interface == LabNetProt::INTERFACE_UART2
				|| interface == LabNetProt::INTERFACE_UART3
				|| interface == LabNetProt::INTERFACE_UART4)
			{
				PinId id{static_cast<Interface::Interfaces>(interface), msg->id().pin()};
				
				if (_pulseHelper.count(id) == 0)
				{
					auto coop = so_5::create_child_coop(*this);
					auto a = coop->make_agent<PulseHelper>(_logger, _selfBox, _labNetBox, _uartBox, id.interface, id.pin);
					
					_pulseHelper[id] = a->so_direct_mbox();
					
					so_environment().register_coop(std::move(coop));
				}
				
				so_5::send<start_pulse>(_pulseHelper[id], msg->high_duration(), msg->low_duration(), msg->pulses());
			}
		})
		.event(_selfBox,
		[this](mhood_t<LabNetProt::Client::StartDigitalOutLoop> msg) {
			std::string loopName = msg->loop_name();
			
			if (loopName.size() > 0)
			{
				if (_loopHelper.count(loopName) != 0)
				{
					so_5::send<abort_loop_helper>(_loopHelper[loopName]);
					_loopHelper.erase(loopName);
				}
			
				if (msg->digital_outputs_size() > 0)
				{
					auto coop = so_5::create_child_coop(*this);
					auto a = coop->make_agent<LoopHelper>(_logger, _selfBox, _labNetBox);
					_loopHelper[loopName] = a->so_direct_mbox();
					so_environment().register_coop(std::move(coop));
			
					so_5::send<LabNetProt::Client::StartDigitalOutLoop>(_loopHelper[loopName], *msg);	
				}
				else
				{
					so_5::send<loop_start_failed>(_labNetBox, loopName);
				}
			}
		})
		.event(_selfBox,
		[this](mhood_t<LabNetProt::Client::StopDigitalOutLoop> msg) {
			auto loopName = msg->loop_name();
			
			if (_loopHelper.count(loopName) == 0)
			{
				
			}
			else
			{
				so_5::send<LabNetProt::Client::StopDigitalOutLoop>(_loopHelper[loopName], *msg);	
				_loopHelper.erase(loopName);
			}
		});
	
	_pausedState
		.event(_selfBox,
		[this](mhood_t<Interface::continue_interface> msg)
		{
			this >>= _runningState;
			
			for (auto& loop : _loopHelper)
			{
				so_5::send<continue_loop_helper>(loop.second);
			}
		});
}