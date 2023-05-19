#ifndef HTTP_H
#define HTTP_H

#include <curl/curl.h>

#include "../core/memory.h"
#include "../core/strings.h"
#include "../core/errors.h"

typedef struct cord_http_client_t {
    CURL *curl;
    char *last_error;
    char *bot_token;
    cord_bump_t *allocator;
} cord_http_client_t;

enum {
    HTTP_GET,
    HTTP_POST,
    HTTP_DELETE,
};

/*
    Ideal API
    The url builder takes memory from an arena that belongs to http_client_t
        cord_url_builder(client->allocator)
        cord_url_builder_add_route()
        cord_url_builder_build()

        cord_discord_rest_post(const char *fmt, data);
        cord_discord_rest_get
*/

typedef struct cord_url_builder_t {
    cord_strbuf_t *string_builder;
    cord_bump_t *allocator;
} cord_url_builder_t;

cord_url_builder_t cord_url_builder_create(cord_bump_t *allocator);
void cord_url_builder_add_route(cord_url_builder_t url_builder, cord_str_t route);
char *cord_url_builder_build(cord_url_builder_t url_builder);

typedef struct cord_http_request_t {
    int type;
    struct curl_slist *header;
    const char *body;
    const char *url;
} cord_http_request_t;

cord_http_client_t *cord_http_client_create(const char *bot_token);
void cord_http_client_destroy(cord_http_client_t *client);

int cord_http_client_perform_request(cord_http_client_t *client,
                                     cord_http_request_t *request);

cord_error_t cord_http_get(cord_http_client_t *client, const char *url);
cord_error_t cord_http_post(cord_http_client_t *client, const char *url, const char *body);
cord_error_t cord_http_delete(cord_http_client_t *client, const char *url);

#endif
