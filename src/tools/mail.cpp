#include <stdio.h>
#include <string.h>
#include "mail.hpp"

namespace
{

struct upload_status 
{
    size_t bytes_read;
};

static char *payload_text;

size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp)
{
    struct upload_status *upload_ctx = (struct upload_status *)userp;
    const char *data;
    size_t room = size * nmemb;

    if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) 
    {
        return 0;
    }
    data = &payload_text[upload_ctx->bytes_read];

    if(data) 
    {
        size_t len = strlen(data);
        if(room < len)
            len = room;
        memcpy(ptr, data, len);
        upload_ctx->bytes_read += len;

        return len;
    }
    return 0;
};

}
 
namespace tools
{

Mail::Mail(Conf conf): _conf(conf), curl(curl_easy_init())
{
}

Mail::~Mail()
{
    curl_easy_cleanup(curl);
}

int Mail::sendmail(const std::string from, const std::string to, std::string subject)
{
    std::string text =
    "To: " + to + "\r\n"
    "From: " + from + "\r\n"
    "Subject: " + subject + "\r\n";
    // grosse dégeulasserie
    ::payload_text = const_cast<char*>(text.c_str());

    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;

    if(curl) 
    {
        /* Set username and password */
        curl_easy_setopt(curl, CURLOPT_USERNAME, _conf.login.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, _conf.password.c_str());

        //curl_easy_setopt(curl, CURLOPT_TLSAUTH_USERNAME, "seb.crypto@hotmail.com");
        //curl_easy_setopt(curl, CURLOPT_TLSAUTH_PASSWORD, "crypto123");

        /* This is the URL for your mailserver. Note the use of smtps:// rather
            * than smtp:// to request a SSL based connection. */
        curl_easy_setopt(curl, CURLOPT_URL, _conf.mailServer.c_str());
        curl_easy_setopt(curl, CURLOPT_PORT, 587);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);

        /* If you want to connect to a site who is not using a certificate that is
            * signed by one of the certs in the CA bundle you have, you can skip the
            * verification of the server's certificate. This makes the connection
            * A LOT LESS SECURE.
            *
            * If you have a CA cert for the server stored someplace else than in the
            * default bundle, then the CURLOPT_CAPATH option might come handy for
            * you. */
        #ifdef SKIP_PEER_VERIFICATION
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        #endif

        /* If the site you are connecting to uses a different host name that what
            * they have mentioned in their server certificate's commonName (or
            * subjectAltName) fields, libcurl will refuse to connect. You can skip
            * this check, but this will make the connection less secure. */
        #ifdef SKIP_HOSTNAME_VERIFICATION
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        #endif

        /* Note that this option is not strictly required, omitting it will result
            * in libcurl sending the MAIL FROM command with empty sender data. All
            * autoresponses should have an empty reverse-path, and should be directed
            * to the address in the reverse-path which triggered them. Otherwise,
            * they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
            * details.
            */
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from.c_str());

        /* Add two recipients, in this particular case they correspond to the
            * To: and Cc: addressees in the header, but they could be any kind of
            * recipient. */
        recipients = curl_slist_append(recipients, to.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        /* We are using a callback function to specify the payload (the headers and
            * body of the message). You could just use the CURLOPT_READDATA option to
            * specify a FILE pointer to read from. */
        
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        /* Since the traffic will be encrypted, it is very useful to turn on debug
            * information within libcurl to see what is happening during the
            * transfer */
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        /* Send the message */
        res = curl_easy_perform(curl);

        /* Check for errors */
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        /* Free the list of recipients */
        curl_slist_free_all(recipients);
    }

    return (int)res;
}

}