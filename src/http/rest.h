#ifndef REST_H
#define REST_H

#include "http.h"

char *cord_http_get_current_user(cord_http_client_t *client);
char *cord_http_get_user(cord_http_client_t *client, const char *user_id);


#endif
