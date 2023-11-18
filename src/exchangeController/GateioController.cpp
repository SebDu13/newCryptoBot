#include "exchangeController/GateioController.hpp"
#include "chrono.hpp"
#include "logger.hpp"
#include <optional>
#include "boost/lexical_cast.hpp"
#include "magic_enum.hpp"

namespace{
const std::string idPattern = "\"id\":\"";
const std::string basePattern = "\"base\":\"";
const std::string quotePattern = "\"quote\":\"";

GateIoCPP::Side convertFrom(ExchangeController::Side side)
{
    switch (side)
    {
    case ExchangeController::Side::buy: return GateIoCPP::Side::buy;
    case ExchangeController::Side::sell: return GateIoCPP::Side::sell;
    default:
        throw ExchangeController::ExchangeControllerException("Define " + std::string(magic_enum::enum_name(side)) + "here: GateIoCPP::Side convertFrom(ExchangeController::Side side)");
    }
}

ExchangeController::OrderStatus fillOrderStatus(const Json::Value& result)
{
    // error Json contain message field
    if(const auto& message = result.get("message", Json::Value()); !message.empty())
    {
        //"label" : "INVALID_CURRENCY",
        //"message" : "PIZA_USDT trade is disabled until 2021-11-05T18:00+08:00[Asia/Shanghai]"
        if(message.toStyledString().find("trade is disabled until", 0) != std::string::npos)
            return ExchangeController::OrderStatus::CurrencyNotAvailable;
        //"message" : "Invalid currency CURRENCY_NAME"
        if(message.toStyledString().find("Invalid currency", 0) != std::string::npos)
            return ExchangeController::OrderStatus::InvalidCurrency;
        //"message" : "Your order size 10 is too small. The minimum is 1 USDT"
        if(message.toStyledString().find("too small", 0) != std::string::npos)
            return ExchangeController::OrderStatus::SizeTooSmall;
        //"message" : "Not enough balance"
        if(message.toStyledString().find("Not enough balance", 0) != std::string::npos)
            return ExchangeController::OrderStatus::NotEnoughBalance;
        //"message" : "Your order size 1 is too large. The maximum is 1000000 USDT"
        if(message.toStyledString().find("is too large. The maximum is", 0) != std::string::npos)
            return ExchangeController::OrderStatus::OrderSizeTooLarge;
    }
    if(const auto message = result.get("status", Json::Value()); !message.empty())
    {
        if(message.toStyledString().find("cancelled", 0) != std::string::npos)
            return ExchangeController::OrderStatus::Cancelled;
        if(message.toStyledString().find("closed", 0) != std::string::npos)
            return ExchangeController::OrderStatus::Closed;
    }
    return ExchangeController::OrderStatus::Unknown;
}
}

