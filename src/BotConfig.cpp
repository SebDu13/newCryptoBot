#include "BotConfig.hpp"
#include <boost/program_options.hpp>
#include "logger.hpp"
#include "magic_enum.hpp"
#include <regex>
#include "FixedPoint.hpp"

namespace{

std::string printExchanges()
{
    std::stringstream stream;
    stream << "Exchanges supported:";
    for (auto name : magic_enum::enum_names<Bot::Exchange>())
        stream << " " << name;
    return stream.str();
}

std::string printModes()
{
    std::stringstream stream;
    stream << "RunningMode supported (default Normal):";
    for (auto name : magic_enum::enum_names<Bot::RunningMode>())
        stream << " " << name;
    return stream.str();
}

void convertCurrencyPair(std::string& pairId, Bot::Exchange exchange)
{
    switch (exchange)
    {
    case Bot::Exchange::Kucoin:
        std::replace( pairId.begin(), pairId.end(), '_', '-');
        break;

    case Bot::Exchange::Gateio:
        std::replace( pairId.begin(), pairId.end(), '-', '_');
        break;
    }
}

bool checkStartTime(const std::string& startTime)
{
     return std::regex_match(startTime, std::regex("^(0[0-9]|1[0-9]|2[0-3]):[0-5][0-9]$"));
}

}

namespace Bot
{

Status BotConfig::loadOptionsFromMain(int argc, char **argv)
{
    namespace po = boost::program_options;

    po::options_description desc("Allowed options");
    desc.add_options()
    ("help", "Print help")
    ("id", po::value<std::string>(), "Set new currency identifier. Ex: ETH_USDT")
    ("limitPrice", po::value<std::string>(), "Set limit price. Put a large price to have more chance to be executed")
    ("quantity", po::value<std::string>(), "Set quantity. If not set, it will use the total amount available on the wallet")
    ("withConsole", "Send logs on console")
    ("greedy", "Duration used to sell if the the price doesn't move and price thresholds are more permissive.")
    ("exchange", po::value<std::string>(), printExchanges().c_str())
    ("maxAmount", po::value<std::string>(), "Max amount to trade (in USDT)")
    ("startTime", po::value<std::string>(), "Start token listing time. hh:mm UTC")
    ("mode", po::value<std::string>(), printModes().c_str());

    po::variables_map vm;        
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm); 

    if (vm.count("help")) 
    {
        LOG_INFO << desc;
        return Status::Failure;
    }

    if (vm.count("id")) 
        _pairId = vm["id"].as<std::string>();
    else 
    {
        LOG_ERROR << "id was not set. --help for more details";
        return Status::Failure;
    }

    if (vm.count("limitPrice")) 
        _limitBuyPrice = Price(vm["limitPrice"].as<std::string>());
    else 
    {
        LOG_ERROR << "limitPrice was not set. --help for more details";
        return Status::Failure;
    }

    if (vm.count("quantity")) 
        _quantity = Quantity(vm["quantity"].as<std::string>());

    if (vm.count("withConsole")) 
        _withConsole = true;
    else
        LOG_INFO << "withConsole was set to " << false;
    
    if (vm.count("greedy")) 
        _greedyMode = true;

    if (vm.count("exchange"))
    {
        if(auto exchangeOpt = magic_enum::enum_cast<Bot::Exchange>(vm["exchange"].as<std::string>()))
            _exchange = *exchangeOpt;
        else
        {
            LOG_ERROR << "invalid exchange name. " << printExchanges();
            return Status::Failure;
        }
    }
    else 
    {
        LOG_ERROR << "exchange was not set. --help for more details";
        return Status::Failure;
    }

    if (vm.count("maxAmount")) 
        _maxAmount = Quantity(vm["maxAmount"].as<std::string>());

    if (vm.count("startTime")) 
    {
        _startTime = vm["startTime"].as<std::string>();
        if(!checkStartTime(_startTime))
        {
            LOG_ERROR << "Wrong startTime format. Should be hh:mm";
            return Status::Failure;
        }
    }
    else
    {
        LOG_ERROR << "startTime was not set. --help for more details";
        return Status::Failure;
    }

    if(vm.count("mode"))
    {
        if(auto modeOpt = magic_enum::enum_cast<Bot::RunningMode>(vm["mode"].as<std::string>()))
        {
            _runningMode = *modeOpt;

            if( _runningMode == RunningMode::WatchAndSell && !vm.count("quantity"))
            {
                LOG_ERROR << "quantity required with " << magic_enum::enum_name(_runningMode) << " mode";
                return Status::Failure;
            }
        }
        else
        {
            LOG_ERROR << "invalid mode name. " << printModes();
            return Status::Failure;
        }
    }

    setFixedPointPrecision();
    convertCurrencyPair(_pairId, _exchange);
    return Status::Success;
}

