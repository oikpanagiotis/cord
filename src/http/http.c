#include "http.h"
#include "../core/errors.h"
#include "../core/log.h"
#include "../string.h"

#include <assert.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool is_curl_error(CURLcode code) {
    return code != CURLE_OK;
}

static const char *curl_error(CURLcode code) {
    return curl_easy_strerror(code);
}

bool cord_http_is_success(cord_http_result_t result) {
    return result.status == 200;
}

cord_url_builder_t cord_url_builder_create(cord_bump_t *allocator) {
    return (cord_url_builder_t){.string_builder = cord_strbuf_create(),
                                .allocator = allocator};
}

void cord_url_builder_add_route(cord_url_builder_t url_builder,
                                cord_str_t route) {
    cord_strbuf_t *builder = url_builder.string_builder;
    bool is_first_route = builder->length == 0;

    if (!is_first_route) {
        cord_strbuf_append(builder, cstr("/"));
    }
    cord_strbuf_append(builder, route);
}

cord_str_t cord_url_builder_build(cord_url_builder_t url_builder) {
    return cord_strbuf_to_str(*url_builder.string_builder);
}

cord_http_client_t *cord_http_client_create(cord_bump_t *allocator,
                                            const char *bot_token) {
    cord_http_client_t *client = balloc(allocator, sizeof(cord_http_client_t));
    if (!client) {
        logger_error("Failed to allocate http client");
        return NULL;
    }
    client->allocator = cord_bump_create_with_size(KB(1));
    client->last_error = NULL;

    size_t token_buf_size = strlen(bot_token) + 1;
    client->bot_token = balloc(allocator, token_buf_size);
    if (!client->bot_token) {
        logger_error("Failed to allocate bot_token");
        free(client);
        return NULL;
    }
    memcpy(client->bot_token, bot_token, token_buf_size);

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
    }

    /*
     * The rest of the allocated values are going to be freed all at once
     * when permanent allocator is destroyed
     */

    curl_global_cleanup();
}

struct curl_slist *discord_api_headers(const char *bot_token) {
    struct curl_slist *list = NULL;
    char auth[256] = {0};
    snprintf(auth, 256, "Authorization: Bot %s", bot_token);
    list = curl_slist_append(list, auth);
    list = curl_slist_append(list, "Accept: application/json");
    list = curl_slist_append(list, "charset: utf-8");
    list = curl_slist_append(list, "Content-Type: application/json");

    return list;
}

static cord_http_request_t *cord_http_request_create(cord_bump_t *bump,
                                                     int type,
                                                     const char *url,
                                                     const char *body) {

    cord_http_request_t *request = balloc(bump, sizeof(cord_http_request_t));
    if (!request) {
        logger_error("Failed to allocate http request");
        return NULL;
    }

    request->url = url;
    request->header = NULL;
    request->type = type;
    request->body = body;
    request->result =
        (cord_http_result_t){.body = NULL, .status = 0, .error = false};

    return request;
}

static char *get_request_type_cstring(cord_http_request_t *request) {
    static char *GET = "GET";
    static char *POST = "POST";
    static char *DELETE = "DELETE";
    static char *PATCH = "PATCH";
    switch (request->type) {
        case HTTP_GET:
            return GET;
        case HTTP_POST:
            return POST;
        case HTTP_DELETE:
            return DELETE;
        case HTTP_PATCH:
            return PATCH;
        default:
            return NULL;
    }
}

static size_t write_cb(void *data, size_t size, size_t nmemb, void *udata) {
    cord_http_result_t *result = udata;
    assert(result);

    size_t string_size = nmemb * size;
    result->body = calloc(1, string_size + 1);
    memcpy(result->body, data, string_size);
    return size * nmemb;
}

static void prepare_request_with_headers(cord_http_client_t *client,
                                         cord_http_request_t *request,
                                         struct curl_slist *headers) {

    char *request_type = get_request_type_cstring(request);
    assert(request_type && "Request type can not be null");

    request->header = headers;
    curl_easy_setopt(client->curl, CURLOPT_CUSTOMREQUEST, request_type);
    curl_easy_setopt(client->curl, CURLOPT_URL, request->url);
    curl_easy_setopt(client->curl, CURLOPT_HTTPHEADER, request->header);

    assert(&request->result.body != NULL);
    curl_easy_setopt(client->curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(client->curl, CURLOPT_WRITEDATA, &request->result);

    if (request->type == HTTP_POST) {
        curl_easy_setopt(client->curl, CURLOPT_POSTFIELDS, request->body);
    }
}

static cord_http_result_t perform_with_headers(cord_http_client_t *client,
                                               cord_http_request_t *request,
                                               struct curl_slist *headers) {
    prepare_request_with_headers(client, request, headers);
    CURLcode rc = curl_easy_perform(client->curl);
    long *status = (long *)&request->result.status;
    curl_easy_getinfo(client->curl, CURLINFO_RESPONSE_CODE, status);
    if (request->result.status != HTTP_OK) {
        request->result.error = true;
    }

    if (is_curl_error(rc)) {
        logger_error("Could not perform HTTP %s request: %s", curl_error(rc));
    }
    return request->result;
}

static cord_http_result_t perform(cord_http_client_t *client,
                                  cord_http_request_t *request) {
    curl_easy_reset(client->curl);
    struct curl_slist *headers = discord_api_headers(client->bot_token);
    return perform_with_headers(client, request, headers);
}

cord_http_result_t cord_http_get(cord_http_client_t *client,
                                 cord_bump_t *allocator,
                                 cord_str_t url) {
    char *cstr_url = calloc(1, url.length + 1);
    memcpy(cstr_url, url.data, url.length);
    cord_http_result_t result = perform(
        client, cord_http_request_create(allocator, HTTP_GET, cstr_url, NULL));
    free(cstr_url);

    if (result.error) {
        logger_error("GET %s responded with status %d", url, result.status);
    }

    return result;
}

cord_http_result_t cord_http_post(cord_http_client_t *client,
                                  cord_bump_t *allocator,
                                  cord_str_t url,
                                  const char *body) {
    char *cstr_url = calloc(1, url.length + 1);
    memcpy(cstr_url, url.data, url.length);
    cord_http_result_t result = perform(
        client, cord_http_request_create(allocator, HTTP_POST, cstr_url, body));
    free(cstr_url);
    return result;
}

cord_http_result_t cord_http_delete(cord_http_client_t *client,
                                    cord_str_t url) {
    char *cstr_url = calloc(1, url.length + 1);
    memcpy(cstr_url, url.data, url.length);
    cord_http_result_t result =
        perform(client,
                cord_http_request_create(
                    client->allocator, HTTP_DELETE, cstr_url, NULL));
    free(cstr_url);
    return result;
}

cord_http_result_t cord_http_patch(cord_http_client_t *client, cord_str_t url) {
    char *cstr_url = calloc(1, url.length + 1);
    memcpy(cstr_url, url.data, url.length);
    cord_http_result_t result =
        perform(client,
                cord_http_request_create(
                    client->allocator, HTTP_PATCH, cstr_url, NULL));
    free(cstr_url);
    return result;
}
