#pragma once
#include <string>
#include <curl/curl.h>
#include <functional>

namespace tools
{
    class Mail
    {
        public:
        struct Conf
        {
            std::string mailServer, login, password;
        };

        Mail(Conf conf);
        ~Mail();
        int sendmail(const std::string from, const std::string to, std::string subject);


        
        private:
            Conf _conf;

            struct upload_status 
            {
                size_t bytes_read;
            };
            struct upload_status upload_ctx = { 0 };
            CURL *curl;
    };
}