#pragma once

#include "id_test.h"
#include "stats.h"
#include <iostream>
#include <fstream>
#include <so_5/all.hpp>
#include "set_dig_out_test.h"

class save_latencies final : public so_5::agent_t
{
public:
    save_latencies(context_t ctx)
        : so_5::agent_t(ctx)
    {
    }

    void on_id_latencies(std::shared_ptr<id_latencies> mes)
    {
        id_stats = mes->stats;
    }

    void on_set_latencies(std::shared_ptr<set_latencies> mes)
    {
        set_stats = mes->stats;

        std::ofstream f;
        f.open("latencies.csv");

        f << "IdTest;SetTest\n";
        f << std::fixed << std::setprecision(3);

        for (size_t i = 0; i < id_stats->records_->size(); i++)
        {
            f << id_stats->records_->at(i) << ";";
            f << set_stats->records_->at(i);
            f << "\n";
        }

        f.close();

        std::cout << "save latencies done " << std::endl;
    }

    void so_define_agent() override
    {
        so_subscribe_self()
            .event(&save_latencies::on_id_latencies)
            .event(&save_latencies::on_set_latencies);
    }

private:
    std::shared_ptr<Stats> id_stats;
    std::shared_ptr<Stats> set_stats;
};