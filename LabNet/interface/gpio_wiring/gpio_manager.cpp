#include "gpio_manager.h"
#include "../digital_messages.h"
#include "../interface_messages.h"
#include "../../network/LabNet.pb.h"
#include "../../network/LabNetClient.pb.h"
#include "resource_request_helper.h"
#include <chrono>
#include <wiringPi.h>
#include <stdint-gcc.h>


namespace LabNet::interface::gpio_wiring
{
    enum PinType
    {
        None = 0,
        Out = 1,
        In = 2
    };

    GpioManager::GpioManager(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, log::Logger logger)
        : so_5::agent_t(ctx)
        , self_box_(self_box)
        , interfaces_manager_box_(interfaces_manager_box)
        , events_box_(events_box)
        , logger_(logger)
    {
        
    }

    GpioManager::~GpioManager()
    {
    }

    void GpioManager::so_evt_finish()
    {
        input_state_reader_.reset();

        so_environment().deregister_coop(res_helper_coop_, so_5::dereg_reason::normal);

        so_5::send<InterfaceStopped>(interfaces_manager_box_, Interfaces::GpioWiring);
        logger_->WriteInfoEntry("gpio wiringPi finished");
    }

    void GpioManager::so_evt_start()
    {
        logger_->WriteInfoEntry("gpio wiringPi started");
    }

    void GpioManager::so_define_agent()
    {
        this >>= init_state_;

        init_state_
            .event(self_box_,
                [this](const mhood_t<InitGpioWiring>& mes) {
                    auto res_helper = so_environment().make_agent<ResourceRequestHelper>(self_box_, logger_);
                    res_helper_box_ = res_helper->so_direct_mbox();
                    res_helper_coop_ = so_environment().register_agent_as_coop(std::move(res_helper));

                    reader_box_ = so_environment().create_mchain(so_5::make_unlimited_mchain_params());
                    input_state_reader_ = std::make_unique<DigitalInputStateReader>(events_box_, reader_box_, logger_);

                    so_5::send<InterfaceInitResult>(interfaces_manager_box_, Interfaces::GpioWiring, true);

                    this >>= running_state_;
                });

        running_state_
            .event(self_box_,
                [this](const mhood_t<InitGpioWiring>& msg) {
                    so_5::send<InterfaceInitResult>(interfaces_manager_box_, Interfaces::GpioWiring, true);
                })
            .event(self_box_,
                [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalIn> msg) {
                    uint8_t pin = static_cast<uint8_t>(msg->pin());
                    auto it = pins_.find(pin);

                    if (it == pins_.end())
                    {
                        so_5::send<DigitalInput>(res_helper_box_, pin, msg->is_inverted(), static_cast<Resistor>(msg->resistor_state()));
                    }
                    else if (it->second == PinType::In)
                    {
                        so_5::send<digital_messages::DigitalInInitResult>(interfaces_manager_box_, Interfaces::GpioWiring, pin, true);
                    }
                    else
                    {
                        so_5::send<digital_messages::DigitalInInitResult>(interfaces_manager_box_, Interfaces::GpioWiring, pin, false);
                    }
                })
            .event(self_box_,
                [this](const so_5::mhood_t<AcquireInputResult> msg) {
                    if (msg->result)
                    {
                        uint32_t pin = msg->dig_in.pin;
                        auto it = pins_.find(pin);
                        if (it == pins_.end())
                        {
                            pins_[pin] = PinType::In;
                            so_5::send<DigitalInput>(reader_box_, msg->dig_in);
                            so_5::send<digital_messages::DigitalInInitResult>(interfaces_manager_box_, Interfaces::GpioWiring, msg->dig_in.pin, true);
                        }
                    }
                    else
                    {
                        so_5::send<digital_messages::DigitalInInitResult>(interfaces_manager_box_, Interfaces::GpioWiring, msg->dig_in.pin, false);
                    }
                })
            .event(self_box_,
                [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalOut> msg) {
                    uint8_t pin = static_cast<uint8_t>(msg->pin());
                    auto it = pins_.find(pin);

                    if (it == pins_.end())
                    {
                        so_5::send<DigitalOutput>(res_helper_box_, pin, msg->is_inverted());
                    }
                    else if (it->second == PinType::Out)
                    {
                        so_5::send<digital_messages::DigitalOutInitResult>(interfaces_manager_box_, Interfaces::GpioWiring, pin, true);
                    }
                    else
                    {
                        so_5::send<digital_messages::DigitalOutInitResult>(interfaces_manager_box_, Interfaces::GpioWiring, pin, false);
                    }
                })
            .event(self_box_,
                [this](const so_5::mhood_t<AcquireOutputResult> msg) {
                    if (msg->result)
                    {
                        uint8_t pin = static_cast<uint8_t>(msg->dig_out.pin);
                        auto it = pins_.find(pin);
                        if (it == pins_.end())
                        {
                            pins_[pin] = PinType::Out;
                            outputs_[pin] = std::make_unique<DigitalOutput>(pin, msg->dig_out.is_inverted);

                            pinMode(pin, OUTPUT);
                            if (outputs_[pin]->is_inverted)
                            {
                                digitalWrite(pin, 1);
                            }
                            else
                            {
                                digitalWrite(pin, 0);
                            }

                            so_5::send<digital_messages::DigitalOutInitResult>(interfaces_manager_box_, Interfaces::GpioWiring, msg->dig_out.pin, true);
                        }
                    }
                    else
                    {
                        so_5::send<digital_messages::DigitalOutInitResult>(interfaces_manager_box_, Interfaces::GpioWiring, msg->dig_out.pin, false);
                    }
                })
            .event(self_box_,
                [this](mhood_t<digital_messages::SetDigitalOut> msg) {
                    auto it = outputs_.find(msg->pin);
                    if (it != outputs_.end())
                    {
                        if (outputs_[msg->pin]->is_inverted)
                        {
                            digitalWrite(outputs_[msg->pin]->pin, !msg->state);
                        }
                        else
                        {
                            digitalWrite(outputs_[msg->pin]->pin, msg->state);
                        }

                        so_5::send<digital_messages::ReturnDigitalOutState>(msg->mbox, Interfaces::GpioWiring, msg->pin, msg->state, std::chrono::high_resolution_clock::now());
                    }
                    else
                    {
                        so_5::send<digital_messages::InvalidDigitalOutPin>(msg->mbox, Interfaces::GpioWiring, msg->pin);
                    }
                })
            .event(self_box_,
                [this](mhood_t<PauseInterface> msg) {
                    so_5::send<PauseInterface>(reader_box_);

                    this >>= paused_state_;
                });

        paused_state_
            .event(self_box_,
                [this](mhood_t<ContinueInterface> msg) {
                    so_5::send<ContinueInterface>(reader_box_);

                    this >>= running_state_;
                });
    }
};