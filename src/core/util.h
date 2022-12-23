#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <jansson.h>

#define map_property(obj, prop, prop_str, key, val) \
	do { \
		if (string_is_equal(key, prop_str)) { \
			obj->prop = val; \
		} \
} while (0)


bool string_is_empty(const char *s);
bool string_is_equal(const char *s1, const char *s2);


#endif