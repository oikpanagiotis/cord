#ifndef HTTP_H
#define HTTP_H

#include <curl/curl.h>

typedef struct cord_http_client_t {
    CURL *curl;
    char *last_error;
    char *bot_token;
} cord_http_client_t;

enum {
    HTTP_GET,
    HTTP_POST,
    HTTP_DELETE,
};

typedef struct cord_http_request_t {
    int type;
    struct curl_slist *header;
    char *body;
    char *url;
} cord_http_request_t;

cord_http_request_t *cord_http_request_create(int type, char *url,
                                              struct curl_slist *header,
                                              char *content);

cord_http_client_t *cord_http_client_create(const char *bot_token);
void cord_http_client_destroy(cord_http_client_t *client);
struct curl_slist *cord_discord_api_header(cord_http_client_t *client);

int cord_http_client_perform_request(cord_http_client_t *client,
                                     cord_http_request_t *reqquest);

#endif
