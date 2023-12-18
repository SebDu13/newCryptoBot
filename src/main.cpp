#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <unistd.h>
#include <chrono>
#include <thread>


//#include "binacpp.h"
//#include "kucoincpp.hpp"
//#include "HuobiApi.hpp"
//#include "exchanges/gateiocpp/src/GateioController.hpp"
//#include "exchangeController/KucoinController.hpp"
#include "exchangeController/ExchangeControllerFactory.hpp"
#include "magic_enum.hpp"
#include "ListingBot.hpp"
#include "BotType.hpp"
#include "BotManager.hpp"

#include "logger.hpp"
#include "BotConfig.hpp"

#include <boost/multiprecision/cpp_dec_float.hpp>


int main(int argc, char **argv) 
{
	//using namespace Bot;

	Bot::BotConfig botConfig;
	if(botConfig.loadOptionsFromMain(argc, argv) == Bot::Status::Failure)
		return -1;
	
	if(botConfig.getMode() == Bot::RunningMode::TradingTime)
	{
		std::string errorMessage = ExchangeController::ExchangeControllerFactory::create(botConfig)->sendFakeOrder(botConfig.getPairId());
		std::cout << errorMessage << std::endl;
		return 0;
	}

	Logger::init(Logger::FilterLevel::Debug
		, botConfig.getPairId()
		, botConfig.getWithConsole()
		, std::string(magic_enum::enum_name(botConfig.getExchange())));

	LOG_INFO << botConfig.toString();

	Bot::BotManager botmanager(botConfig);
	botmanager.startOnTime();
	


	/* *** HUOBI *** */
	/*Json::Value result;
	HuobiApi huobiApi("","","");
	int counter = 0;
	while(counter < 5000)
	{
		counter++;
		huobiApi.getMexcTicker(botConfig.getPairId(), result);
		LOG_DEBUG << result;
	}*/

	return 0;	
}