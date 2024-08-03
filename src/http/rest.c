#include "rest.h"
#include "../core/log.h"
#include "../core/memory.h"
#include "../discord/client.h"
#include "http.h"

cord_http_result_t cord_http_get_current_user(cord_http_client_t *client,
                                              cord_bump_t *allocator) {
    cord_url_builder_t url_builder = cord_url_builder_create(allocator);
    cord_url_builder_add_route(url_builder, cstr(DISCORD_API_URL));
    cord_url_builder_add_route(url_builder, cstr("users/@me"));
    char *url = cord_url_builder_build(url_builder);

    cord_http_result_t result = cord_http_get(client, allocator, url);
    free(url);
    return result;
}

cord_http_result_t cord_http_get_user(cord_http_client_t *http,
                                      cord_bump_t *allocator,
                                      const char *user_id) {
    cord_strbuf_t *url = cord_strbuf_from_cstring("/users/");
    cord_strbuf_append(url, cstr(user_id));

    cord_http_result_t result =
        cord_http_get(http, allocator, cord_strbuf_to_cstring(*url));

    return result;
}
