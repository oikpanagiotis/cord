#ifndef UTIL_H
#define UTIL_H

#include "typedefs.h"

#include <assert.h>
#include <jansson.h>
#include <stdbool.h>

#define array_length(arr) (sizeof(arr) / sizeof(*arr))

/*
 * Utility macro to set a struct field value using the field name as key
 */
#define map_property(obj, prop, prop_str, key, val)                            \
    do {                                                                       \
        if (string_is_equal(key, prop_str)) {                                  \
            obj->prop = val;                                                   \
        }                                                                      \
    } while (0)

bool string_is_empty(const char *s);
bool string_is_equal(const char *s1, const char *s2);
bool string_is_null_or_empty(const char *string);
bool is_posix_error(i32 code);
const char *bool_to_string(bool value);

#endif