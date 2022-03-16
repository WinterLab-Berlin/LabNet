#pragma once

#include <iostream>
#include <memory>
#include "client.h"
#include <so_5/all.hpp>
#include <chrono>
#include <ratio>
#include <iomanip>
#include "stats.h"
#include "start_mes.h"

#include "prot/LabNet.pb.h"
#include "prot/LabNetClient.pb.h"
#include "prot/LabNetServer.pb.h"

struct set_latencies
{
    std::shared_ptr<Stats> stats;
};

class set_dig_out_test final : public so_5::agent_t
{
public:
    set_dig_out_test(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t next_box, const so_5::mbox_t save_box, std::shared_ptr<Client> client, int32_t req)
        : so_5::agent_t(ctx)
        , self_box_(self_box)
        , next_box_(next_box)
        , save_box_(save_box)
        , client_(client)
        , req_(req)
    {
    }

private:
    void so_define_agent() override
    {
        this >>= init_state_;

        init_state_
            .event(self_box_,
                [this](const mhood_t<start>) {
                    std::cout << "set digital out test" << std::endl;

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
                        std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalOut> dig_out = std::make_shared<LabNetProt::Client::GpioWiringPiInitDigitalOut>();
                        dig_out->set_pin(5);
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
                        turn_on_ = std::make_shared<LabNetProt::Client::DigitalOutSet>();
                        LabNetProt::PinId* id = new LabNetProt::PinId();
                        id->set_interface(LabNetProt::Interfaces::INTERFACE_GPIO_WIRINGPI);
                        id->set_pin(5);
                        turn_on_->set_allocated_id(id);
                        turn_on_->set_state(true);

                        turn_off_ = std::make_shared<LabNetProt::Client::DigitalOutSet>();
                        turn_off_->set_allocated_id(id);
                        turn_off_->set_state(false);

                        client_->SendMessage(turn_on_, LabNetProt::Client::DIGITAL_OUT_SET);
                        start_time = std::chrono::high_resolution_clock::now();

                        this >>= test_state_;
                    }
                    else
                    {
                        std::cout << "cannot init digital out" << std::endl;
                    }
                });

        test_state_
            .event(self_box_,
                [this](const std::shared_ptr<LabNetProt::Server::DigitalOutState> mes) {
                    auto time = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double, std::milli> ms = time - start_time;

                    stats->AddRecord(ms.count());

                    req_--;
                    if (req_ >= 0)
                    {
                        std::cout << "\r       \r" << req_;

                        if (mes->state())
                            client_->SendMessage(turn_off_, LabNetProt::Client::DIGITAL_OUT_SET);
                        else
                            client_->SendMessage(turn_on_, LabNetProt::Client::DIGITAL_OUT_SET);

                        start_time = std::chrono::high_resolution_clock::now();
                    }
                    else
                    {
                        std::cout << std::fixed << std::setprecision(2)
                                  << "\rmean: " << stats->GetMean() << " std dev: " << stats->GetStandardDeviation() << " median: " << stats->GetPercentile(50)
                                  << " p25: " << stats->GetPercentile(25) << " p75: " << stats->GetPercentile(75)
                                  << " p2.5: " << stats->GetPercentile(2.5) << " p97.5: " << stats->GetPercentile(97.5) << std::endl;

                        if (next_box_)
                        {
                            client_->SetRecvBox(next_box_);
                            so_5::send<start>(next_box_);

                            std::shared_ptr<set_latencies> l = std::make_shared<set_latencies>();
                            l->stats = std::move(stats);
                            so_5::send<std::shared_ptr<set_latencies>>(save_box_, std::move(l));
                        }
                        else
                        {
                            client_->Stop();
                        }
                    }
                });
    }

    void so_evt_start() override
    {
        stats = std::make_shared<Stats>();
    }

    so_5::state_t init_state_ { this, "init_state" };
    so_5::state_t test_state_ { this, "test_state" };
    so_5::mbox_t self_box_;
    so_5::mbox_t next_box_;
    so_5::mbox_t save_box_;
    std::shared_ptr<Client> client_;
    std::chrono::steady_clock::time_point start_time;

    std::shared_ptr<Stats> stats;
    int32_t req_;
    std::shared_ptr<LabNetProt::Client::DigitalOutSet> turn_on_;
    std::shared_ptr<LabNetProt::Client::DigitalOutSet> turn_off_;
};