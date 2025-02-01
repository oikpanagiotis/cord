#ifndef HTTP_H
#define HTTP_H

#include <curl/curl.h>

#include "../core/errors.h"
#include "../core/memory.h"
#include "../core/strings.h"

typedef enum http_code_t { HTTP_OK = 200 } http_code_t;

typedef struct cord_http_client_t {
    CURL *curl;
    char *last_error;
    char *bot_token;
    cord_bump_t *allocator;
} cord_http_client_t;

enum { HTTP_GET, HTTP_POST, HTTP_DELETE, HTTP_PATCH };

typedef struct cord_url_builder_t {
    cord_strbuf_t *string_builder;
    cord_bump_t *allocator;
} cord_url_builder_t;

cord_url_builder_t cord_url_builder_create(cord_bump_t *allocator);
void cord_url_builder_add_route(cord_url_builder_t url_builder,
                                cord_str_t route);
cord_str_t cord_url_builder_build(cord_url_builder_t url_builder);

typedef struct cord_http_result_t {
    char *body;
    i32 status;
    bool error;
} cord_http_result_t;

typedef struct cord_http_request_t {
    i32 type;
    struct curl_slist *header;
    const char *body;
    const char *url;
    cord_http_result_t result;
} cord_http_request_t;

cord_http_client_t *cord_http_client_create(cord_bump_t *allocator,
                                            const char *bot_token);
void cord_http_client_destroy(cord_http_client_t *client);

int cord_http_client_perform_request(cord_http_client_t *client,
                                     cord_http_request_t *request);

cord_http_result_t cord_http_get(cord_http_client_t *client,
                                 cord_bump_t *allocator,
                                 cord_str_t url);

cord_http_result_t cord_http_post(cord_http_client_t *client,
                                  cord_bump_t *allocator,
                                  cord_str_t url,
                                  const char *body);

cord_http_result_t cord_http_delete(cord_http_client_t *client, cord_str_t url);

bool cord_http_is_success(cord_http_result_t result);

#endif
