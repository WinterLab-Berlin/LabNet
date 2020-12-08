#include "board_actor.h"
#include "../../network/LabNet.pb.h"
#include "../../network/LabNetClient.pb.h"
#include "../../network/server_messages.h"
#include "../digital_messages.h"
#include "../interface_messages.h"
#include "resource_request_helper.h"
#include <chrono>
#include <wiringPi.h>

namespace LabNet::interface::io_board
{
    BoardActor::BoardActor(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, Logger logger)
        : so_5::agent_t(ctx)
        , _self_box(self_box)
        , _interfaces_manager_box(interfaces_manager_box)
        , _events_box(events_box)
        , _logger(logger)
    {
        _out_pins[1] = 5;
        _out_pins[2] = 26;
        _out_pins[3] = 6;
        _out_pins[4] = 22;
        _out_pins[5] = 21;
        _out_pins[6] = 7;
        _out_pins[7] = 1;
        _out_pins[8] = 0;
        _out_pins[9] = 4;
        _out_pins[10] = 3;

        _in_pins[1] = 23;
        _in_pins[2] = 27;
        _in_pins[3] = 28;
        _in_pins[4] = 24;
        _in_pins[5] = 25;
        _in_pins[6] = 29;
    }

    BoardActor::~BoardActor()
    {
    }

    void BoardActor::so_evt_finish()
    {
        _input_state_reader.reset();

        if (_reader_box)
            so_environment().deregister_coop(_res_helper_coop, so_5::dereg_reason::normal);

        so_5::send<InterfaceStopped>(_interfaces_manager_box, Interfaces::IO_BOARD);
        _logger->writeInfoEntry("io board finished");
    }

    void BoardActor::so_evt_start()
    {
        _logger->writeInfoEntry("io board started");
    }

    void BoardActor::so_define_agent()
    {
        this >>= init_state;

        init_state
            .event(_self_box,
                [this](const mhood_t<init_io_board>& mes) {
                    auto res_helper = so_environment().make_agent<ResourceRequestHelper>(_self_box, _logger);
                    _res_helper_box = res_helper->so_direct_mbox();
                    _res_helper_coop = so_environment().register_agent_as_coop(std::move(res_helper));
                    

                    _reader_box = so_environment().create_mchain(so_5::make_unlimited_mchain_params());
                    _input_state_reader = std::make_unique<DigitalInputStateReader>(_events_box, _reader_box, _logger);

                    so_5::send<InterfaceInitResult>(_interfaces_manager_box, Interfaces::IO_BOARD, true);

                    this >>= running_state;
                });

        running_state
            .event(_self_box,
                [this](const mhood_t<init_io_board>& msg) {
                    so_5::send<InterfaceInitResult>(_interfaces_manager_box, Interfaces::IO_BOARD, true);
                })
            .event(_self_box,
                [this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalIn> msg) {
                    uint32_t pin = msg->pin();
                    auto it = _in_pins.find(pin);

                    if (it != _in_pins.end())
                    {
                        auto inp_it = _inputs.find(pin);
                        if (inp_it == _inputs.end())
                        {
                            so_5::send<DigitalInput>(_res_helper_box, it->second, it->first, msg->is_inverted(), static_cast<Resistor>(msg->resistor_state()));
                        }
                        else
                        {
                            so_5::send<digital_messages::DigitalInInitResult>(_interfaces_manager_box, Interfaces::IO_BOARD, pin, true);
                        }
                    }
                    else
                    {
                        so_5::send<digital_messages::DigitalInInitResult>(_interfaces_manager_box, Interfaces::IO_BOARD, pin, false);
                    }
                })
            .event(_self_box,
                [this](const so_5::mhood_t<AcquireInputResult> msg) {
                    uint8_t pin = msg->dig_in.pin_l;
                    
                    if (msg->result)
                    {
                        auto it = _inputs.find(pin);
                        if (it == _inputs.end())
                        {
                            _inputs[pin] = std::make_unique<DigitalInput>(msg->dig_in);
                            so_5::send<DigitalInput>(_reader_box, msg->dig_in);
                        }
                        so_5::send<digital_messages::DigitalInInitResult>(_interfaces_manager_box, Interfaces::IO_BOARD, pin, true);
                    }
                    else
                    {
                        so_5::send<digital_messages::DigitalInInitResult>(_interfaces_manager_box, Interfaces::IO_BOARD, pin, false);
                    }
                })
            .event(_self_box,
                [this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalOut> msg) {
                    uint32_t pin = msg->pin();

                    auto it = _out_pins.find(pin);
                    if (it != _out_pins.end())
                    {
                        auto out_it = _outputs.find(pin);
                        if (out_it == _outputs.end())
                        {
                            so_5::send<DigitalOutput>(_res_helper_box, it->second, it->first, msg->is_inverted());
                        }
                        else
                        {
                            so_5::send<digital_messages::DigitalOutInitResult>(_interfaces_manager_box, Interfaces::IO_BOARD, pin, true);
                        }
                    }
                    else
                    {
                        so_5::send<digital_messages::DigitalOutInitResult>(_interfaces_manager_box, Interfaces::IO_BOARD, pin, false);
                    }
                })
            .event(_self_box,
                [this](const so_5::mhood_t<AcquireOutputResult> msg) {
                    if (msg->result)
                    {
                        uint8_t pin = msg->dig_out.pin_l;
                        auto it = _outputs.find(pin);
                        if (it == _outputs.end())
                        {
                            _outputs[pin] = std::make_unique<DigitalOutput>(msg->dig_out);

                            pinMode(_outputs[pin]->pin_h, OUTPUT);
                            if (_outputs[pin]->is_inverted)
                            {
                                digitalWrite(_outputs[pin]->pin_h, 1);
                            }
                            else
                            {
                                digitalWrite(_outputs[pin]->pin_h, 0);
                            }

                            so_5::send<digital_messages::DigitalOutInitResult>(_interfaces_manager_box, Interfaces::IO_BOARD, pin, true);
                        }
                    }
                    else
                    {
                        so_5::send<digital_messages::DigitalOutInitResult>(_interfaces_manager_box, Interfaces::GPIO_WIRING, msg->dig_out.pin_l, false);
                    }
                })
            .event(_self_box,
                [this](const mhood_t<digital_messages::SetDigitalOut>& msg) {
                    auto it = _outputs.find(msg->pin);
                    if (it != _outputs.end())
                    {
                        if (it->second->is_inverted)
                        {
                            digitalWrite(it->second->pin_h, !msg->state);
                        }
                        else
                        {
                            digitalWrite(it->second->pin_h, msg->state);
                        }

                        so_5::send<digital_messages::ReturnDigitalOutState>(msg->mbox, Interfaces::IO_BOARD, msg->pin, msg->state, std::chrono::high_resolution_clock::now());
                    }
                    else
                    {
                        so_5::send<digital_messages::InvalidDigitalOutPin>(msg->mbox, Interfaces::IO_BOARD, msg->pin);
                    }
                })
            .event(_self_box,
                [this](const mhood_t<PauseInterface>& msg) {
                    so_5::send<PauseInterface>(_reader_box);

                    this >>= paused_state;
                });

        paused_state
            .event(_self_box,
                [this](const mhood_t<ContinueInterface>& msg) {
                    so_5::send<ContinueInterface>(_reader_box);

                    this >>= running_state;
                });
    }
};