#include "errors.h"

typedef struct api_error {
    int type;
    char *message;
} api_error;

static api_error errors[] = {
    {CORD_OK, ""},

    {CORD_ERR_MALLOC, "Memory allocation failed"},

    {CORD_ERR_USER_SERIALIZATION, "Failed to serialize user object"},
    {CORD_ERR_GUILD_MEMBER_SERIALIZATION, "Failed to serialize guild member object"},
    {CORD_ERR_MSG_SERIALIZATION, "Failed to serialize message object"},

    {CORD_ERR_EMOJI_SERIALIZATION, "Failed to serialize emoji object"},

};

char *cord_error(int type) {
    if (type < 0 || type > ERR_COUNT) {
        return "";
    }

    return errors[type].message;
}

bool is_posix_error(i32 code) {
    return code < (i32)0;
}