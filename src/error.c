#include "error.h"

typedef struct api_error {
    int type;
    char *message;
} api_error;

static api_error errors[] = {
    { CORD_OK, "" },

    { CORD_ERR_MALLOC, "Memory allocation failed" },
    
    { ERR_USER_SERIALIZATION, "Failed to serialize user object" },
    { ERR_GUILD_MEMBER_SERIALIZATION, "Failed to serialize guild member object" },
    { ERR_MSG_SERIALIZATION, "Failed to serialize message object" },


};

char *cord_error(int type) {
    if (type < 0 || type > ERR_COUNT) {
        return "";
    }

    return errors[type].message;
}
