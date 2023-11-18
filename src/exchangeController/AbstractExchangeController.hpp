#pragma once
#include <string>
#include <stdexcept>
#include "magic_enum.hpp"
#include "BotType.hpp"

namespace ExchangeController
{
    class ExchangeControllerException : public std::runtime_error
    {
        public:
        ExchangeControllerException(const std::string what): std::runtime_error(what){}
    };

    enum class Side
    {
        buy,
        sell
    };

    enum class OrderStatus
    {
        Cancelled,
        Closed,
        SizeTooSmall,
        NotEnoughBalance,
        OrderSizeTooLarge,
        InvalidCurrency,
        CurrencyNotAvailable,
        Unknown
    };

    struct OrderResult
    {
        OrderStatus status = OrderStatus::Unknown;
        Quantity fillPrice;
        Quantity filledTotal;
        Quantity amount;
        Quantity fee;

        inline std::string toString() const
        {
            return std::string("status=" + std::string(magic_enum::enum_name(status))
            + " fillPrice=" + fillPrice.toString()
            + " filledTotal=" + filledTotal.toString()
            + " amount=" + amount.toString()
            + " fee=" + fee.toString());
        }

        bool operator==(OrderResult& other)
        {
            return status == other.status && fillPrice == other.fillPrice && filledTotal == other.filledTotal && amount == other.amount && fee == other.fee;
        }
    };

    struct TickerResult
    {
        double last;
        double high24h;
        double low24h;
        double baseVolume;
        double quoteVolume;
        double lowestAsk;
        double highestBid;

        inline std::string toString() const
        {
            return std::string("last=" + std::to_string(last)
            + " high24h=" + std::to_string(high24h)
            + " low24h=" + std::to_string(low24h)
            + " baseVolume=" + std::to_string(baseVolume)
            + " quoteVolume=" + std::to_string(quoteVolume)
            + " lowestAsk=" + std::to_string(lowestAsk)
            + " highestBid=" + std::to_string(highestBid));
        }

        bool operator==(const TickerResult& other) const
        {
            return last == other.last 
                && high24h == other.high24h 
                && low24h == other.low24h 
                && baseVolume == other.baseVolume 
                && quoteVolume == other.quoteVolume
                && lowestAsk == other.lowestAsk
                && highestBid == other.highestBid;
        }

        bool operator!=(const TickerResult& other) const
        {
            return !(*this==other);
        }
    };

    struct CurrencyPair
    {
        std::string id;
        std::string base;
        std::string quote; // in general USDT
        //fee
        //min_quote_amount
        //amount_precision
        //precision
        //trade_status
        //sell_start
        //buy_start

        inline std::string toString() const
        {
            return std::string("id=" + id + " base=" + base + " quote=" + quote);
        }
    };

class AbstractExchangeController
{
    public:
        virtual ~AbstractExchangeController(){};
        virtual TickerResult getSpotTicker(const std::string& currencyPair) const =0;
        virtual std::string getOrderBook(const std::string& currencyPair) const =0;
        virtual OrderResult sendOrder(const std::string& currency_pair, const Side side, const Quantity& quantity, const Price& price) const  =0;
        virtual Quantity computeMaxQuantity(const Price& price) const=0;
        // the Min order size (amount * price) the exchange accepts. For instance 1 on GateIO
        virtual Quantity getMinOrderSize() const = 0;
        virtual Quantity getAmountLeft(const OrderResult& buyOrderResult) const = 0;
        // Transfer the right amount on the trading account
        virtual Quantity prepareAccount(const Price& price,const std::optional<Quantity>& maxAmount, const std::optional<Quantity>& quantity) const = 0;
};

} /* end ExchangeController namespace */ 