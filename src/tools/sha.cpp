#include <string.h>
#include "sha.hpp"

namespace tools{

time_t get_current_epoch( ) {

    struct timeval tv;
    gettimeofday(&tv, NULL); 

    return tv.tv_sec ;
}

unsigned long get_current_ms_epoch( ) {

    struct timeval tv;
    gettimeofday(&tv, NULL); 

    return tv.tv_sec * 1000 + tv.tv_usec / 1000 ;

}

std::string b2a_hex( char *byte_arr, int n ) 
{

    const static std::string HexCodes = "0123456789abcdef";
    std::string HexString;
    for ( int i = 0; i < n ; ++i ) {
        unsigned char BinValue = byte_arr[i];
        HexString += HexCodes[( BinValue >> 4 ) & 0x0F];
        HexString += HexCodes[BinValue & 0x0F];
    }
    return HexString;
}

std::string hmac_sha256( const char *key, const char *data) 
{

    unsigned char* digest;
    digest = HMAC(EVP_sha256(), key, strlen(key), (unsigned char*)data, strlen(data), NULL, NULL);    
    return b2a_hex( (char *)digest, SHA256_DIGEST_LENGTH );
}

std::string hmac_sha512( const char *key, const char *data) 
{

    unsigned char* digest;
    digest = HMAC(EVP_sha512(), key, strlen(key), (unsigned char*)data, strlen(data), NULL, NULL);    
    return b2a_hex( (char *)digest, SHA512_DIGEST_LENGTH );
}  

std::string sha256( const char *data ) 
{
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, strlen(data) );
    SHA256_Final(digest, &sha256);
    return b2a_hex( (char *)digest, SHA256_DIGEST_LENGTH );
}

std::string sha512( const char *data ) 
{
    unsigned char digest[SHA512_DIGEST_LENGTH];
    SHA512_CTX sha512;
    SHA512_Init(&sha512);
    SHA512_Update(&sha512, data, strlen(data) );
    SHA512_Final(digest, &sha512);
    return b2a_hex( (char *)digest, SHA512_DIGEST_LENGTH );
}

} /* namespace tools */ 