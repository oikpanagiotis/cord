#include "util.h"
#include "typedefs.h"

#include <string.h>


bool string_is_empty(const char *s) {
	return (strlen(s) == 0) ? true : false;
}

bool string_is_equal(const char *s1, const char *s2) {
	return (strcmp(s1, s2) == 0);
}

bool string_is_null_or_empty(const char *string) {
    if (!string) {
        return false;
    }
    if (string_is_empty(string)) {
        return false;
    }
    return true;
}

bool is_posix_error(i32 code) {
	return code < (i32)0;
}

const char *bool_to_string(bool value) {
    static const char *true_string = "true";
    static const char *false_string = "false";
    return value ? true_string : false_string;
}