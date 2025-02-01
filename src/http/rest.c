#include "rest.h"
#include "../core/log.h"
#include "../core/memory.h"
#include "../discord/client.h"
#include "../discord/entities.h"
#include "../discord/serialization.h"
#include "http.h"
#include <curl/curl.h>

static json_t *parse_and_free_json(char *json) {
    json_error_t error = {};
    json_t *json_obj = json_loads(json, 0, &error);
    if (!json_obj) {
        logger_error(
            "Failed to parse json. (line:%d): %s", error.line, error.text);
    }
    free(json);
    return json_obj;
}

cord_user_t *cord_api_get_current_user(cord_http_client_t *client,
                                       cord_bump_t *allocator) {
    cord_url_builder_t url_builder = cord_url_builder_create(allocator);
    cord_url_builder_add_route(url_builder, cstr(DISCORD_API_URL));
    cord_url_builder_add_route(url_builder, cstr("users/@me"));
    cord_str_t url = cord_url_builder_build(url_builder);

    cord_http_result_t result = cord_http_get(client, allocator, url);
    if (result.error) {
        logger_error("Failed to get current user");
        return NULL;
    }

    json_t *json = parse_and_free_json(result.body);
    if (!json) {
        return NULL;
    }

    cord_bump_t *serialize_bump = cord_bump_create();
    cord_serialize_result_t user = cord_user_serialize(json, serialize_bump);
    if (user.error) {
        logger_error("Failed to serialize user: %s", cord_error(user.error));
        cord_bump_destroy(serialize_bump);
        return NULL;
    }

    return user.obj;
}

cord_http_result_t cord_http_get_user(cord_http_client_t *http,
                                      cord_bump_t *allocator,
                                      const char *user_id) {
    cord_strbuf_t *url = cord_strbuf_from_cstring("/users/");
    cord_strbuf_append(url, cstr(user_id));
    cord_str_t str_url = cord_strbuf_to_str(*url);

    cord_http_result_t result = cord_http_get(http, allocator, str_url);

    return result;
}
