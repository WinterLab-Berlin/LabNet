#include "GPIOManager.h"
#include "../../network/server_messages.h"
#include "../../network/LabNet.pb.h"
#include "../../network/LabNetClient.pb.h"
#include "../DigitalMessages.h"
#include "../InterfaceMessages.h"
#include <chrono>
#include <wiringPi.h>

using namespace io_board;

GPIOManager::GPIOManager(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, Logger logger)
    : so_5::agent_t(ctx)
    , _self_box(self_box)
    , _interfaces_manager_box(interfaces_manager_box)
    , _events_box(events_box)
    , _logger(logger)
{
}

GPIOManager::~GPIOManager()
{
}

void GPIOManager::so_evt_finish()
{
    _inputStateReader.reset();

    so_5::send<Interface::interface_stopped>(_interfaces_manager_box, Interface::Interfaces::IO_BOARD);
    _logger->writeInfoEntry("io board finished");
}

void GPIOManager::so_evt_start()
{
    _outPins[1] = 5;
    _outPins[2] = 26;
    _outPins[3] = 6;
    _outPins[4] = 22;
    _outPins[5] = 21;
    _outPins[6] = 7;
    _outPins[7] = 1;
    _outPins[8] = 0;
    _outPins[9] = 4;
    _outPins[10] = 3;

    _inPins[1] = 23;
    _inPins[2] = 27;
    _inPins[3] = 28;
    _inPins[4] = 24;
    _inPins[5] = 25;
    _inPins[6] = 29;

    for (auto& out : _outPins)
    {
        _outputs[out.first] = DigitalOutput { out.second, out.first };
    }

    for (auto& inp : _inPins)
    {
        _inputs[inp.first] = DigitalInput { inp.second, inp.first };
    }

    _logger->writeInfoEntry("io board started");
}

void GPIOManager::so_define_agent()
{
    this >>= init_state;

    init_state
        .event(_self_box,
            [this](const mhood_t<init_io_board>& mes) {
                _inputStateReader = std::make_unique<DigitalInputStateReader>(_events_box, &_inputs, _logger);

                so_5::send<Interface::interface_init_result>(_interfaces_manager_box, Interface::Interfaces::IO_BOARD, true);

                this >>= running_state;
            });

    running_state
        .event(_self_box,
            [this](const mhood_t<init_io_board>& msg) {
                so_5::send<Interface::interface_init_result>(_interfaces_manager_box, Interface::Interfaces::IO_BOARD, true);
            })
        .event(_self_box,
            [this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalIn> msg) {
                uint32_t pin = msg->pin();
                if (pin > 0 && pin < 7)
                {
                    _inputs[pin].is_inverted = msg->is_inverted();

                    pinMode(_inputs[pin].pin_h, INPUT);
                    switch (msg->resistor_state())
                    {
                    case LabNetProt::Client::IoBoardInitDigitalIn_Resistor::IoBoardInitDigitalIn_Resistor_OFF:
                        pullUpDnControl(_inputs[pin].pin_h, PUD_OFF);
                        break;
                    case LabNetProt::Client::IoBoardInitDigitalIn_Resistor::IoBoardInitDigitalIn_Resistor_PULL_DOWN:
                        pullUpDnControl(_inputs[pin].pin_h, PUD_DOWN);
                        break;
                    case LabNetProt::Client::IoBoardInitDigitalIn_Resistor::IoBoardInitDigitalIn_Resistor_PULL_UP:
                        pullUpDnControl(_inputs[pin].pin_h, PUD_UP);
                        break;
                    }

                    _inputs[pin].available = true;
                    _inputs[pin].state = 2;

                    so_5::send<DigitalMessages::digital_in_init_result>(_interfaces_manager_box, Interface::IO_BOARD, pin, true);
                }
                else
                {
                    so_5::send<DigitalMessages::digital_in_init_result>(_interfaces_manager_box, Interface::IO_BOARD, pin, false);
                }
            })
        .event(_self_box,
            [this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalOut> msg) {
                uint32_t pin = msg->pin();
                if (pin > 0 && pin < 11)
                {
                    _outputs[pin].is_inverted = msg->is_inverted();
                    _outputs[pin].available = true;

                    pinMode(_outputs[pin].pin_h, OUTPUT);
                    if (_outputs[pin].is_inverted)
                    {
                        digitalWrite(_outputs[pin].pin_h, 1);
                    }
                    else
                    {
                        digitalWrite(_outputs[pin].pin_h, 0);
                    }

                    so_5::send<DigitalMessages::digital_out_init_result>(_interfaces_manager_box, Interface::IO_BOARD, pin, true);
                }
                else
                {
                    so_5::send<DigitalMessages::digital_out_init_result>(_interfaces_manager_box, Interface::IO_BOARD, pin, false);
                }
            })
        .event(_self_box,
            [this](const mhood_t<DigitalMessages::set_digital_out>& msg) {
                //_logger->writeInfoEntry(string_format("set digital out %d", msg->state));
                if (msg->pin > 0 && msg->pin < 11)
                {
                    if (_outputs[msg->pin].is_inverted)
                    {
                        digitalWrite(_outputs[msg->pin].pin_h, !msg->state);
                    }
                    else
                    {
                        digitalWrite(_outputs[msg->pin].pin_h, msg->state);
                    }

                    so_5::send<DigitalMessages::return_digital_out_state>(msg->mbox, Interface::IO_BOARD, msg->pin, msg->state, std::chrono::high_resolution_clock::now());
                }
                else
                {
                    so_5::send<DigitalMessages::invalid_digital_out_pin>(msg->mbox, Interface::IO_BOARD, msg->pin);
                }
            })
        .event(_self_box,
            [this](const mhood_t<Interface::pause_interface>& msg) {
                _inputStateReader.reset();

                this >>= paused_state;
            });

    paused_state
        .event(_self_box,
            [this](const mhood_t<Interface::continue_interface>& msg) {
                _inputStateReader = std::make_unique<DigitalInputStateReader>(_events_box, &_inputs, _logger);

                this >>= running_state;
            });
}
