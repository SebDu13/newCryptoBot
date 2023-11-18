#pragma once
#include <chrono>
#include "BotType.hpp"
#include "tools/LinearExtrapoler.hpp"

namespace Bot
{

class PriceWatcher
{
    public:
    PriceWatcher(TimeThresholdConfig config);

    //return false if the price doesn't move above thresholdPercent (0.1 for 10%)
    // since more than timeSec compare to last call.
    bool isMoving(double price, double profit);

    private:
    std::chrono::system_clock::time_point _startTime;
    double _thresholdPercent;
    tools::LinearExtrapoler _timeThresholdExtrapoler;
    bool _previousPriceIsInit = false;
    double _previousPrice;
};

}