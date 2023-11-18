#include "chrono.hpp"
#include "logger.hpp"

namespace tools
{

Chrono::Chrono(const char* fileName, const size_t line, const char* functionName):
fileName(fileName),
line(line),
functionName(functionName)
{
    startTime = std::chrono::high_resolution_clock::now();
}

Chrono::~Chrono()
{
    BOOST_LOG_SEV(Logger::log, boost::log::trivial::debug) 
        << LOG_LOCATION_PARAM(fileName, line, functionName) 
        << " executed in "
        << std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now()-startTime).count() << "ms";
}

}