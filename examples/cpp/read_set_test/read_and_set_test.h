#pragma once

#include <iostream>
#include <memory>
#include "client.h"
#include <so_5/all.hpp>
#include <chrono>
#include <ratio>
#include <map>
#include <iomanip>
#include "start_mes.h"

#include "prot/LabNet.pb.h"
#include "prot/LabNetClient.pb.h"
#include "prot/LabNetServer.pb.h"

class read_and_set_test final : public so_5::agent_t
{
public:
    read_and_set_test(context_t ctx, const so_5::mbox_t self_box, std::shared_ptr<Client> client)
        : so_5::agent_t(ctx)
        , self_box_(self_box)
        , client_(client)
    {
        for (size_t i = 0; i < max_pins_nbr_; i++)
        {
            in_out_map_[in_pins_[i]] = out_pins_[i];
        }
    }

private:
    void so_define_agent() override
    {
        init_state_
            .event(self_box_,
                [this](const mhood_t<start>) {
                    std::cout << "read and set test" << std::endl;

                    std::shared_ptr<LabNetProt::Client::LabNetResetRequest> reset = std::make_shared<LabNetProt::Client::LabNetResetRequest>();
                    client_->SendMessage(reset, LabNetProt::Client::LABNET_RESET_REQUEST);
                })
            .event(self_box_,
                [this](const std::shared_ptr<LabNetProt::Server::LabNetResetReply>) {
                    std::shared_ptr<LabNetProt::Client::GpioWiringPiInit> gpio = std::make_shared<LabNetProt::Client::GpioWiringPiInit>();
                    client_->SendMessage(gpio, LabNetProt::Client::GPIO_WIRINGPI_INIT);
                })
            .event(self_box_,
                [this](const std::shared_ptr<LabNetProt::Server::InterfaceInitResult> res) {
                    if (res->IsInitialized())
                    {
                        pinInitNbr_ = 0;

                        std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalOut> dig_out = std::make_shared<LabNetProt::Client::GpioWiringPiInitDigitalOut>();
                        dig_out->set_pin(out_pins_[pinInitNbr_]);
                        dig_out->set_is_inverted(false);
                        client_->SendMessage(dig_out, LabNetProt::Client::GPIO_WIRINGPI_INIT_DIGITAL_OUT);
                    }
                    else
                    {
                        std::cout << "cannot init WiringPi" << std::endl;
                    }
                })
            .event(self_box_,
                [this](const std::shared_ptr<LabNetProt::Server::DigitalOutInitResult> res) {
                    if (res->is_succeed())
                    {
                        pinInitNbr_++;
                        if (pinInitNbr_ < max_pins_nbr_)
                        {
                            std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalOut> dig_out = std::make_shared<LabNetProt::Client::GpioWiringPiInitDigitalOut>();
                            dig_out->set_pin(out_pins_[pinInitNbr_]);
                            dig_out->set_is_inverted(false);
                            client_->SendMessage(dig_out, LabNetProt::Client::GPIO_WIRINGPI_INIT_DIGITAL_OUT);
                        }
                        else
                        {
                            pinInitNbr_ = 0;
                            std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalIn> dig_out = std::make_shared<LabNetProt::Client::GpioWiringPiInitDigitalIn>();
                            dig_out->set_pin(in_pins_[pinInitNbr_]);
                            dig_out->set_is_inverted(false);
                            client_->SendMessage(dig_out, LabNetProt::Client::GPIO_WIRINGPI_INIT_DIGITAL_IN);
                        }
                    }
                    else
                    {
                        std::cout << "cannot init digital out" << std::endl;
                    }
                })
            .event(self_box_,
                [this](const std::shared_ptr<LabNetProt::Server::DigitalInInitResult> res)
                {
                    if (res->is_succeed())
                    {
                        pinInitNbr_++;
                        if (pinInitNbr_ < max_pins_nbr_)
                        {
                            std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalIn> dig_out = std::make_shared<LabNetProt::Client::GpioWiringPiInitDigitalIn>();
                            dig_out->set_pin(in_pins_[pinInitNbr_]);
                            dig_out->set_is_inverted(false);
                            client_->SendMessage(dig_out, LabNetProt::Client::GPIO_WIRINGPI_INIT_DIGITAL_IN);
                        }
                        else
                        {
                            this >>= test_state_;
                        }
                    }
                    else
                    {
                        std::cout << "cannot init digital in" << std::endl;
                    }
                });

        test_state_
            .event(self_box_,
                [this](const std::shared_ptr<LabNetProt::Server::DigitalInState> mes) {
                    try
                    {
                        int pin = in_out_map_.at(mes->pin().pin());

                        if (mes->state())
                        {
                            auto turn_on = std::make_shared<LabNetProt::Client::DigitalOutSet>();
                            LabNetProt::PinId* id = new LabNetProt::PinId();
                            id->set_interface(LabNetProt::Interfaces::INTERFACE_GPIO_WIRINGPI);
                            id->set_pin(pin);
                            turn_on->set_allocated_id(id);
                            turn_on->set_state(true);

                            client_->SendMessage(turn_on, LabNetProt::Client::DIGITAL_OUT_SET);
                        }
                        else
                        {
                            auto turn_off = std::make_shared<LabNetProt::Client::DigitalOutSet>();
                            LabNetProt::PinId* id = new LabNetProt::PinId();
                            id->set_interface(LabNetProt::Interfaces::INTERFACE_GPIO_WIRINGPI);
                            id->set_pin(pin);
                            turn_off->set_allocated_id(id);
                            turn_off->set_state(false);

                            client_->SendMessage(turn_off, LabNetProt::Client::DIGITAL_OUT_SET);
                        }
                    }
                    catch (const std::exception&)
                    {
                    }
                });
    }

    void so_evt_start() override
    {
        this >>= init_state_;
    }

    int max_pins_nbr_ = 14;
    int in_pins_[14] = { 29, 28, 27, 26, 31, 11, 10, 6, 5, 4, 1, 16, 15, 8 };
    int out_pins_[14] = { 25, 24, 23, 22, 21, 30, 14, 13, 12, 3, 2, 0, 7, 9 };
    std::map<int, int> in_out_map_;
    int pinInitNbr_ = 0;

    so_5::state_t init_state_ { this, "init_state" };
    so_5::state_t test_state_ { this, "test_state" };
    so_5::mbox_t self_box_;
    std::shared_ptr<Client> client_;
};