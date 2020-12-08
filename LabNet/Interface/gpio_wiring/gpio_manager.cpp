#include "gpio_manager.h"
#include "../digital_messages.h"
#include "../interface_messages.h"
#include "../../network/LabNet.pb.h"
#include "../../network/LabNetClient.pb.h"
#include "resource_request_helper.h"
#include <chrono>
#include <wiringPi.h>


namespace LabNet::interface::gpio_wiring
{
    enum PinType
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
        
    }

    GpioManager::~GpioManager()
    {
    }

    void GpioManager::so_evt_finish()
    {
        _input_state_reader.reset();

        so_environment().deregister_coop(_res_helper_coop, so_5::dereg_reason::normal);

        so_5::send<InterfaceStopped>(_interfaces_manager_box, Interfaces::GPIO_WIRING);
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
                    auto res_helper = so_environment().make_agent<ResourceRequestHelper>(_self_box, _logger);
                    _res_helper_box = res_helper->so_direct_mbox();
                    _res_helper_coop = so_environment().register_agent_as_coop(std::move(res_helper));

                    _reader_box = so_environment().create_mchain(so_5::make_unlimited_mchain_params());
                    _input_state_reader = std::make_unique<DigitalInputStateReader>(_events_box, _reader_box, _logger);

                    so_5::send<InterfaceInitResult>(_interfaces_manager_box, Interfaces::GPIO_WIRING, true);

                    this >>= running_state;
                });

        running_state
            .event(_self_box,
                [this](const mhood_t<init_gpio_wiring>& msg) {
                    so_5::send<InterfaceInitResult>(_interfaces_manager_box, Interfaces::GPIO_WIRING, true);
                })
            .event(_self_box,
                [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalIn> msg) {
                    uint8_t pin = static_cast<uint8_t>(msg->pin());
                    auto it = _pins.find(pin);

                    if (it == _pins.end())
                    {
                        so_5::send<DigitalInput>(_res_helper_box, pin, msg->is_inverted(), static_cast<Resistor>(msg->resistor_state()));
                    }
                    else if (it->second == PinType::In)
                    {
                        so_5::send<digital_messages::DigitalInInitResult>(_interfaces_manager_box, Interfaces::GPIO_WIRING, pin, true);
                    }
                    else
                    {
                        so_5::send<digital_messages::DigitalInInitResult>(_interfaces_manager_box, Interfaces::GPIO_WIRING, pin, false);
                    }
                })
            .event(_self_box,
                [this](const so_5::mhood_t<AcquireInputResult> msg) {
                    if (msg->result)
                    {
                        uint32_t pin = msg->dig_in.pin;
                        auto it = _pins.find(pin);
                        if (it == _pins.end())
                        {
                            _pins[pin] = PinType::In;
                            so_5::send<DigitalInput>(_reader_box, msg->dig_in);
                            so_5::send<digital_messages::DigitalInInitResult>(_interfaces_manager_box, Interfaces::GPIO_WIRING, msg->dig_in.pin, true);
                        }
                    }
                    else
                    {
                        so_5::send<digital_messages::DigitalInInitResult>(_interfaces_manager_box, Interfaces::GPIO_WIRING, msg->dig_in.pin, false);
                    }
                })
            .event(_self_box,
                [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalOut> msg) {
                    uint8_t pin = static_cast<uint8_t>(msg->pin());
                    auto it = _pins.find(pin);

                    if (it == _pins.end())
                    {
                        so_5::send<DigitalOutput>(_res_helper_box, pin, msg->is_inverted());
                    }
                    else if (it->second == PinType::Out)
                    {
                        so_5::send<digital_messages::DigitalOutInitResult>(_interfaces_manager_box, Interfaces::GPIO_WIRING, pin, true);
                    }
                    else
                    {
                        so_5::send<digital_messages::DigitalOutInitResult>(_interfaces_manager_box, Interfaces::GPIO_WIRING, pin, false);
                    }
                })
            .event(_self_box,
                [this](const so_5::mhood_t<AcquireOutputResult> msg) {
                    if (msg->result)
                    {
                        uint8_t pin = static_cast<uint8_t>(msg->dig_out.pin);
                        auto it = _pins.find(pin);
                        if (it == _pins.end())
                        {
                            _pins[pin] = PinType::Out;
                            _outputs[pin] = std::make_unique<DigitalOutput>(pin, msg->dig_out.is_inverted);

                            pinMode(pin, OUTPUT);
                            if (_outputs[pin]->is_inverted)
                            {
                                digitalWrite(pin, 1);
                            }
                            else
                            {
                                digitalWrite(pin, 0);
                            }

                            so_5::send<digital_messages::DigitalOutInitResult>(_interfaces_manager_box, Interfaces::GPIO_WIRING, msg->dig_out.pin, true);
                        }
                    }
                    else
                    {
                        so_5::send<digital_messages::DigitalOutInitResult>(_interfaces_manager_box, Interfaces::GPIO_WIRING, msg->dig_out.pin, false);
                    }
                })
            .event(_self_box,
                [this](mhood_t<digital_messages::SetDigitalOut> msg) {
                    auto it = _outputs.find(msg->pin);
                    if (it != _outputs.end())
                    {
                        if (_outputs[msg->pin]->is_inverted)
                        {
                            digitalWrite(_outputs[msg->pin]->pin, !msg->state);
                        }
                        else
                        {
                            digitalWrite(_outputs[msg->pin]->pin, msg->state);
                        }

                        so_5::send<digital_messages::ReturnDigitalOutState>(msg->mbox, Interfaces::GPIO_WIRING, msg->pin, msg->state, std::chrono::high_resolution_clock::now());
                    }
                    else
                    {
                        so_5::send<digital_messages::InvalidDigitalOutPin>(msg->mbox, Interfaces::GPIO_WIRING, msg->pin);
                    }
                })
            .event(_self_box,
                [this](mhood_t<PauseInterface> msg) {
                    so_5::send<PauseInterface>(_reader_box);

                    this >>= paused_state;
                });

        paused_state
            .event(_self_box,
                [this](mhood_t<ContinueInterface> msg) {
                    so_5::send<ContinueInterface>(_reader_box);

                    this >>= running_state;
                });
    }
};