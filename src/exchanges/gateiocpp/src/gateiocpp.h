#pragma once

#include <string>
#include <curl/curl.h>
#include "FixedPoint.hpp"
#include "tools/jsoncpp-1.8.3/json.h"

class GateIoCPP {
	public:
		using CurrencyPairsResult = std::string; // as CurrencyPairsResult is big, using a JSON make it too slow
		using SpotTickersResult = Json::Value;

		/* Names match with the api parameters, do not rename */
		enum class Side
		{
			buy,
			sell
		};

		/*
		- gtc: GoodTillCancelled
		- ioc: ImmediateOrCancelled, taker only
		- poc: PendingOrCancelled, makes a post-only order 
				that always enjoys a maker fee*/
		enum class TimeInForce
		{
			gtc,
			ioc,
			poc
		};

		/* Names match with the api parameters, do not rename */
		enum class Direction
		{
			to,
			from
		};

		 GateIoCPP(const std::string &api_key, const std::string &secret_key);
		 virtual ~GateIoCPP();
		 void cleanup();

		// Public API
		 void get_currency_pairs(CurrencyPairsResult &json_result) const;
		 void send_limit_order ( 
			const std::string& currency_pair,
			const Side side,
			const TimeInForce timeInForce,
			const Quantity& quantity,
			const Price& price,
			Json::Value &json_result ) const;
		 void get_spot_tickers(const std::string& currencyPair, SpotTickersResult &json_result) const;
		 void getOrderBook(const std::string& currencyPair, std::string &result) const;
		 void getAccountBalance(Json::Value &result) const;
		 void getSubAccountBalances(const std::string& uid, Json::Value &result) const;
		 void transferSubAnnounts(const std::string& currency
			, const std::string& subaccountUid
			, Direction direction
			, Quantity amount
			, Json::Value &result) const;

		private:
		std::string api_key = "";
		std::string secret_key = "";
		CURL* curl = NULL;

		 void curl_api( std::string &url, std::string &result_json ) const;
		 void curl_api_with_header(const std::string &url
			,const std::vector <std::string> &extra_http_header
			,const std::string &post_data
			,const std::string &action
			,std::string &str_result) const;
		 std::vector <std::string> generateSignedHttpHeader(const std::string& action
		, const std::string& prefix
		, const std::string& param
		, const std::string& body) const;

		template<class ResultType>
		void getGeneric(const std::string& url, ResultType &result) const;

};