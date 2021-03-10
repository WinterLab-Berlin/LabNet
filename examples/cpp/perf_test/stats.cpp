#include "stats.h"
#include <vector>
#include <algorithm>
#include <cmath>

Stats::Stats()
{
    records_ = std::make_unique<std::deque<double>>();
    oldM_ = newM_ = oldS_ = newS_ = 0.0;
}

Stats::~Stats()
{
}

void Stats::AddRecord(double record)
{
    records_->push_front(record);

    // See Knuth TAOCP vol 2, 3rd edition, page 232
    if (records_->size() == 1)
    {
        min_ = record;
        max_ = record;
        oldM_ = newM_ = record;
        oldS_ = 0.0;
    }
    else
    {
        newM_ = oldM_ + (record - oldM_) / records_->size();
        newS_ = oldS_ + (record - oldM_) * (record - newM_);

        // set up for next iteration
        oldM_ = newM_;
        oldS_ = newS_;

        if (record < min_)
            min_ = record;
        else if (record > max_)
            max_ = record;
    }
}

int64_t Stats::NumRecords()
{
    return records_->size();
}

double Stats::GetPercentile(double percent)
{
    if (percent < 0)
        percent = 0;
    else if (percent > 100)
        percent = 100;

    std::vector<double> sorted(records_->begin(), records_->end());
    std::sort(sorted.begin(), sorted.end());
    
    int32_t rank = std::round(percent * sorted.size() / 100.0);
    if (rank > 0)
        rank--;

    return sorted[rank];
}

double Stats::GetMean()
{
    return (records_->size() > 0) ? newM_ : 0.0;
}

double Stats::GetVariance()
{
    return ((records_->size() > 1) ? newS_ / (records_->size() - 1) : 0.0);
}

double Stats::GetStandardDeviation()
{
    return std::sqrt(GetVariance());
}

double Stats::GetMin()
{
    return min_;
}

double Stats::GetMax()
{
    return max_;
}
