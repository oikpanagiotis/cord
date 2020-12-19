#ifndef ERROR_H
#define ERROR_H

// Library's API return values
typedef enum {
    // Success
    ERR_NONE = 0,

    // Malloc failed
    ERR_MALLOC,

    // Object serialization errors
    ERR_USER_SERIALIZATION,
    ERR_GUILD_MEMBER_SERIALIZATION,
    ERR_MSG_SERIALIZATION,

    ERR_COUNT
};

char *cord_error(int type);

#endif