#pragma once
#include <memory>
#include "AbstractExchangeController.hpp"
#include "BotType.hpp"
#include "BotConfig.hpp"

namespace ExchangeController
{

class ExchangeControllerFactory
{
    public:
    static std::unique_ptr<AbstractExchangeController> create(const Bot::BotConfig& config);
};

}