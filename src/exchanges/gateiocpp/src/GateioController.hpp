#pragma once

#include <string>
#include <unordered_set>
#include "gateiocpp.h"
#include "exchangeController/AbstractExchangeController.hpp"

namespace ExchangeController{

class GateioController: public AbstractExchangeController
{
    public:
    GateioController(const Bot::ApiKeys& apiKeys);
    virtual ~GateioController();
    TickerResult getSpotTicker(const std::string& currencyPair) const override;
    std::string getOrderBook(const std::string& currencyPair) const override;
    OrderResult sendOrder(const std::string& currencyPair, const Side side, const Quantity& quantity, const Price& price) const override;
    Quantity computeMaxQuantity(const Price& price) const override;
    Quantity getMinOrderSize() const override;
    Quantity getAmountLeft(const OrderResult& buyOrderResult) const override;
    Quantity prepareAccount(const Price& price,const std::optional<Quantity>& maxAmount, const std::optional<Quantity>& quantity) const override;


    private:
    const std::string subAccountId = "8630502"; // a mettre dans la config ? 
    GateIoCPP _gateIoAPI;
    std::unordered_set<std::string> allCurrencyPairsCache;
    std::size_t rawCurrencyPairsResultSize;

    Quantity getMainAccountBalance() const;
    Quantity getSubAccountBalance() const;
};

} /* end ExchangeController namespace */ 