#ifndef REST_H
#define REST_H

#include "../discord/entities.h"
#include "http.h"

cord_user_t *cord_api_get_current_user(cord_http_client_t *client,
                                       cord_bump_t *allocator);

cord_http_result_t cord_http_get_user(cord_http_client_t *client,
                                      cord_bump_t *allocator,
                                      const char *user_id);

cord_http_result_t cord_http_authenticate(cord_http_client_t *client,
                                          cord_bump_t *allocator);

#endif
