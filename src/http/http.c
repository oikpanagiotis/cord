#include "http.h"
#include "../core/log.h"

#include <assert.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool is_curl_error(CURLcode code) {
    return code != CURLE_OK;
}

static const char *curl_error(CURLcode code) {
    return curl_easy_strerror(code);
}

cord_url_builder_t cord_url_builder_create(cord_bump_t *allocator) {
    return (cord_url_builder_t){.string_builder =
                                    cord_strbuf_create_with_allocator(allocator),
                                .allocator = allocator};
}

void cord_url_builder_add_route(cord_url_builder_t url_builder, cord_str_t route) {
    cord_strbuf_t *builder = url_builder.string_builder;
    bool is_first_route = builder->length == 0;

    if (!is_first_route) {
        cord_strbuf_append(builder, cstr("/"));
    }
    cord_strbuf_append(builder, route);
}

char *cord_url_builder_build(cord_url_builder_t url_builder) {
    return cord_strbuf_to_cstring(url_builder.string_builder);
}

cord_http_client_t *cord_http_client_create(const char *bot_token) {
    cord_http_client_t *client = malloc(sizeof(cord_http_client_t));
    if (!client) {
        logger_error("Failed to allocate http client");
        return NULL;
    }

    // FIXME: We never free this
    client->bot_token = strdup(bot_token);
    if (!client->bot_token) {
        logger_error("Failed to allocate bot_token");
        free(client);
        return NULL;
    }

    curl_global_init(CURL_GLOBAL_ALL);
    client->curl = curl_easy_init();
    if (!client->curl) {
        logger_error("Failed to init curl");
        free(client);
        curl_global_cleanup();
        return NULL;
    }
    curl_easy_setopt(client->curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

    return client;
}

void cord_http_client_destroy(cord_http_client_t *client) {
    if (client) {
        if (client->curl) {
            curl_easy_cleanup(client->curl);
        }
        if (client->bot_token) {
            free(client->bot_token);
        }
        free(client);
    }
    curl_global_cleanup();
}

struct curl_slist *discord_api_headers(const char *bot_token) {
    struct curl_slist *list = NULL;
    char auth[256] = {0};
    snprintf(auth, 256, "Authorization: Bot %s", bot_token);
    list = curl_slist_append(list, auth);
    list = curl_slist_append(list, "Accept: application/json");
    list = curl_slist_append(list, "charset: utf-8");
    list = curl_slist_append(list, "Content-type: application/json");

    return list;
}

cord_http_request_t *cord_http_request_create(int type, char *url, char *content) {
    cord_http_request_t *request = malloc(sizeof(cord_http_request_t));
    if (!request) {
        logger_error("Failed to allocate http request");
        return NULL;
    }

    request->url = url;
    request->header = NULL;
    request->type = type;
    request->body = content;
    return request;
}

static void log_request(cord_http_request_t *request) {
    static const char *types[] = {"GET", "POST", "DELETE"};

    logger_info("Performing HTTP %s to %s with %s", types[request->type],
                not_null_cstring_dash(request->url),
                not_null_cstring_dash(request->body));
}

static char *get_request_type_cstring(cord_http_request_t *request) {
    static char *GET = "GET";
    static char *POST = "POST";
    if (request->type == HTTP_GET) {
        return GET;
    } else if (request->type == HTTP_POST) {
        return POST;
    }
    return NULL;
}

static void prepare_request_options(cord_http_client_t *client,
                                    cord_http_request_t *request) {
    char *request_type = get_request_type_cstring(request);
    assert(request_type && "Request type can not be null");

    request->header = discord_api_headers(client->bot_token);
    curl_easy_setopt(client->curl, CURLOPT_CUSTOMREQUEST, request_type);
    curl_easy_setopt(client->curl, CURLOPT_URL, request->url);
    curl_easy_setopt(client->curl, CURLOPT_HTTPHEADER, request->header);
    if (request->type == HTTP_POST) {
        curl_easy_setopt(client->curl, CURLOPT_POSTFIELDS, request->body);
    }
}

int cord_http_client_perform_request(cord_http_client_t *client,
                                     cord_http_request_t *request) {
    prepare_request_options(client, request);
    // log_request(request);

    CURLcode rc = curl_easy_perform(client->curl);
    if (is_curl_error(rc)) {
        logger_error("HTTP %s Request failed: %s", curl_error(rc));
    }
    return (int)rc;
}
