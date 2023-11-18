#pragma once 

#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <string>
#include <unistd.h>
#include <sys/time.h>

namespace tools
{
    time_t get_current_epoch();
    unsigned long get_current_ms_epoch();

    std::string b2a_hex( char *byte_arr, int n );

    std::string hmac_sha256( const char *key, const char *data);
    std::string hmac_sha512( const char *key, const char *data);

    std::string sha256( const char *data );
    std::string sha512( const char *data );
}