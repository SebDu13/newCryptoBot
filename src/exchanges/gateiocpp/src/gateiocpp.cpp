#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <exception>

#include "gateiocpp.h"
#include "logger.hpp"
#include "magic_enum.hpp"
#include "sha.hpp"
#include "chrono.hpp"
#include "tools.hpp"

#define GATEIO_HOST "https://api.gateio.ws"

namespace{

void convertToJson(const std::string& input, Json::Value& output)
{
	try 
	{
		Json::Reader reader;
		output.clear();	
		reader.parse( input , output );
	} 
	catch ( std::exception &e ) 
	{
		LOG_ERROR <<  "Error ! " << e.what(); 
	}
}

size_t curl_cb( void *content, size_t size, size_t nmemb, std::string *buffer ) 
{	
	buffer->append((char*)content, size*nmemb);
	return size*nmemb;
}

}


GateIoCPP::GateIoCPP(const std::string &api_key, const std::string &secret_key ) 
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
	GateIoCPP::curl = curl_easy_init();
	GateIoCPP::api_key = api_key;
	GateIoCPP::secret_key = secret_key;
}

GateIoCPP::~GateIoCPP()
{
	cleanup();
}


void GateIoCPP::cleanup()
{
	curl_easy_cleanup(GateIoCPP::curl);
	curl_global_cleanup();
}

void GateIoCPP::get_currency_pairs(CurrencyPairsResult &result) const
{
	LOG_DEBUG;
	CHRONO_THIS_SCOPE;

	std::string url(GATEIO_HOST);  
	url += "/api/v4/spot/currency_pairs";

	curl_api(url,result) ;
}

template<class ResultType>
void GateIoCPP::getGeneric(const std::string& url, ResultType &json_result) const
{
	//CHRONO_THIS_SCOPE;

	std::string _url(GATEIO_HOST);
	_url += url;

	//LOG_DEBUG << "_url " << _url;

	std::string str_result;
	curl_api( _url, str_result );

	if ( !str_result.empty() ) 
	{	
		if constexpr(std::is_same<ResultType, Json::Value>::value)
		{
			convertToJson(str_result, json_result);
		}
		if constexpr(std::is_same<ResultType, std::string>::value)
		{
			json_result = std::move(str_result);
		}
	}
	else
		LOG_ERROR <<  "Failed to get anything.";
}

void GateIoCPP::getOrderBook(const std::string& currencyPair, std::string &result) const
{
	getGeneric("/api/v4/spot/order_book?currency_pair=" + currencyPair, result);
}

void GateIoCPP::get_spot_tickers(const std::string& currencyPair, SpotTickersResult &json_result) const
{
	getGeneric("/api/v4/spot/tickers?currency_pair=" + currencyPair, json_result);
}

