#include "errors.h"
#include "memory.h"
#include <assert.h>
#include <stdlib.h>

typedef struct lib_error {
    int type;
    char *message;
} lib_error;

static lib_error errors[] = {
    {CORD_OK, ""},

    {CORD_ERR_MALLOC, "Memory allocation failed"},
    {CORD_ERR_HTTP_REQUEST, "Failed to perform HTTP request"},

    {CORD_ERR_OBJ_SERIALIZE, "Failed to serialize object"},

};

char *cord_error(int type) {
    static_assert(array_length(errors) == ERR_COUNT, "cord_error_t mismatch");

    if (type < 0 || type >= ERR_COUNT) {
        return "";
    }

    return errors[type].message;
}

bool is_posix_error(i32 code) {
    return code < (i32)0;
}
