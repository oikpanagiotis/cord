#include "util.h"

#include <string.h>

bool string_is_empty(const char *s) {
	return (strlen(s) == 0) ? true : false;
}

bool string_is_equal(const char *s1, const char *s2) {
	return (strcmp(s1, s2) == 0);
}
