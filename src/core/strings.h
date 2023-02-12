#ifndef STRINGS_H
#define STRINGS_H

#include "memory.h"
#include "typedefs.h"

#include <stdbool.h>
#include <sys/types.h>

/*
 * String view structure
 *
 * This is is an allocation-free string view structure that performs
 * every operation by changing the length field and where the data pointer
 * points. It will never attempty to overwrite the underlying memory.
 *
 */
typedef struct cord_str_t {
    char *data;
    ssize_t length;
} cord_str_t;

/*
 * Utility function to create a string view from a C string literal
 */
cord_str_t cstr(string_ref string);

bool cord_str_valid(cord_str_t string);
bool cord_str_equals(cord_str_t first, cord_str_t second);
bool cord_str_equals_cstring(cord_str_t first, const char *second);
bool cord_str_equals_ignore_case(cord_str_t first, cord_str_t second);
bool cord_str_contains(cord_str_t haystack, cord_str_t needle);

/*
 * Returns substring of cord_str_t as defined by the indices (begin inclusive,
 * end non-inclusive)
 */
cord_str_t cord_str_substring(cord_str_t string, size_t begin, size_t end);

/*
 * Returns the string trimmed of any whitespace characters at the start, end
 */
cord_str_t cord_str_trim(cord_str_t string);
cord_str_t cord_str_remove_prefix(cord_str_t string, cord_str_t prefix);
cord_str_t cord_str_remove_suffix(cord_str_t string, cord_str_t suffix);

/*
 * Splits the passed cord_str_t and returns the first split found
 */
cord_str_t cord_str_pop_first_split(cord_str_t *string, cord_str_t split_by);
char cord_str_first_char(cord_str_t string);
char cord_str_last_char(cord_str_t string);

/*
 * String builder structure
 *
 * This is a string builder backed by a bump allocator that owns and
 * manages the memory that is uses. It is suitable for creating large
 * strings of size that is not known in compile-time. When the string builder
 * is no longer needed, it must be free'd using cord_strbuf_destroy().
 *
 */
typedef struct cord_strbuf_t {
    char *data;
    size_t length;
    size_t capacity;
    cord_bump_t *allocator;
} cord_strbuf_t;

#define cord_strbuf_undefined (cord_strbuf_t){NULL, 0, 0, NULL};

cord_strbuf_t *cord_strbuf_create(void);
void cord_strbuf_destroy(cord_strbuf_t *builder);
bool cord_strbuf_valid(cord_strbuf_t *builder);
bool cord_strbuf_empty(cord_strbuf_t *builder);
cord_strbuf_t *cord_strbuf_from_cstring(const char *cstring);

/*
 * Used to append strings to string builder. C style strings can be converted
 * to cord_str_t with cstr() function.
 */
void cord_strbuf_append(cord_strbuf_t *builder, cord_str_t string);
cord_strbuf_t *cord_strbuf_create_with_allocator(cord_bump_t *allocator);
cord_str_t cord_strbuf_to_str(cord_strbuf_t builder);

/*
 * Allocates and returns a null terminated string.
 * The caller is responsible for freeing the returned string.
 */
char *cord_strbuf_to_cstring(cord_strbuf_t builder);

bool cstring_is_empty(const char *string);
bool cstring_is_equal(const char *s1, const char *s2);
bool cstring_is_null_or_empty(const char *string);
const char *bool_to_cstring(bool value);
char *not_null_cstring(char *string);
char *not_null_cstring_dash(char *string);

char *cstring_of(cord_strbuf_t *builder, cord_bump_t *allocator);

#endif