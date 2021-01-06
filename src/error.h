#ifndef ERROR_H
#define ERROR_H

// Library's API return values
typedef enum {
    // Success
    CORD_OK = 0,

    // Malloc failed
    CORD_ERR_MALLOC,

    // Object serialization errors
    ERR_USER_SERIALIZATION,
    ERR_GUILD_MEMBER_SERIALIZATION,
    ERR_MSG_SERIALIZATION,

    ERR_COUNT
};

char *cord_error(int type);

#endif