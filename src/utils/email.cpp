#include <format>
#include <stdio.h>
#include <string.h>
#include <string>

#include <curl/curl.h>
#include <curl/easy.h>

#include "../include/hyperblock/utils.hpp"

namespace hyper_block {

struct upload_status {
    size_t bytes_read;
};

static size_t
payload_source(char const *payload_text, char *ptr, size_t size, size_t nmemb, void *userp)
{
    struct upload_status *upload_ctx = (struct upload_status *) userp;
    const char           *data;
    size_t                room = size * nmemb;

    if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1))
    {
        return 0;
    }

    data = &payload_text[upload_ctx->bytes_read];

    if (data)
    {
        size_t len = strlen(data);
        if (room < len)
            len = room;
        memcpy(ptr, data, len);
        upload_ctx->bytes_read += len;

        return len;
    }

    return 0;
}

int
send_email(std::string const &from_email, std::string const &to_email, std::string const &subject,
           std::string const &body, std::string const &smtp_user, std::string const &smtp_pwd,
           unsigned short starttls_port, std::string const &smtp_server)
{
    std::string user_pwd = smtp_user + ":" + smtp_pwd;

    CURL                *curl;
    CURLcode             res        = CURLE_OK;
    struct curl_slist   *recipients = NULL;
    struct upload_status upload_ctx = {0};

    curl = curl_easy_init();
    if (curl)
    {
        /* "smtp://mail.gandi.net"*/
        curl_easy_setopt(curl, CURLOPT_URL, smtp_server.data());
        curl_easy_setopt(curl, CURLOPT_PORT, starttls_port);
        curl_easy_setopt(curl, CURLOPT_USERPWD, user_pwd.data());
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from_email.data());

        recipients = curl_slist_append(recipients, to_email.data());

        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, [&](char *ptr, size_t size, size_t nmemb, void *userp) -> size_t {
            return payload_source(std::format("Date: Mon, 29 Nov 2010 21:54:29 +1100\r\n"
                                              "To: {}"
                                              "\r\n"
                                              "From: {}"
                                              "\r\n"
                                              "Subject: {}\r\n"
                                              "\r\n"
                                              "\r\n"
                                              "{}"
                                              "\r\n",
                                              from_email, to_email, subject, body)
                                      .data(),
                                  ptr, size, nmemb, userp);
        });
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        curl_slist_free_all(recipients);

        curl_easy_cleanup(curl);
    }

    return (int) res;
}

}   // namespace hyper_block