ApiKeys BotConfig::getApiKeys() const
{
    std::string apiKeyEnv, secretKeyEnv, passPhrase;

    switch(_exchange)
    {
        case Exchange::Gateio:
            apiKeyEnv = "GATEIO_K";
            secretKeyEnv = "GATEIO_S";
        break;

        case Exchange::Kucoin:
            apiKeyEnv = "KUCOIN_K";
            secretKeyEnv = "KUCOIN_S";
            passPhrase = "KUCOIN_P";
        break;
    }

    ApiKeys apiKeys;
    if(auto key = std::getenv(apiKeyEnv.c_str()))
        apiKeys.pub = std::string(key);
    else
        LOG_WARNING << apiKeyEnv << " not set or null";

    if(auto key = std::getenv(secretKeyEnv.c_str()))
        apiKeys.secret = std::string(key);
    else
        LOG_WARNING << secretKeyEnv << " not set or null";

    if(auto key = std::getenv(passPhrase.c_str()))
        apiKeys.passphrase = std::string(key);

    return apiKeys;
}

std::string BotConfig::toString() const
{
    std::stringstream stream;
    stream << "*** BotConfig: ***" << std::endl;
    stream << "exchange=" << magic_enum::enum_name(_exchange) << std::endl;
    stream << "pairId=" << _pairId << std::endl;
    stream << "startTime=" << _startTime << std::endl;
    stream << "limitBuyPrice=" << _limitBuyPrice.value << std::endl;
    if(_quantity) stream << "quantity=" << _quantity->toString() << std::endl;
    stream << "withConsole=" << _withConsole << std::endl;
    stream << "greedy=" << _greedyMode << std::endl;
    if(_maxAmount) stream << "maxAmount=" << *_maxAmount << std::endl;
    stream << "runningMode=" << magic_enum::enum_name(_runningMode) << std::endl;

    return stream.str();
}

TimeThresholdConfig BotConfig::getTimeThresholdConfig() const
{
    TimeThreshold lowBound, highBound;
    double thresholdPercent;
    if(_greedyMode)
    {
        lowBound.profit = 1.05;
        lowBound.timeSec=12;
        highBound.profit = 1.5;
        highBound.timeSec=1;
        thresholdPercent=0.05;
    }
    else
    {
        lowBound.profit = 1.05;
        lowBound.timeSec=10;
        highBound.profit = 1.2;
        highBound.timeSec=1;
        thresholdPercent=0.1;
    }
    return {.priceThresholdPercent=thresholdPercent, .lowBound = lowBound, .highBound = highBound};
}

PriceThresholdConfig BotConfig::getPriceThresholdConfig() const
{   
    PriceThreshold lowBound, highBound;
    if(_greedyMode)
    {
        lowBound.profit = 1.2;
        lowBound.lossThreshold=0.8;
        highBound.profit = 2;
        highBound.lossThreshold=0.9;
    }
    else
    {
        lowBound.profit = 1.05;
        lowBound.lossThreshold=0.8;
        highBound.profit = 1.8;
        highBound.lossThreshold=0.9;
    }
    return {.lowBound = lowBound, .highBound = highBound};
}

unsigned int BotConfig::getThreadNumber() const
{
    if(_runningMode == RunningMode::WatchAndSell)
        return 1;

    switch(_exchange)
    {
        case Exchange::Gateio:
        case Exchange::Kucoin:
        return 10;

        default:
        return 1;
    }
}

unsigned int BotConfig::getDurationBeforeStartMs() const
{
    if(_runningMode == RunningMode::WatchAndSell)
        return 0;

    const int defaultValue = 50; // milliseconds
    switch(_exchange)
    {
        case Exchange::Kucoin:
        return 20000 - defaultValue;

        default:
        return defaultValue;
    }
}

unsigned int BotConfig::getDelayBetweenBotsSpawnUs() const
{
    switch(_exchange)
    {
        case Exchange::Kucoin:
        return 10000; // 10ms

        default:
        return 500;
    }
}

void BotConfig::setFixedPointPrecision() const
{
    switch(_exchange)
    {
        case Exchange::Kucoin:
            tools::FixedPoint::minPrecision = 3;
            break;

        default:
            tools::FixedPoint::minPrecision = 4;
            break;
    }
}

MailConfig BotConfig::getMailConfig() const
{
    MailConfig mailConfig;
    std::string mailLoginEnv = "MAIL_LOGIN";
    std::string mailPassWordEnv = "MAIL_PASSWORD";

    if(auto key = std::getenv(mailLoginEnv.c_str()))
        mailConfig.login = std::string(key);
    else
        LOG_WARNING << mailLoginEnv << " not set or null";

    if(auto key = std::getenv(mailPassWordEnv.c_str()))
        mailConfig.password = std::string(key);
    else
        LOG_WARNING << mailPassWordEnv << " not set or null";

    mailConfig.mailServer = "smtp.gmail.com";
    mailConfig.from  = "<sebcrypto57@gmail.com>";
    mailConfig.to = "<sebastien.suignard@hotmail.fr>";

    return mailConfig;
}

}