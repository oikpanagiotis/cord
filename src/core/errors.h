#ifndef ERRORS_H
#define ERRORS_H

#include <stdbool.h>

#include "typedefs.h"

// Library's API return values
typedef enum cord_error_t {
    // Success
    CORD_OK,

    CORD_ERR_MALLOC,
    CORD_ERR_HTTP_REQUEST,

    CORD_ERR_OBJ_SERIALIZE,

    ERR_COUNT
} cord_error_t;

char *cord_error(int type);

bool is_posix_error(i32 code);

#endif
