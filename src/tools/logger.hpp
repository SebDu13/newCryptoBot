#pragma once

#include <boost/log/trivial.hpp>

#define JUST_FILENAME(f) std::max<const char*>(f,\
    std::max(strrchr(f, '\\')+1, strrchr(f, '/')+1))

#define LOG_LOCATION_PARAM(file, line, function) std::string("[") + JUST_FILENAME(file) + ":" + std::to_string(line) + "] [" + function + "] "
#define LOG_LOCATION LOG_LOCATION_PARAM(__FILE__, __LINE__, __FUNCTION__)

#define LOG_DEBUG   BOOST_LOG_SEV(Logger::log, boost::log::trivial::debug) << LOG_LOCATION
#define LOG_INFO    BOOST_LOG_SEV(Logger::log, boost::log::trivial::info) << LOG_LOCATION
#define LOG_WARNING BOOST_LOG_SEV(Logger::log, boost::log::trivial::warning) << LOG_LOCATION
#define LOG_ERROR   BOOST_LOG_SEV(Logger::log, boost::log::trivial::error) << LOG_LOCATION

struct Logger
{
	enum class FilterLevel
	{
		Verbose,
		Debug
	};

	inline static boost::log::sources::severity_logger<boost::log::trivial::severity_level> log;
	static void init(FilterLevel filterLevel, const std::string& pairId, bool withConsole, const std::string exchange);
};