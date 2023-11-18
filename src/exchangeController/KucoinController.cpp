#include "exchangeController/KucoinController.hpp"
#include "chrono.hpp"
#include "logger.hpp"
#include <optional>
#include "boost/lexical_cast.hpp"
#include "magic_enum.hpp"

namespace {

KucoinCPP::Side convertFrom(ExchangeController::Side side)
{
    switch (side)
    {
    case ExchangeController::Side::buy: return KucoinCPP::Side::buy;
    case ExchangeController::Side::sell: return KucoinCPP::Side::sell;
    default:
        throw ExchangeController::ExchangeControllerException("Define " + std::string(magic_enum::enum_name(side)) + "here: GateIoCPP::Side convertFrom(ExchangeController::Side side)");
    }
}

ExchangeController::OrderStatus fillOrderStatus(const Json::Value& result)
{
    // error Json contain message field
    if(const auto message = result.get("code", Json::Value()); !message.empty())
    {
        if(message.toStyledString().find("400600", 0) != std::string::npos)
            return ExchangeController::OrderStatus::CurrencyNotAvailable;
        if(message.toStyledString().find("400200", 0) != std::string::npos) // Forbidden to place an order. Sometime there is this message. Should i keep that ? 
            return ExchangeController::OrderStatus::CurrencyNotAvailable;
        if(message.toStyledString().find("200002", 0) != std::string::npos) // "Too many requests in a short period of time, please retry later"
            return ExchangeController::OrderStatus::CurrencyNotAvailable;
        if(message.toStyledString().find("200000", 0) != std::string::npos)
            return ExchangeController::OrderStatus::Closed;
        if(message.toStyledString().find("900001", 0) != std::string::npos)
            return ExchangeController::OrderStatus::InvalidCurrency;
        if(message.toStyledString().find("400100", 0) != std::string::npos)
            return ExchangeController::OrderStatus::SizeTooSmall;
        if(message.toStyledString().find("200004", 0) != std::string::npos)
            return ExchangeController::OrderStatus::NotEnoughBalance;
        // order cancel if limit price too high ??
    }
    return ExchangeController::OrderStatus::Unknown;
}
}

namespace ExchangeController
{

KucoinController::KucoinController(const Bot::ApiKeys& apiKeys)
: _kucoinAPI(apiKeys.pub, apiKeys.secret, apiKeys.passphrase)
{}

KucoinController::~KucoinController()
{
}

TickerResult KucoinController::getSpotTicker(const std::string& currencyPair) const
{
    //CHRONO_THIS_SCOPE;
    static size_t failureCount = 0;
    static double high_24h = 0; // artificial because not suported by the api but required
    Json::Value result;
    _kucoinAPI.getTicker(currencyPair, result);

    if(result.get("code", Json::Value()).empty() 
        || result.get("data", Json::Value()).empty() 
        || result["code"] != "200000")
    {
        if(++failureCount > 3)
            throw ExchangeControllerException("KucoinController::getSpotTicker cannot get Ticker *** POSITION IS OPEN ***");
        return {};
    }
    failureCount = 0;

    const auto& data = result["data"];
    double last = boost::lexical_cast<double>(data["price"].asString());

    if(last > high_24h)
        high_24h = last;

    return { .last = last
        , .high24h = high_24h
        , .low24h = 0
        , .baseVolume = 0
        , .quoteVolume = 0
        , .lowestAsk = boost::lexical_cast<double>(data["bestAsk"].asString())
        , .highestBid = boost::lexical_cast<double>(data["bestBid"].asString())};
}

std::string KucoinController::getOrderBook(const std::string& ) const
{
    return "";
}

OrderResult KucoinController::sendOrder(const std::string& currencyPair, const Side side, const Quantity& quantity, const Price& price) const
{
    Json::Value result, resultOrderInfo;
    _kucoinAPI.sendLimitOrder(currencyPair, convertFrom(side), KucoinCPP::TimeInForce::IOC, quantity, price, result);

    const auto& status = fillOrderStatus(result);

    LOG_DEBUG << result;

    if(status != OrderStatus::Closed)
        return {status, Quantity(), Quantity(), Quantity(), Quantity()};

    std::string code, cancelExist, dealFunds, dealSize, fee;
    size_t tryNumber = 0;
    do
    {
        sleep(2);
        try
        {
            _kucoinAPI.getOrder(result["data"]["orderId"].asString(), resultOrderInfo);
            LOG_DEBUG << resultOrderInfo << std::endl << "tryNumber=" << tryNumber;
            const auto& data = resultOrderInfo["data"];
            
            // vérifier que "code == 200000" ??
            code = resultOrderInfo["code"].asString();
            cancelExist = data["cancelExist"].asString();
            dealFunds = data["dealFunds"].asString();
            dealSize = data["dealSize"].asString();
            fee = data["fee"].asString();
            ++tryNumber;

        }
        catch(...)
        {
            LOG_ERROR << "Unknown exception caught... resultOrderInfo=" << resultOrderInfo;
            continue;
        }

    } while (code == "200000" && cancelExist == "false" && (dealFunds == "0" || dealSize == "0"));

    return { (cancelExist == "true" ? OrderStatus::Cancelled: status)
    , Quantity(dealFunds)
    , Quantity(std::move(dealFunds))
    , Quantity(std::move(dealSize))
    , Quantity(std::move(fee)) };
}

    Quantity KucoinController::computeMaxQuantity(const Price& price) const
    {
        Json::Value result;
        _kucoinAPI.getAccountBalances(result);
        for(const auto& account : result["data"])
        {
            if(account["currency"] == "USDT")
            {
                Quantity quantity(account["available"].asString());
                const tools::FixedPoint percent("0.97");

                return Quantity{(quantity * percent)/price};
            }
        }
        LOG_DEBUG << result;
        return Quantity();
    }

    Quantity KucoinController::prepareAccount(const Price& price,const std::optional<Quantity>& maxAmount, const std::optional<Quantity>& quantity) const
    {
        return computeMaxQuantity(price); // account transfer to do
    }

    Quantity KucoinController::getMinOrderSize() const
    {
        return Quantity{"1"};
    }

    Quantity KucoinController::getAmountLeft(const OrderResult& buyOrderResult) const
    {
        return buyOrderResult.amount;
    }

}