#include "GpioManager.h"
#include "../DigitalMessages.h"
#include "../InterfaceMessages.h"
#include "../../Network/LabNet.pb.h"
#include "../../Network/LabNetClient.pb.h"
#include <chrono>
#include <wiringPi.h>

using namespace gpio_wiring;

enum pin_type
{
    None = 0,
    Out = 1,
    In = 2
};

GpioManager::GpioManager(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, Logger logger)
    : so_5::agent_t(ctx)
    , _self_box(self_box)
    , _interfaces_manager_box(interfaces_manager_box)
    , _events_box(events_box)
    , _logger(logger)
{
    std::vector<char> pin = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 };
    for (int i = 0; i < pin.size(); i++)
    {
        _pins[i] = pin_type::None;
    }
}

GpioManager::~GpioManager()
{
}

void GpioManager::so_evt_finish()
{
    _inputStateReader.reset();

    so_5::send<Interface::interface_stopped>(_interfaces_manager_box, Interface::Interfaces::GPIO_WIRING);
    _logger->writeInfoEntry("gpio wiringPi finished");
}

void GpioManager::so_evt_start()
{
    _logger->writeInfoEntry("gpio wiringPi started");
}

void GpioManager::so_define_agent()
{
    this >>= init_state;

    init_state
        .event(_self_box,
            [this](const mhood_t<init_gpio_wiring>& mes) {
                _reader_box = so_environment().create_mchain(so_5::make_unlimited_mchain_params());
                _inputStateReader = std::make_unique<DigitalInputStateReader>(_events_box, _reader_box, _logger);

                so_5::send<Interface::interface_init_result>(_interfaces_manager_box, Interface::Interfaces::GPIO_WIRING, true);

                this >>= running_state;
            });

    running_state
        .event(_self_box,
            [this](const mhood_t<init_gpio_wiring>& msg) {
                so_5::send<Interface::interface_init_result>(_interfaces_manager_box, Interface::Interfaces::GPIO_WIRING, true);
            })
        .event(_self_box,
            [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalIn> msg) {
                uint32_t pin = msg->pin();
                std::map<char, char>::iterator it = _pins.find(pin);
                if (it != _pins.end() && it->second != pin_type::Out)
                {
                    if (it->second == pin_type::None)
                    {
                        it->second = pin_type::In;

                        so_5::send<DigitalInput>(_reader_box, pin, msg->is_inverted(), 2, static_cast<resistor>(msg->resistor_state()));
                    }

                    so_5::send<DigitalMessages::digital_in_init_result>(_interfaces_manager_box, Interface::GPIO_WIRING, pin, true);
                }
                else
                {
                    so_5::send<DigitalMessages::digital_in_init_result>(_interfaces_manager_box, Interface::GPIO_WIRING, pin, false);
                }
            })
        .event(_self_box,
            [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalOut> msg) {
                uint32_t pin = msg->pin();
                std::map<char, char>::iterator it = _pins.find(pin);
                if (it != _pins.end() && it->second != pin_type::In)
                {
                    if (it->second == pin_type::None)
                    {
                        it->second = pin_type::Out;
                        _outputs[pin] = DigitalOutput { pin, msg->is_inverted() };

                        pinMode(pin, OUTPUT);
                        if (_outputs[pin].is_inverted)
                        {
                            digitalWrite(pin, 1);
                        }
                        else
                        {
                            digitalWrite(pin, 0);
                        }
                    }

                    so_5::send<DigitalMessages::digital_out_init_result>(_interfaces_manager_box, Interface::GPIO_WIRING, pin, true);
                }
                else
                {
                    so_5::send<DigitalMessages::digital_out_init_result>(_interfaces_manager_box, Interface::GPIO_WIRING, pin, false);
                }
            })
        .event(_self_box,
            [this](mhood_t<DigitalMessages::set_digital_out> msg) {
                std::map<int, DigitalOutput>::iterator it = _outputs.find(msg->pin);
                if (it != _outputs.end())
                {
                    if (_outputs[msg->pin].is_inverted)
                    {
                        digitalWrite(_outputs[msg->pin].pin, !msg->state);
                    }
                    else
                    {
                        digitalWrite(_outputs[msg->pin].pin, msg->state);
                    }

                    so_5::send<DigitalMessages::return_digital_out_state>(msg->mbox, Interface::GPIO_WIRING, msg->pin, msg->state, std::chrono::high_resolution_clock::now());
                }
                else
                {
                    so_5::send<DigitalMessages::invalid_digital_out_pin>(msg->mbox, Interface::GPIO_WIRING, msg->pin);
                }
            })
        .event(_self_box,
            [this](mhood_t<Interface::pause_interface> msg) {
                so_5::send<Interface::pause_interface>(_reader_box);

                this >>= paused_state;
            });

    paused_state
        .event(_self_box,
            [this](mhood_t<Interface::continue_interface> msg) {
                so_5::send<Interface::continue_interface>(_reader_box);

                this >>= running_state;
            });
}
