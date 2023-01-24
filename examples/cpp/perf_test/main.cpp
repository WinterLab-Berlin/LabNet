#include <iostream>
#include <chrono>
#include <wiringPi.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <random>
#include "stats.h"

using namespace std::chrono_literals;

struct pin_test_data
{
    int in_pin; // response pin
    int out_pin; // test pin
    int state; // signal state: 0 - wait for offset and turn test pin on, 1 - wait until response pin is on
    double offset; // wait for this time and turn test pin on
    std::chrono::system_clock::time_point time; // store test pin on time
    std::vector<double> latencies; // store all latencies
};

int main(int argc, char *argv[])
{
    int err = wiringPiSetup();
    if (err == -1)
    {
        std::cerr << "wiringPi init failed\n";
        exit(1);
    }

    int in_pins[] = {29, 28, 27, 26, 31, 11, 10, 6, 5, 4, 1, 16, 15, 8}; // all response pins
    int out_pins[] = {25, 24, 23, 22, 21, 30, 14, 13, 12, 3, 2, 0, 7, 9}; // all test pins
    int runs = 10000; // test runs on a pin
    int tests = 1; // number of pins to test
    
    if(argc > 1)
    {
        int tmp = atoi(argv[1]);
        if(tmp > 0 && tmp < 15)
            tests = tmp;
    }

    int all_runs_max = runs * tests;
    int all_runs = 0;
    std::random_device rd;  // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(1000, 3000); // offset range

    std::vector<pin_test_data> pin_tests;
    for (size_t t = 0; t < tests; t++)
    {
        pin_test_data tmp;
        tmp.in_pin = in_pins[t];
        tmp.out_pin = out_pins[t];
        tmp.state = 0;
        tmp.offset = distr(gen);
        pin_tests.push_back(tmp);

        pinMode(out_pins[t], OUTPUT);
        digitalWrite(out_pins[t], 0);
        pinMode(in_pins[t], INPUT);
    }
    
    std::chrono::system_clock::time_point start_time = std::chrono::high_resolution_clock::now();
    std::chrono::system_clock::time_point time;
    std::chrono::duration<double, std::micro> dur;
    double running_time_micro;
    int test_done = 0;
    int pin_state;
    int progress = 0;
    while (test_done < tests)
    {
        time = std::chrono::high_resolution_clock::now();
        dur = time - start_time;
        running_time_micro = dur.count();

        for (size_t t = 0; t < tests; t++)
        {
            if(pin_tests[t].state == 0 && pin_tests[t].offset < running_time_micro) // set test signal
            {
                //std::cout << "set: " << running_time_ms << std::endl;
                digitalWrite(pin_tests[t].out_pin, 1);
                pin_tests[t].state = 1;
                pin_tests[t].time = std::chrono::high_resolution_clock::now();
            }
        }

        for (size_t t = 0; t < tests; t++)
        {
            if(pin_tests[t].state == 1)
            {
                pin_state = digitalRead(pin_tests[t].in_pin);
                if(pin_state == 1) // response signal
                {
                    time = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double, std::milli> latency = time - pin_tests[t].time;
                    pin_tests[t].latencies.push_back(latency.count());
                    all_runs++;

                    digitalWrite(pin_tests[t].out_pin, 0);
                    if (pin_tests[t].latencies.size() < runs)
                    {
                        pin_tests[t].state = 0;
                        pin_tests[t].offset = running_time_micro + distr(gen);
                    }
                    else
                    {
                        pin_tests[t].state = 255;
                        test_done++;
                    }
                }
                else if(pin_tests[t].offset + 100000 < running_time_micro) // skip the run, if no response within 100ms
                {
                    digitalWrite(pin_tests[t].out_pin, 0);
                    pin_tests[t].state = 0;
                    pin_tests[t].offset = running_time_micro + distr(gen);
                }
            }
        }

        int np = (int)(((float)all_runs / all_runs_max) * 100.0);
        if(np != progress)
        {
            progress = np;
            std::cout << "\r                    \r" << "progress " << progress << "\%" << std::flush;
        }
    }
    time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> total_dur = time - start_time;
    double total_dur_sec = total_dur.count() / 1000.0;
    double events_per_sec = all_runs_max / total_dur_sec;
    std::cout << std::fixed << std::setprecision(3)
              << "\ntotal run time: " << total_dur_sec << "sec\tevents per second: " << events_per_sec << std::endl;

    Stats glob_stats;
    for (size_t t = 0; t < tests; t++)
    {
        for (size_t i = 0; i < pin_tests[t].latencies.size(); i++)
        {
            glob_stats.AddRecord(pin_tests[t].latencies.at(i));
        }
    }

    std::cout << "records: " << glob_stats.records_->size() << std::endl;
    std::cout << std::fixed << std::setprecision(5)
                << "\rmean: " << glob_stats.GetMean() << " std dev: " << glob_stats.GetStandardDeviation() << " median: " << glob_stats.GetPercentile(50)
                << " p25: " << glob_stats.GetPercentile(25) << " p75: " << glob_stats.GetPercentile(75)
                << " p2.5: " << glob_stats.GetPercentile(2.5) << " p97.5: " << glob_stats.GetPercentile(97.5) << std::endl;

    std::ofstream f;
    f.open("latencies.csv");
    f << "SetAndRead\n";
    f << "tests: " << tests << "\n";
    f << "records: " << glob_stats.records_->size() << "\n";
    f << "total run time: " << total_dur_sec << "\n";
    f << "events per second: " << events_per_sec << "\n";

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
