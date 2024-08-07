#include "strings.h"
#include "memory.h"
#include "typedefs.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>

#define DEFAULT_STRBUF_SIZE 128

cord_str_t cstr(string_ref string) {
    size_t length = strlen(string);
    return (cord_str_t){(char *)string, length};
}

bool cord_str_valid(cord_str_t string) {
    return string.length != -1 && string.data;
}

bool cord_str_equals(cord_str_t first, cord_str_t second) {
    if (first.length != second.length) {
        return false;
    }

    for (ssize_t i = 0; i < first.length; i++) {
        if (first.data[i] != second.data[i]) {
            return false;
        }
    }
    return true;
}

bool cord_str_equals_cstring(cord_str_t first, const char *second) {
    return (second) ? cord_str_equals(first, cstr(second)) : false;
}

bool cord_str_equals_ignore_case(cord_str_t first, cord_str_t second) {
    if (first.length != second.length) {
        return false;
    }

    for (ssize_t i = 0; i < first.length; i++) {
        if (tolower((int)first.data[i]) != tolower((int)second.data[i])) {
            return false;
        }
    }
    return true;
}

bool cord_str_contains(cord_str_t haystack, cord_str_t needle) {
    if (haystack.length < needle.length) {
        return false;
    }

    ssize_t needle_index = -1;
    ssize_t matched_chars = 0;
    for (ssize_t i = 0; i < haystack.length - needle.length; i++) {
        if (matched_chars == needle.length) {
            return true;
        }

        if ((haystack.data[i] == needle.data[0]) && needle_index == -1) {
            needle_index = 0;
            matched_chars++;
            continue;
        }

        if (haystack.data[i] == needle.data[needle_index + 1]) {
            matched_chars++;
            needle_index++;
        } else {
            matched_chars = 0;
            needle_index = -1;
        }
    }
    return false;
}

cord_str_t cord_str_substring(cord_str_t string, size_t start, size_t end) {
    return (cord_str_t){string.data + start, end - start};
}

cord_str_t cord_str_trim(cord_str_t string) {
    string_ref trimmable[] = {"\n", "\t", "\r", " "};
    cord_str_t clean = string;
    for (u32 i = 0; i < 4; i++) {
        clean = cord_str_remove_prefix(clean, cstr(trimmable[i]));
        clean = cord_str_remove_suffix(clean, cstr(trimmable[i]));
    }
    return clean;
}

cord_str_t cord_str_remove_prefix(cord_str_t string, cord_str_t prefix) {
    if ((string.data[0] != prefix.data[0]) ||
        (string.length <= prefix.length)) {
        return string;
    }
    ssize_t index = 0;
    while (string.data[index] == prefix.data[index]) {
        index++;
    }

    ssize_t offset = (index == prefix.length) ? index : 0;
    cord_str_t clean =
        (cord_str_t){string.data + offset, string.length - offset};
    return cord_str_remove_prefix(clean, prefix);
}

cord_str_t cord_str_remove_suffix(cord_str_t string, cord_str_t suffix) {
    if ((string.data[string.length - 1] != suffix.data[suffix.length - 1]) ||
        (string.length <= suffix.length)) {
        return string;
    }

    ssize_t matches = 0;
    for (ssize_t i = 0; i < suffix.length; i++) {
        if (string.data[string.length - i - 1] ==
            suffix.data[suffix.length - i - 1]) {
            matches++;
        }
    }
    ssize_t offset = (matches == suffix.length) ? matches : 0;
    cord_str_t clean = (cord_str_t){string.data, string.length - offset};
    return cord_str_remove_suffix(clean, suffix);
}

cord_str_t cord_str_pop_first_split(cord_str_t *string, cord_str_t split_by) {
    size_t matched_chars = 0;
    size_t index = 0;

    if (split_by.length == 0) {
        return *string;
    }

    for (ssize_t i = 0; i < string->length - split_by.length - 1; i++) {
        if ((ssize_t)matched_chars == split_by.length) {
            break;
        }

        if (string->data[i] == split_by.data[matched_chars + 1]) {
            matched_chars++;
        }

        if ((string->data[i] == split_by.data[0]) && matched_chars == 0) {
            matched_chars++;
            index = i;
        }
    }

    if ((ssize_t)matched_chars < split_by.length) {
        return *string;
    }

    cord_str_t first_split = {string->data, index + matched_chars - 1};

    // Modify the passed string to start after the delimiter
    string->data = string->data + index + 1;
    string->length -= index + 1;

    return first_split;
}

