#ifndef ERRORS_H
#define ERRORS_H

#include <stdbool.h>

#include "typedefs.h"

// Library's API return values
typedef enum cord_error_t {
    // Success
    CORD_OK = 0,

    CORD_ERR_MALLOC,
    CORD_ERR_HTTP_REQUEST,

    // Object serialization errors
    CORD_ERR_USER_SERIALIZATION,
    CORD_ERR_GUILD_MEMBER_SERIALIZATION,
    CORD_ERR_MSG_SERIALIZATION,

    CORD_ERR_EMOJI_SERIALIZATION,

    ERR_COUNT
} cord_error_t;

char *cord_error(int type);

bool is_posix_error(i32 code);

#endif