void GateIoCPP::send_limit_order ( 
	const std::string& currency_pair, 
	const Side side,
	const TimeInForce timeInForce,
	const Quantity& quantity,
	const Price& price,
	Json::Value &json_result ) const
{
	//LOG_DEBUG;
	//CHRONO_THIS_SCOPE;

	if ( api_key.size() == 0 || secret_key.size() == 0 )
	{
		LOG_ERROR << "API Key and Secret Key has not been set.";
		return ;
	}

	std::string url(GATEIO_HOST);
	std::string prefix("/api/v4/spot/orders");
	url += prefix;

	Json::Value bodyJson;
	bodyJson["text"] = """";
	bodyJson["currency_pair"] = currency_pair;
	bodyJson["type"] = "limit";
	bodyJson["account"] = "spot";
	bodyJson["side"] = std::string(magic_enum::enum_name(side));
	bodyJson["iceberg"] = "0";
	bodyJson["amount"] = quantity.toStringExact();
	bodyJson["price"] = price.toStringExact();
	bodyJson["time_in_force"] = std::string(magic_enum::enum_name(timeInForce));
	bodyJson["auto_borrow"] = false;

	std::string body = bodyJson.toStyledString();

	//LOG_DEBUG << "url = " << url << " body = " << body;
	std::string action("POST");
	const auto httpHeader = generateSignedHttpHeader(action, prefix, "", body);

	LOG_DEBUG;
	std::string result;
	curl_api_with_header( url, httpHeader, body, action, result );
	if ( result.size() > 0 ) 
	{
		convertToJson(result, json_result);
	} 
	else 
		LOG_ERROR << "Failed to get anything.";
}

void GateIoCPP::getAccountBalance(Json::Value &result) const
{
	std::string url(GATEIO_HOST);
	std::string prefix("/api/v4/wallet/total_balance");
	url += prefix;
	std::string action("GET");
	std::string body;
	std::string stringResult;

	const auto httpHeader = generateSignedHttpHeader(action, prefix, "", body);
	curl_api_with_header( url, httpHeader, body, action, stringResult );

	//LOG_INFO << stringResult;

	if ( stringResult.size() > 0 ) 
	{
		convertToJson(stringResult, result);
	} 
	else 
		LOG_ERROR << "Failed to get anything.";
}

void GateIoCPP::getSubAccountBalances(const std::string& uid, Json::Value &result) const
{
	std::string url(GATEIO_HOST);
	std::string prefix("/api/v4/wallet/sub_account_balances");
	std::string param("sub_uid=" + uid); // 8630502
	url += prefix + "?" + param;
	std::string action("GET");
	std::string body;
	std::string stringResult;

	const auto httpHeader = generateSignedHttpHeader(action, prefix, param, body);
	curl_api_with_header( url, httpHeader, body, action, stringResult );

	//LOG_INFO << stringResult;

	if ( stringResult.size() > 0 ) 
	{
		convertToJson(stringResult, result);
	} 
	else 
		LOG_ERROR << "Failed to get anything.";
}

void GateIoCPP::transferSubAnnounts(const std::string& currency
	, const std::string& subaccountUid
	, Direction direction
	, Quantity amount
	, Json::Value &result) const
{
	if ( api_key.size() == 0 || secret_key.size() == 0 )
	{
		LOG_ERROR << "API Key and Secret Key has not been set.";
		return ;
	}

	if(amount.nearZero())
	{
		result.clear();
		return;
	}

	std::string url(GATEIO_HOST);
	std::string prefix("/api/v4/wallet/sub_account_transfers");
	std::string action("POST");
	url += prefix;

	Json::Value bodyJson;
	bodyJson["currency"] = currency;
	bodyJson["sub_account"] = subaccountUid;
	bodyJson["direction"] = std::string(magic_enum::enum_name(direction));;
	bodyJson["amount"] = amount.toString();
	bodyJson["sub_account_type"] = "spot";
	std::string body = bodyJson.toStyledString();

	std::string stringResult;

	const auto httpHeader = generateSignedHttpHeader(action, prefix, "", body);
	curl_api_with_header( url, httpHeader, body, action, stringResult );
	convertToJson(stringResult, result); // if ok, result is empty
}

void GateIoCPP::curl_api( std::string &url, std::string &result_json ) const
{
	curl_api_with_header( url , {}, "" , "GET", result_json );	
}

std::vector <std::string> GateIoCPP::generateSignedHttpHeader(const std::string& action
	, const std::string& prefix
	, const std::string& param
	, const std::string& body) const
{
	std::string bodyHash = tools::sha512(body.c_str());
	auto timeStamp = tools::get_current_epoch();

	std::stringstream sign;
	sign << action << std::endl;
	sign << prefix << std::endl;
	sign << param << std::endl; 
	sign << bodyHash << std::endl;
	sign << timeStamp;

	std::string signHash = tools::hmac_sha512(secret_key.c_str(), sign.str().c_str());

	return std::vector <std::string>{
		"Content-Type: application/json"
		,"Timestamp: " + std::to_string(timeStamp)
		,"KEY: " + api_key
		,"SIGN: " + signHash};
}

void GateIoCPP::curl_api_with_header(const std::string &url
			,const std::vector <std::string> &extra_http_header
			,const std::string &post_data
			,const std::string &action
			, std::string &str_result) const
{
	//CHRONO_THIS_SCOPE;
	CURLcode res;

	if( curl ) {

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str() );
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_cb);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str_result );
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_ENCODING, "gzip");
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		if ( extra_http_header.size() > 0 ) {
			
			struct curl_slist *chunk = NULL;
			for ( int i = 0 ; i < extra_http_header.size() ;i++ ) {
				chunk = curl_slist_append(chunk, extra_http_header[i].c_str() );
			}
			curl_easy_setopt(GateIoCPP::curl, CURLOPT_HTTPHEADER, chunk);
		}

		if ( action == "PUT" || action == "DELETE" ) 
			curl_easy_setopt(GateIoCPP::curl, CURLOPT_CUSTOMREQUEST, action.c_str() );

		if(!post_data.empty() || action == "POST")
			curl_easy_setopt(GateIoCPP::curl, CURLOPT_POSTFIELDS, post_data.c_str() );

		res = curl_easy_perform(GateIoCPP::curl);
		curl_easy_reset(curl); // reset the options

		/* Check for errors */ 
		if ( res != CURLE_OK ) {
			LOG_ERROR <<  "failed: " << curl_easy_strerror(res);
		} 	
	}
}



