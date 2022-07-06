#pragma once

#include <deque>
#include <memory>

class Stats
{
public:
    Stats();
    ~Stats();

    void AddRecord(double record);
    int64_t NumRecords();
    
    // nearest-rank method
    double GetPercentile(double percent);

    double GetMean();
    double GetVariance();
    double GetStandardDeviation();
    double GetMin();
    double GetMax();

    std::unique_ptr<std::deque<double>> records_;

private:
    double oldM_, newM_, oldS_, newS_;
    double min_, max_;
};
