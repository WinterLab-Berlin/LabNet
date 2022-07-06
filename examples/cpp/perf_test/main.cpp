#include <iostream>
#include <chrono>
#include <wiringPi.h>
#include <thread>
#include <fstream>
#include <iomanip>
#include <thread>
#include <future>
#include <sstream>
#include <vector>
#include <deque>
#include "stats.h"

using namespace std::chrono_literals;

void test_func(std::shared_ptr<Stats> stats, int pin_in, int pin_out, int runs, int thread_nbr, int* progress)
{
    pinMode(pin_out, OUTPUT);
    digitalWrite(pin_out, 0);
    pinMode(pin_in, INPUT);

    int pin_state = 0;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point time;
    std::chrono::duration<double, std::milli> dur;
    for (size_t i = 0; i < runs;)
    {
        int np = (int)(((float)i / runs) * 100.0);
        if(np != *progress)
        {
            *progress = np;
        }
        
        start_time = std::chrono::high_resolution_clock::now();
        digitalWrite(pin_out, 1);

        int loop = 0;
        bool check = true;
        while (pin_state == 0)
        {
            pin_state = digitalRead(pin_in);
            loop++;
            if(loop > 100)
            {
                time = std::chrono::high_resolution_clock::now();
                dur = time - start_time;
                if(dur.count() > 10)
                {
                    break;
                }
                loop = 0;
            }
        }

        if(pin_state == 1)
        {
            i++;
            time = std::chrono::high_resolution_clock::now();
            dur = time - start_time;
            stats->AddRecord(dur.count());
        }

        digitalWrite(pin_out, 0);
        while (pin_state == 1)
        {
            pin_state = digitalRead(pin_in);
        }

        std::this_thread::sleep_for(1ms);
    }
}

void pr_func(int* all_progress, int tests)
{
    for (;;)
    {
        int p = 0;
        for (size_t t = 0; t < tests; t++)
        {
            p += all_progress[t];
        }
        p /= tests;
        std::cout << "\r                    \r" << "progress " << p << "\% of " << tests << std::flush;

        if(p >= 99)
            return;
        
        std::this_thread::sleep_for(100ms);
    }
}

int main(int argc, char *argv[])
{
    int err = wiringPiSetup();
    if (err == -1)
    {
        std::cerr << "wiringPi init failed\n";
        exit(1);
    }

    int in_pins[] = {29, 28, 27, 26, 31, 11, 10, 6, 5, 4, 1, 16, 15, 8};
    int out_pins[] = {25, 24, 23, 22, 21, 30, 14, 13, 12, 3, 2, 0, 7, 9};
    int run_progress[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int runs = 10000;
    int tests = 1;
    
    if(argc > 1)
    {
        int tmp = atoi(argv[1]);
        if(tmp > 0 && tmp < 15)
            tests = tmp;
    }

    std::vector<std::thread> threads;
    std::deque<std::shared_ptr<Stats>> t_stats;
    for (size_t t = 0; t < tests; t++)
    {
        std::shared_ptr<Stats> stats = std::make_shared<Stats>();
        threads.push_back(std::thread(&test_func, stats, in_pins[t], out_pins[t], runs, t, &run_progress[t]));
        t_stats.push_back(stats);
    }
    std::thread pr_thread(&pr_func, run_progress, tests);
    pr_thread.join();

    for (size_t t = 0; t < tests; t++)
    {
        threads[t].join();
    }

    Stats glob_stats;
    for (size_t t = 0; t < tests; t++)
    {
        for (size_t i = 0; i < t_stats[t]->records_->size(); i++)
        {
            glob_stats.AddRecord(t_stats[t]->records_->at(i));
        }
    }

    std::cout << "\nrecords: " << glob_stats.records_->size() << std::endl; 
    std::cout << std::fixed << std::setprecision(3)
              << "\rmean: " << glob_stats.GetMean() << " std dev: " << glob_stats.GetStandardDeviation() << " median: " << glob_stats.GetPercentile(50)
              << " p25: " << glob_stats.GetPercentile(25) << " p75: " << glob_stats.GetPercentile(75)
              << " p2.5: " << glob_stats.GetPercentile(2.5) << " p97.5: " << glob_stats.GetPercentile(97.5) << std::endl;

    std::ofstream f;
    f.open("latencies.csv");
    f << "SetAndRead\n";
    f << std::fixed << std::setprecision(6);
    for (size_t i = 0; i < glob_stats.records_->size(); i++)
    {
        f << glob_stats.records_->at(i);
        f << "\n";
    }
    f.close();
    std::cout << "save latencies done " << std::endl;

	return 0;
}
