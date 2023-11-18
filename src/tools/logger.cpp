#include "logger.hpp"
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/attributes/attribute.hpp>

using namespace boost::log;

void Logger::init(FilterLevel filterLevel, const std::string& pairId, bool withConsole, const std::string exchange) 
{
    add_common_attributes();

    if (filterLevel == FilterLevel::Debug)
    core::get()->set_filter(trivial::debug <= trivial::severity);
    else if (filterLevel == FilterLevel::Verbose)
    core::get()->set_filter(trivial::info <= trivial::severity);
    else
    core::get()->set_filter(trivial::warning <= trivial::severity);

    // log format: [TimeStamp] [Severity Level] Log message
    auto fmtTimeStamp = expressions::
    format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S:%f");
    auto fmtSeverity = expressions::
    attr<trivial::severity_level>("Severity");
    auto fmtThreadID = expressions::
    attr<attributes::current_thread_id::value_type>("ThreadID");

    formatter logFmt =
    expressions::format("[%1%] [%2%] [%3%] %4%")
    % fmtThreadID
    % fmtTimeStamp
    % fmtSeverity
    % expressions::smessage;

    if(withConsole)
    {
        auto console_sink = ::add_console_log(std::cout);
        console_sink->set_formatter(logFmt);
    }

    add_file_log(
    keywords::target = "logs/", keywords::file_name = exchange + "_%y%m%d_%3N_" + pairId + ".log",
    keywords::rotation_size = 30 * 1024 * 1024,
    keywords::scan_method = sinks::file::scan_matching,
    keywords::format = logFmt);
}
