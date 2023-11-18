#include <optional>
#include <math.h> 
#include "ListingBot.hpp"
#include "chrono.hpp"
#include "logger.hpp"
#include "magic_enum.hpp"
#include "tools.hpp"
#include "PriceWatcher.hpp"
#include "exchangeController/ExchangeControllerFactory.hpp"

namespace Bot
{

ListingBot::ListingBot(const BotConfig& botConfig, Quantity quantity)
: _exchangeController(ExchangeController::ExchangeControllerFactory::create(botConfig))
, _pairId(botConfig.getPairId())
, _limitBuyPrice(botConfig.getLimitBuyPrice())
, _quantity(quantity)
, _priceThresholdConfig(botConfig.getPriceThresholdConfig())
, _timeThreasholdConfig(botConfig.getTimeThresholdConfig())
, _runningMode(botConfig.getMode())
{
}

ListingBot::~ListingBot()
{
    if(_thread.joinable())
        _thread.join();
}

bool ListingBot::shouldStop()
{
    if(_stopFlag && *_stopFlag)
    {
        LOG_INFO << "_stopflag is set, stopping...";
        return true;
    }
    return false;
}

void ListingBot::notifyStop()
{
    if(_stopFlag)
        *_stopFlag = true;
}

ListingBotStatus ListingBot::run()
{
    ListingBotStatus status;
    std::optional<ExchangeController::OrderResult> buyOrderResult;

    if(_runningMode == RunningMode::WatchAndSell)
    {
        buyOrderResult = { .status = ExchangeController::OrderStatus::Closed
            , .fillPrice = _quantity * _limitBuyPrice
            , .filledTotal = _quantity * _limitBuyPrice
            , .amount = _quantity
            , .fee = Quantity("0")};
    }
    else
    {
        // Wait
        buyOrderResult = buySync();
        if(!buyOrderResult)
            return status;

        if(buyOrderResult->status != ExchangeController::OrderStatus::Closed)
        {
            LOG_ERROR << "Cannot buy " << _pairId << " because buyOrderResult.status=" << magic_enum::enum_name(buyOrderResult->status);
            return status;
        }
    }

    notifyStop();

    // Wait
    shouldSellSync(*buyOrderResult);
    
    ExchangeController::OrderResult sellOrderResult = sellAll(*buyOrderResult);

    if(sellOrderResult.status != ExchangeController::OrderStatus::Closed)
    {
        LOG_ERROR << "Cannot sell " << _pairId 
            << " because buyOrderResult.status=" << magic_enum::enum_name(sellOrderResult.status) 
            << " *** POSITION IS OPEN ***";
        status.status = ListingBotStatus::Status::OpenPosition;
        return status;
    }

    status.buyPrice = buyOrderResult->fillPrice / buyOrderResult->amount;
    status.sellPrice = sellOrderResult.fillPrice / sellOrderResult.amount;
    status.pnl = sellOrderResult.fillPrice - buyOrderResult->fillPrice - (buyOrderResult->fee * status.buyPrice) - sellOrderResult.fee;
    status.amount =  buyOrderResult->fillPrice;
    status.status = ListingBotStatus::Status::Ok;
    return status;
}

void ListingBot::runAsync(std::atomic<bool>* stopFlag, std::promise<ListingBotStatus> promise)
{
    _stopFlag = stopFlag;
    _thread = std::thread([this](std::promise<ListingBotStatus> promise){
        LOG_INFO << "Running asynchronously...";
        promise.set_value(this->run());
    }, std::move(promise));
}

std::optional<ExchangeController::OrderResult> ListingBot::buySync()
{
    ExchangeController::OrderResult buyOrderResult;
    do
    {
        CHRONO_THIS_SCOPE;
        
        if(shouldStop())
            return std::nullopt;

        if(tools::kbhit())
        {
            LOG_INFO << "key pressed, stopping...";

            notifyStop();
            //return std::nullopt;
        }

        buyOrderResult = _exchangeController->sendOrder(_pairId, ExchangeController::Side::buy , _quantity, _limitBuyPrice);

    } while (buyOrderResult.status == ExchangeController::OrderStatus::CurrencyNotAvailable);

    return buyOrderResult;
}

void  ListingBot::justBuy()
{
    LOG_INFO;
    _exchangeController->sendOrder(_pairId, ExchangeController::Side::buy , _quantity, _limitBuyPrice);
}


void ListingBot::shouldSellSync(const ExchangeController::OrderResult& buyOrderResult) const
{
    ExchangeController::TickerResult previousTickerResult;
    const Price purchasePrice = buyOrderResult.fillPrice / buyOrderResult.amount;
    Bot::PriceWatcher priceWatcher(_timeThreasholdConfig);
    tools::LinearExtrapoler priceThreasholdExtrapoler(_priceThresholdConfig.lowBound, _priceThresholdConfig.highBound);

    while(!tools::kbhit())
    {
        const ExchangeController::TickerResult tickerResult = _exchangeController->getSpotTicker(_pairId);
        const double profit = tickerResult.high24h/(double)purchasePrice;
        const double lossThreshold = priceThreasholdExtrapoler.extrapolate(profit);

        if(previousTickerResult != tickerResult)
        {
            LOG_INFO << "purchasePrice: " << purchasePrice
                    << " current PROFIT=" << ((tickerResult.last - (double)purchasePrice)/(double)purchasePrice) * 100 << "%"
                    << " current lossThreshold=" << lossThreshold
                    << " tickerResult " << tickerResult.toString();
            //LOG_INFO << "OrderBook: " << _exchangeController->getOrderBook(_pairId);
            previousTickerResult = tickerResult;
        }

        if(tickerResult.high24h == 0 || tickerResult.last == 0)
            continue;

        if(tickerResult.last < (tickerResult.high24h * lossThreshold))
        {
            LOG_INFO << "Price went under loss threshold(" << lossThreshold << "), stopping... ";
            return;
        }

        if(!priceWatcher.isMoving(tickerResult.last, profit))
        {
            LOG_INFO << "Price doesn't move anymore, stopping... ";
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    LOG_INFO << "key pressed, stopping...";
}

void ListingBot::watch() const
{
    ExchangeController::TickerResult previousTickerResult;
    Bot::PriceWatcher priceWatcher(_timeThreasholdConfig);
    tools::LinearExtrapoler priceThreasholdExtrapoler(_priceThresholdConfig.lowBound, _priceThresholdConfig.highBound);
    double profit = 1;
    while(!tools::kbhit())
    {
        const ExchangeController::TickerResult tickerResult = _exchangeController->getSpotTicker(_pairId);
        LOG_INFO << "tickerResult " << tickerResult.toString();
        /*if(previousTickerResult != tickerResult)
        {
            LOG_INFO << "tickerResult " << tickerResult.toString();
            //LOG_INFO << "jsonOrderBook " << _exchangeController->getOrderBook(_pairId);
            previousTickerResult = tickerResult;
        }

        LOG_DEBUG << "profit=" << profit << " priceThreshold=" << priceThreasholdExtrapoler.extrapolate(profit);

        if(!priceWatcher.isMoving(tickerResult.last, profit))
        {
            LOG_INFO << "Price doesn't move anymore, stopping... ";
            return;
        }

        profit += 0.1;*/
    }
}

ExchangeController::OrderResult ListingBot::sellAll(const ExchangeController::OrderResult& buyOrderResult)
{
    if(buyOrderResult.amount.value != 0)
    {
        Quantity amountLeft = _exchangeController->getAmountLeft(buyOrderResult);
        Price smallPrice = buyOrderResult.fillPrice / buyOrderResult.amount * tools::FixedPoint("0.2");

        // For small order smallPrice * amount can be lower than the exchange min order size and it fails
        if(smallPrice * amountLeft < _exchangeController->getMinOrderSize())
            smallPrice = (_exchangeController->getMinOrderSize() * tools::FixedPoint(1.1)) / amountLeft;

        LOG_INFO << "amount=" << amountLeft << " price=" << smallPrice;
        return _exchangeController->sendOrder(_pairId, ExchangeController::Side::sell, amountLeft, smallPrice);
    }
    return ExchangeController::OrderResult();
}

} /* end namespace Bot */ 