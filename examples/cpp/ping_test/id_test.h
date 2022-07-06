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

struct id_latencies
{
    std::shared_ptr<Stats> stats;
};

class id_test final : public so_5::agent_t
{
public:
    id_test(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t next_box, const so_5::mbox_t save_box, std::shared_ptr<Client> client, int32_t req)
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
                [this](const std::shared_ptr<LabNetProt::Server::LabNetIdReply> id) {
                    auto time = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double, std::milli> ms = time - start_time;

                    stats->AddRecord(ms.count());

                    req_--;
                    if (req_ >= 0)
                    {
                        std::cout << "\r       \r" << req_ << std::flush;
                        client_->SendMessage(id_mes_, LabNetProt::Client::LABNET_ID_REQUEST);
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

                            std::shared_ptr<id_latencies> l = std::make_shared<id_latencies>();
                            l->stats = std::move(stats);
                            so_5::send<std::shared_ptr<id_latencies>>(save_box_, std::move(l));
                        }
                        else
                        {
                            client_->Stop();
                        }
                    }
                })
            .event(self_box_,
                [this](const mhood_t<start>) {
                    std::cout << "id test" << std::endl;
                    client_->SendMessage(id_mes_, LabNetProt::Client::LABNET_ID_REQUEST);

                    start_time = std::chrono::high_resolution_clock::now();
                });
    }

    void so_evt_start() override
    {
        id_mes_ = std::make_shared<LabNetProt::Client::LabNetIdRequest>();
        stats = std::make_shared<Stats>();
    }

    so_5::state_t init_state_ { this, "init_state" };
    so_5::mbox_t self_box_;
    so_5::mbox_t next_box_;
    so_5::mbox_t save_box_;
    std::shared_ptr<Client> client_;
    std::chrono::system_clock::time_point start_time;

    std::shared_ptr<Stats> stats;
    int32_t req_;
    std::shared_ptr<LabNetProt::Client::LabNetIdRequest> id_mes_;
};