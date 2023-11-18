#pragma once
#include <string>
#include <chrono>

#define CHRONO_THIS_SCOPE tools::Chrono chrono(__FILE__, __LINE__, __FUNCTION__)

namespace tools
{
    // should move this one in an other file
    class Chrono
    {
        public:
        Chrono(const char* fileName, const size_t  line, const char*  functionName);
        virtual ~Chrono();

        private:
        const char* fileName;
        const size_t line;
        const char* functionName;
        std::chrono::_V2::system_clock::time_point startTime;
    };
}