char cord_str_first_char(cord_str_t string) {
    return string.data[0];
}

char cord_str_last_char(cord_str_t string) {
    return string.data[string.length - 1];
}

static void copy_str_to_buffer(void *buffer, cord_str_t string) {
    memcpy(buffer, string.data, string.length);
}

#define STRBUF_GROWTH_FACTOR 2

static size_t strbuf_memory_left(cord_strbuf_t *builder) {
    return builder->capacity - builder->length;
}

cord_strbuf_t *cord_strbuf_create_with_size(size_t size) {
    cord_strbuf_t *builder = malloc(sizeof(cord_strbuf_t));
    if (!builder) {
        return NULL;
    }

    builder->data = calloc(1, size);
    if (!builder->data) {
        free(builder);
        return NULL;
    }
    builder->length = 0;
    builder->capacity = size;
    return builder;
}

cord_strbuf_t *cord_strbuf_create(void) {
    return cord_strbuf_create_with_size(DEFAULT_STRBUF_SIZE);
}

void cord_strbuf_destroy(cord_strbuf_t *builder) {
    if (builder) {
        if (builder->data) {
            free(builder->data);
            builder->data = NULL;
        }
        free(builder);
        builder = NULL;
    }
}

bool cord_strbuf_valid(cord_strbuf_t *builder) {
    return builder && builder->capacity > 0;
}

bool cord_strbuf_empty(cord_strbuf_t *builder) {
    return cord_strbuf_valid(builder) && builder->length == 0;
}

cord_strbuf_t *cord_strbuf_from_cstring(const char *cstring) {
    cord_strbuf_t *builder = cord_strbuf_create();
    cord_strbuf_append(builder, cstr(cstring));
    return builder;
}

void cord_strbuf_append(cord_strbuf_t *builder, cord_str_t string) {
    if ((size_t)string.length > strbuf_memory_left(builder)) {
        size_t new_size = builder->capacity * STRBUF_GROWTH_FACTOR;
        char *new_memory = realloc(builder->data, new_size);
        if (!new_memory) {
            return;
        }
        builder->data = new_memory;
        builder->capacity = new_size;
    }

    char *free_space = builder->data + builder->length;
    copy_str_to_buffer(free_space, string);
    builder->length += string.length;
}

cord_str_t cord_strbuf_to_str_idx(cord_strbuf_t builder, size_t startIdx) {
    return (cord_str_t){builder.data + startIdx, builder.length};
}

cord_str_t cord_strbuf_to_str(cord_strbuf_t builder) {
    return cord_strbuf_to_str_idx(builder, (size_t)0);
}

char *cord_strbuf_to_cstring(cord_strbuf_t builder) {
    char *cstring = calloc(1, builder.length + 1);
    return memcpy(cstring, builder.data, builder.length);
}

char *cord_strbuf_build(cord_strbuf_t builder) {
    char *cstring = calloc(1, builder.length + 1);
    return memcpy(cstring, builder.data, builder.length);
}

bool cstring_is_empty(const char *string) {
    return (strlen(string) == 0) ? true : false;
}

bool cstring_is_equal(const char *s1, const char *s2) {
    return (strcmp(s1, s2) == 0);
}

bool cstring_is_null_or_empty(const char *string) {
    if (!string) {
        return true;
    }
    if (cstring_is_empty(string)) {
        return true;
    }
    return false;
}

const char *bool_to_cstring(bool value) {
    static const char *true_string = "true";
    static const char *false_string = "false";
    return value ? true_string : false_string;
}

char *not_null_cstring(char *string) {
    static char *null_string = "";
    return string ? string : null_string;
}

const char *not_null_cstring_dash(const char *string) {
    static const char *dash = "-";
    return string ? string : dash;
}

char *cstring_of(cord_strbuf_t *builder, cord_bump_t *allocator) {
    const size_t size = builder->length + 1;
    char *cstring = balloc(allocator, size);
    return memcpy(cstring, builder->data, size);
}