namespace ExchangeController{

GateioController::GateioController(const Bot::ApiKeys& apiKeys):_gateIoAPI(apiKeys.pub, apiKeys.secret)
{
}

GateioController::~GateioController()
{
}

TickerResult GateioController::getSpotTicker(const std::string& currencyPair) const
{
    //CHRONO_THIS_SCOPE;
    GateIoCPP::SpotTickersResult result;
    _gateIoAPI.get_spot_tickers(currencyPair, result);
    TickerResult tickerResult;
    try
    {
        tickerResult = {boost::lexical_cast<double>(result[0]["last"].asString())
            , boost::lexical_cast<double>(result[0]["high_24h"].asString())
            , boost::lexical_cast<double>(result[0]["low_24h"].asString())
            , boost::lexical_cast<double>(result[0]["base_volume"].asString())
            , boost::lexical_cast<double>(result[0]["quote_volume"].asString())
            , boost::lexical_cast<double>(result[0]["lowest_ask"].asString())
            , boost::lexical_cast<double>(result[0]["highest_bid"].asString())};
    }
    catch(std::exception& e)
    {
        LOG_ERROR << "std::exception caught: " << e.what();
        sleep(1); // in case order api limit is reached
        return {};
    }
    catch(...)
    {
        LOG_ERROR << "Unknown exception caught";
        sleep(1);
        return {};
    }
    
    return tickerResult;
}

std::string GateioController::getOrderBook(const std::string& currencyPair) const
{
    std::string result;
    _gateIoAPI.getOrderBook(currencyPair,result);
    return result;
}

OrderResult GateioController::sendOrder(const std::string& currencyPair, const Side side, const Quantity& quantity, const Price& price) const
{
    Json::Value result;
    _gateIoAPI.send_limit_order(currencyPair, convertFrom(side), GateIoCPP::TimeInForce::ioc, quantity, price, result);
    const auto& status = fillOrderStatus(result);
    
    LOG_DEBUG << result;
        
    if( status == OrderStatus::Closed || status == OrderStatus::Cancelled)
        return {status
        , std::move(Quantity(result["fill_price"].asString()))
        , std::move(Quantity(result["filled_total"].asString()))
        , std::move(Quantity(result["amount"].asString()))
        , std::move(Quantity(result["fee"].asString())) };
    else
        return {status, Quantity(), Quantity(), Quantity(), Quantity()};
}

Quantity GateioController::computeMaxQuantity(const Price& price) const
{
    Json::Value result;
    _gateIoAPI.getAccountBalance(result);

    Json::Value result2;
    _gateIoAPI.getSubAccountBalances(GateioController::subAccountId, result2);
    LOG_INFO << "subAccount result: " << result2;

    if(result["details"]["spot"]["currency"] == "USDT")
    {
        Quantity quantity(result["details"]["spot"]["amount"].asString());
        LOG_INFO << "There is " << quantity << " USDT on spot account";
        const tools::FixedPoint percent("0.97");

        return Quantity{(quantity * percent)/price};
    }

    return Quantity{};
}

Quantity GateioController::getMainAccountBalance() const
{
    Json::Value result;
    _gateIoAPI.getAccountBalance(result);
    if(result["details"]["spot"]["currency"] == "USDT")
        return Quantity(result["details"]["spot"]["amount"].asString());
    else
        throw ExchangeController::ExchangeControllerException("Account amount is not in USDT. " + result.asString());
}

Quantity GateioController::getSubAccountBalance() const
{
    Json::Value result;
    _gateIoAPI.getSubAccountBalances(GateioController::subAccountId, result);

    return Quantity(result[0]["available"]["USDT"].asString());
}

Quantity GateioController::prepareAccount(const Price& price,const std::optional<Quantity>& maxAmountOpt, const std::optional<Quantity>& quantityOpt) const
{
    tools::FixedPoint percent{"0.98"};
    
    Quantity accountAmount = getMainAccountBalance();
    LOG_INFO << "There is " << accountAmount << " USDT on spot account";

    if(!maxAmountOpt && !quantityOpt)
    {
        Quantity subAccountAmount = getSubAccountBalance();
        LOG_INFO << "There is " << subAccountAmount << " USDT on sub account";

        Json::Value transferResult;
        _gateIoAPI.transferSubAnnounts("USDT", GateioController::subAccountId, GateIoCPP::Direction::from, subAccountAmount , transferResult);

        if(!transferResult.empty())
            throw ExchangeController::ExchangeControllerException("Issue when transfering money on subaccount" + transferResult.toStyledString());

        return Quantity{((accountAmount + subAccountAmount) * percent)/price};
    }

    Quantity quantity = quantityOpt.value_or(0);
    Quantity amountRequired = maxAmountOpt.value_or(quantity * price);
    LOG_INFO << "Amount required on USDT on spot account: " << amountRequired;

    // Not enough on main account
    if(Quantity AmountNeeded = amountRequired - accountAmount; AmountNeeded > 0)
    {
        Quantity subAccountAmount = getSubAccountBalance();
        LOG_INFO << "There is " << subAccountAmount << " USDT on sub account";

        AmountNeeded = (subAccountAmount < AmountNeeded ? subAccountAmount : AmountNeeded);
        Json::Value transferResult;
        LOG_INFO << "Transfering " << AmountNeeded << " USDT from sub account";
        _gateIoAPI.transferSubAnnounts("USDT", GateioController::subAccountId, GateIoCPP::Direction::from, AmountNeeded , transferResult);

        if(!transferResult.empty())
            throw ExchangeController::ExchangeControllerException("Issue when transfering money on subaccount" + transferResult.toStyledString());
        
        return Quantity{((accountAmount + AmountNeeded) * percent)/price};
    }
    // Too much on main account
    else if(Quantity extraAmount = accountAmount - amountRequired; extraAmount > 0)
    {
        Json::Value transferResult;
        LOG_INFO << "Transfering " << extraAmount << " USDT to sub account";
        _gateIoAPI.transferSubAnnounts("USDT", GateioController::subAccountId, GateIoCPP::Direction::to, extraAmount , transferResult);

        if(!transferResult.empty())
            throw ExchangeController::ExchangeControllerException("Issue when transfering money on subaccount" + transferResult.toStyledString());
    }

    return Quantity{(amountRequired * percent)/price};
}

Quantity GateioController::getMinOrderSize() const
{
    return Quantity{"1"};
}

Quantity GateioController::getAmountLeft(const OrderResult& buyOrderResult) const
{
    return buyOrderResult.amount - buyOrderResult.fee;
}

} /* end of namespace ExchangeController*/