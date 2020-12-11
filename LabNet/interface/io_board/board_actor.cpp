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
    BoardActor::BoardActor(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, log::Logger logger)
        : so_5::agent_t(ctx)
        , self_box_(self_box)
        , interfaces_manager_box_(interfaces_manager_box)
        , events_box_(events_box)
        , logger_(logger)
    {
        out_pins_[1] = 5;
        out_pins_[2] = 26;
        out_pins_[3] = 6;
        out_pins_[4] = 22;
        out_pins_[5] = 21;
        out_pins_[6] = 7;
        out_pins_[7] = 1;
        out_pins_[8] = 0;
        out_pins_[9] = 4;
        out_pins_[10] = 3;

        in_pins_[1] = 23;
        in_pins_[2] = 27;
        in_pins_[3] = 28;
        in_pins_[4] = 24;
        in_pins_[5] = 25;
        in_pins_[6] = 29;
    }

    BoardActor::~BoardActor()
    {
    }

    void BoardActor::so_evt_finish()
    {
        input_state_reader_.reset();

        if (reader_box_)
            so_environment().deregister_coop(res_helper_coop_, so_5::dereg_reason::normal);

        so_5::send<InterfaceStopped>(interfaces_manager_box_, Interfaces::IoBoard);
        logger_->WriteInfoEntry("io board finished");
    }

    void BoardActor::so_evt_start()
    {
        logger_->WriteInfoEntry("io board started");
    }

    void BoardActor::so_define_agent()
    {
        this >>= init_state_;

        init_state_
            .event(self_box_,
                [this](const mhood_t<InitIoBoard>& mes) {
                    auto res_helper = so_environment().make_agent<ResourceRequestHelper>(self_box_, logger_);
                    res_helper_box_ = res_helper->so_direct_mbox();
                    res_helper_coop_ = so_environment().register_agent_as_coop(std::move(res_helper));
                    

                    reader_box_ = so_environment().create_mchain(so_5::make_unlimited_mchain_params());
                    input_state_reader_ = std::make_unique<DigitalInputStateReader>(events_box_, reader_box_, logger_);

                    so_5::send<InterfaceInitResult>(interfaces_manager_box_, Interfaces::IoBoard, true);

                    this >>= running_state_;
                });

        running_state_
            .event(self_box_,
                [this](const mhood_t<InitIoBoard>& msg) {
                    so_5::send<InterfaceInitResult>(interfaces_manager_box_, Interfaces::IoBoard, true);
                })
            .event(self_box_,
                [this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalIn> msg) {
                    uint32_t pin = msg->pin();
                    auto it = in_pins_.find(pin);

                    if (it != in_pins_.end())
                    {
                        auto inp_it = inputs_.find(pin);
                        if (inp_it == inputs_.end())
                        {
                            so_5::send<DigitalInput>(res_helper_box_, it->second, it->first, msg->is_inverted(), static_cast<Resistor>(msg->resistor_state()));
                        }
                        else
                        {
                            so_5::send<digital_messages::DigitalInInitResult>(interfaces_manager_box_, Interfaces::IoBoard, pin, true);
                        }
                    }
                    else
                    {
                        so_5::send<digital_messages::DigitalInInitResult>(interfaces_manager_box_, Interfaces::IoBoard, pin, false);
                    }
                })
            .event(self_box_,
                [this](const so_5::mhood_t<AcquireInputResult> msg) {
                    uint8_t pin = msg->dig_in.pin_l;
                    
                    if (msg->result)
                    {
                        auto it = inputs_.find(pin);
                        if (it == inputs_.end())
                        {
                            inputs_[pin] = std::make_unique<DigitalInput>(msg->dig_in);
                            so_5::send<DigitalInput>(reader_box_, msg->dig_in);
                        }
                        so_5::send<digital_messages::DigitalInInitResult>(interfaces_manager_box_, Interfaces::IoBoard, pin, true);
                    }
                    else
                    {
                        so_5::send<digital_messages::DigitalInInitResult>(interfaces_manager_box_, Interfaces::IoBoard, pin, false);
                    }
                })
            .event(self_box_,
                [this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalOut> msg) {
                    uint32_t pin = msg->pin();

                    auto it = out_pins_.find(pin);
                    if (it != out_pins_.end())
                    {
                        auto out_it = outputs_.find(pin);
                        if (out_it == outputs_.end())
                        {
                            so_5::send<DigitalOutput>(res_helper_box_, it->second, it->first, msg->is_inverted());
                        }
                        else
                        {
                            so_5::send<digital_messages::DigitalOutInitResult>(interfaces_manager_box_, Interfaces::IoBoard, pin, true);
                        }
                    }
                    else
                    {
                        so_5::send<digital_messages::DigitalOutInitResult>(interfaces_manager_box_, Interfaces::IoBoard, pin, false);
                    }
                })
            .event(self_box_,
                [this](const so_5::mhood_t<AcquireOutputResult> msg) {
                    if (msg->result)
                    {
                        uint8_t pin = msg->dig_out.pin_l;
                        auto it = outputs_.find(pin);
                        if (it == outputs_.end())
                        {
                            outputs_[pin] = std::make_unique<DigitalOutput>(msg->dig_out);

                            pinMode(outputs_[pin]->pin_h, OUTPUT);
                            if (outputs_[pin]->is_inverted)
                            {
                                digitalWrite(outputs_[pin]->pin_h, 1);
                            }
                            else
                            {
                                digitalWrite(outputs_[pin]->pin_h, 0);
                            }

                            so_5::send<digital_messages::DigitalOutInitResult>(interfaces_manager_box_, Interfaces::IoBoard, pin, true);
                        }
                    }
                    else
                    {
                        so_5::send<digital_messages::DigitalOutInitResult>(interfaces_manager_box_, Interfaces::IoBoard, msg->dig_out.pin_l, false);
                    }
                })
            .event(self_box_,
                [this](const mhood_t<digital_messages::SetDigitalOut>& msg) {
                    auto it = outputs_.find(msg->pin);
                    if (it != outputs_.end())
                    {
                        if (it->second->is_inverted)
                        {
                            digitalWrite(it->second->pin_h, !msg->state);
                        }
                        else
                        {
                            digitalWrite(it->second->pin_h, msg->state);
                        }

                        so_5::send<digital_messages::ReturnDigitalOutState>(msg->mbox, Interfaces::IoBoard, msg->pin, msg->state, std::chrono::high_resolution_clock::now());
                    }
                    else
                    {
                        so_5::send<digital_messages::InvalidDigitalOutPin>(msg->mbox, Interfaces::IoBoard, msg->pin);
                    }
                })
            .event(self_box_,
                [this](const mhood_t<PauseInterface>& msg) {
                    so_5::send<PauseInterface>(reader_box_);

                    this >>= paused_state_;
                });

        paused_state_
            .event(self_box_,
                [this](const mhood_t<ContinueInterface>& msg) {
                    so_5::send<ContinueInterface>(reader_box_);

                    this >>= running_state_;
                });
    }
};