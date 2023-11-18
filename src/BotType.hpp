#pragma once
#include <algorithm>
#include <string>
#include "tools/LinearExtrapoler.hpp"
#include "tools/FixedPoint.hpp"
#include "tools/magic_enum.hpp"

namespace Bot
{

// The idea here is more the profit increasing,
// more the lossThreshold becoming small and less risky
struct PriceThreshold: public tools::Extrapolable
{
    double profit;
    double lossThreshold;

    double getX() const override { return profit;};
    double getY() const override { return lossThreshold;};
};

// The idea here is more the profit increasing,
// more the timeSec in position if price doesn't move becoming small and less risky
struct TimeThreshold: tools::Extrapolable
{
    double profit;
    unsigned int timeSec;

    double getX() const override { return profit;};
    double getY() const override { return timeSec;};
};

struct PriceThresholdConfig
{
    PriceThreshold lowBound, highBound;
};

struct TimeThresholdConfig
{
    double priceThresholdPercent;
    TimeThreshold lowBound, highBound;
};

struct MailConfig
{
    std::string mailServer, login, password, from, to;
};

enum class Exchange
{
    Gateio,
    Kucoin
};

enum class Status
{
    Success,
    Failure
};

struct ListingBotStatus
{
    enum class Status
    {
        Aborted,
        Ok,
        OpenPosition
    };

    Status status = Status::Aborted;
    Quantity pnl = 0, buyPrice = 0, sellPrice = 0, amount = 0;

    ListingBotStatus operator+(const ListingBotStatus &other ) const
    {
        return { .status = std::max(this->status, other.status)
                , .pnl = this->pnl + other.pnl
                , .buyPrice = this->buyPrice + other.buyPrice
                , .sellPrice = this->sellPrice + other.sellPrice
                , .amount = this->amount + other.amount};
    };

    ListingBotStatus& operator+=(const ListingBotStatus &other )
    {
        *this = *this + other;
        return *this;
    }
    std::string str();
    friend std::ostream& operator<<(std::ostream& os, const ListingBotStatus& other);
};

std::ostream& operator<<(std::ostream& os, const ListingBotStatus& status);

struct ApiKeys
{
    std::string pub;
    std::string secret;
    std::string passphrase;
};

enum class RunningMode
{
    Normal,
    WatchAndSell
};

}