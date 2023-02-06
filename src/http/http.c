#include "http.h"
#include "../core/log.h"

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

cord_http_client_t *cord_http_client_create(const char *bot_token) {
    cord_http_client_t *client = malloc(sizeof(cord_http_client_t));
    if (!client) {
        logger_error("Failed to allocate http client");
        return NULL;
    }

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

struct curl_slist *cord_discord_api_header(cord_http_client_t *client) {
    struct curl_slist *list = NULL;
    char auth[256] = {0};
    snprintf(auth, 256, "Authorization: Bot %s", client->bot_token);
    list = curl_slist_append(list, auth);
    list = curl_slist_append(list, "Accept: application/json");
    list = curl_slist_append(list, "charset: utf-8");
    list = curl_slist_append(list, "Content-type: application/json");

    return list;
}

cord_http_request_t *cord_http_request_create(int type, char *url,
                                              struct curl_slist *headers,
                                              char *content) {
    cord_http_request_t *req = malloc(sizeof(cord_http_request_t));
    if (!req) {
        logger_error("Failed to allocate http request");
        return NULL;
    }

    req->url = url;
    req->header = headers;
    req->type = type;
    req->body = content;
    return req;
}

int cord_http_client_perform_request(cord_http_client_t *client,
                                     cord_http_request_t *req) {
    static char *GET = "GET";
    static char *POST = "POST";
    char *request_type = NULL;
    if (req->type == HTTP_GET) {
        request_type = GET;
    } else if (req->type == HTTP_POST) {
        request_type = POST;
    }

    curl_easy_setopt(client->curl, CURLOPT_CUSTOMREQUEST, request_type);
    curl_easy_setopt(client->curl, CURLOPT_URL, req->url);
    curl_easy_setopt(client->curl, CURLOPT_HTTPHEADER, req->header);
    if (req->type == HTTP_POST) {
        curl_easy_setopt(client->curl, CURLOPT_POSTFIELDS, req->body);
    }

    CURLcode rc = curl_easy_perform(client->curl);
    if (rc != CURLE_OK) {
        logger_error("HTTP Request failed: %s", curl_easy_strerror(rc));
    }
    return (int)rc;
}
