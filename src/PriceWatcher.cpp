#include <cmath>
#include "PriceWatcher.hpp"
#include "logger.hpp"

namespace Bot
{

PriceWatcher::PriceWatcher(TimeThresholdConfig config)
: _thresholdPercent(config.priceThresholdPercent)
, _timeThresholdExtrapoler(config.lowBound, config.highBound)
{
    _startTime = std::chrono::high_resolution_clock::now();
}

// profit: 1.2 for 20% profit
bool PriceWatcher::isMoving(double price, double profit)
{
    if(!_previousPriceIsInit)
    {
        LOG_DEBUG << "_previousPriceIsInit=" << _previousPriceIsInit << " . price=" << price << " _previousPrice=" << _previousPrice;
        _previousPrice = price;
        _previousPriceIsInit = true;
        return true;
    }
    
    const double movePercent = abs(price - _previousPrice)/_previousPrice;
    if(price > _previousPrice && movePercent > _thresholdPercent)
    {
        _startTime = std::chrono::high_resolution_clock::now();
        LOG_DEBUG << "movePercent=" << movePercent << " reset timer. price=" << price << " _previousPrice=" << _previousPrice;
        _previousPrice = price;
    }

    double timeThreshold = _timeThresholdExtrapoler.extrapolate(profit);
    //LOG_DEBUG << "profit=" << profit << " timeThreshold " << timeThreshold << " sec";

    if(auto durationMs = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now()-_startTime).count();
         durationMs < ( timeThreshold* 1000))
        return true;

    LOG_INFO << "price=" << price << " does not move since " << timeThreshold << " sec";
    return false;
}

}