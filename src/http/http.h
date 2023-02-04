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

typedef struct http_request_t {
	int type;
	struct curl_slist *header;
	char *body;
	char *url;
} http_request_t;

http_request_t *http_request_create(int type, char *url, struct curl_slist *header, char *content);
cord_http_client_t *http_client_create(const char *bot_token);
void http_client_destroy(cord_http_client_t *client);
struct curl_slist *discord_api_header(cord_http_client_t *client);
int http_client_perform_request(cord_http_client_t *client, http_request_t *req);

#endif
