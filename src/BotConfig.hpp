#pragma once
#include <string>
#include <vector>
#include <optional>
#include "BotType.hpp"

namespace Bot
{
    class BotConfig
    {
        public:
        Status loadOptionsFromMain(int argc, char **argv);

        std::string getPairId() const {return _pairId;};
        Price getLimitBuyPrice() const {return _limitBuyPrice;};
        std::optional<Quantity> getQuantity() const {return _quantity;};
        ApiKeys getApiKeys() const;
        bool getWithConsole() const {return _withConsole;};
        std::string toString() const;
        TimeThresholdConfig getTimeThresholdConfig() const;
        PriceThresholdConfig getPriceThresholdConfig() const;
        Exchange getExchange() const {return _exchange;};
        std::optional<Quantity> getMaxAmount() const {return _maxAmount;};
        std::string getStartTime() const {return _startTime;};
        RunningMode getMode() const {return _runningMode;};
        unsigned int getThreadNumber() const;
        unsigned int getDurationBeforeStartMs() const;
        unsigned int getDelayBetweenBotsSpawnUs() const;
        MailConfig getMailConfig() const;

        private:
        std::string _pairId = "";
        Price _limitBuyPrice = Price(); // generally large
        std::optional<Quantity> _quantity = std::nullopt;
        bool _withConsole = false;
        bool _greedyMode = false; // good for token not listed on any exchange
        Exchange _exchange;
        std::optional<Quantity> _maxAmount = std::nullopt;
        std::string _startTime ="";
        RunningMode _runningMode = RunningMode::Normal;
        
        void setFixedPointPrecision() const;
    };
}