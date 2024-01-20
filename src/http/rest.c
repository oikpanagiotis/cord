#include "rest.h"
#include "http.h"

char *cord_http_get_current_user(cord_http_client_t *client) {
    cord_error_t err = cord_http_get(client, "/users/@me");
}

char *cord_http_get_user(cord_http_client_t *http, const char *user_id) {
    cord_strbuf_t *url = cord_strbuf_from_cstring("/users/");
    cord_strbuf_append(url, cstr(user_id));

    cord_error_t err = cord_http_get(http, url);
}

