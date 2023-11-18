#pragma once

#include <string>
#include "kucoincpp.hpp"
#include "exchangeController/AbstractExchangeController.hpp"

namespace ExchangeController{

class KucoinController: public AbstractExchangeController
{
    public:
    KucoinController(const Bot::ApiKeys& apiKeys);
    virtual ~KucoinController();
    TickerResult getSpotTicker(const std::string& currencyPair) const override;
    std::string getOrderBook(const std::string& currencyPair) const override;
    OrderResult sendOrder(const std::string& currencyPair, const Side side, const Quantity& quantity, const Price& price) const override;
    Quantity computeMaxQuantity(const Price& price) const override;
    Quantity getMinOrderSize() const override;
    Quantity getAmountLeft(const OrderResult& buyOrderResult) const override;
    Quantity prepareAccount(const Price& price,const std::optional<Quantity>& maxAmount, const std::optional<Quantity>& quantity) const override;

    private:
    KucoinCPP _kucoinAPI;
};
}