#include "serialization.h"
#include "../core/errors.h"
#include "../core/log.h"
#include "../strings.h"
#include "entities.h"

#include <assert.h>
#include <jansson.h>
#include <stdio.h>

static void append_quote(cord_json_writer_t writer) {
    cord_strbuf_append(writer.buffer, cstr("\""));
}

static void end_append_key(cord_json_writer_t writer) {
    cord_strbuf_append(writer.buffer, cstr(": "));
}

static void append_key(cord_json_writer_t writer, cord_str_t key) {
    append_quote(writer);
    cord_strbuf_append(writer.buffer, key);
    append_quote(writer);
    end_append_key(writer);
}

static void append_comma(cord_json_writer_t writer) {
    cord_strbuf_append(writer.buffer, cstr(","));
}

static void append_final_value(cord_json_writer_t writer, char *cstring) {
    cord_strbuf_append(writer.buffer, cstr(cstring));
}

static void append_non_final_value(cord_json_writer_t writer, char *cstring) {
    append_final_value(writer, cstring);
    append_comma(writer);
}

#define MAX_FORMAT_BUFFER_LENGTH 64

static bool copy_i64_to_string(char *format, i64 value) {
    if (value > MAX_FORMAT_BUFFER_LENGTH) {
        return false;
    }

    i32 rc = snprintf(format, MAX_FORMAT_BUFFER_LENGTH, "%ld", value);
    if (is_posix_error(rc)) {
        return false;
    }
    return true;
}

cord_json_writer_t cord_json_writer_create(cord_bump_t *allocator) {
    return (cord_json_writer_t){.buffer = cord_strbuf_create_with_allocator(allocator),
                                .allocator = allocator};
}

void cord_json_writer_start(cord_json_writer_t writer) {
    cord_strbuf_append(writer.buffer, cstr("{"));
}

void cord_json_writer_end(cord_json_writer_t writer) {
    cord_strbuf_append(writer.buffer, cstr("}"));
}

bool cord_json_writer_write_string(cord_json_writer_t writer, cord_str_t key,
                              cord_str_t value) {
    append_key(writer, key);
    append_quote(writer);
    cord_strbuf_append(writer.buffer, value);
    append_quote(writer);
    return true;
}

static bool copy_bool_to_string(char *format, bool value) {
    i32 rc = snprintf(format, MAX_FORMAT_BUFFER_LENGTH, "%s", bool_to_cstring(value));
    if (is_posix_error(rc)) {
        logger_warn("Could not convert bool (%s) to string", bool_to_cstring(value));
    }
    return is_posix_error(rc);
}

bool cord_json_writer_write_number(cord_json_writer_t writer, cord_str_t key, i64 value) {
    append_key(writer, key);
    char format[MAX_FORMAT_BUFFER_LENGTH] = {0};
    bool success = copy_i64_to_string(format, value);
    if (!success) {
        logger_warn("Failed to convert i64: %ld to string", value);
        return false;
    }
    append_non_final_value(writer, format);
    return true;
}

bool cord_json_writer_write_boolean(cord_json_writer_t writer, cord_str_t key,
                                    bool value) {
    append_key(writer, key);
    char format[MAX_FORMAT_BUFFER_LENGTH] = {0};
    bool success = copy_bool_to_string(format, value);
    if (!success) {
        logger_warn("Failed to convert bool: %s to string", bool_to_cstring(value));
        return false;
    }
    append_non_final_value(writer, format);
    return true;
}

bool cord_json_writer_write_null(cord_json_writer_t writer, cord_str_t key) {
    append_key(writer, key);
    append_non_final_value(writer, "null");
    return true;
}

bool cord_json_writer_write_object(cord_json_writer_t writer, cord_str_t key,
                                   json_t *object) {
    append_key(writer, key);
    // foreach field write using write functions
    // for each object write using itself
    assert(0 && "Not Implemented");
    return false;
}

bool cord_json_writer_write_array(cord_json_writer_t writer, cord_str_t key,
                                  cord_array_t *array) {
    assert(0 && "Not Implemented");
    return false;
}

#define serialize_string(writer, string, obj, field)                                     \
    if (obj->field) {                                                                    \
        cord_json_writer_write_string(writer, cstr(string),                              \
                                      cord_strbuf_to_str(*((obj)->field)));              \
    }

char *cord_message_get_json(cord_json_writer_t writer, cord_message_t *message) {
    cord_json_writer_start(writer);
    {
        serialize_string(writer, "content", message, content);
    }
    cord_json_writer_end(writer);
    return cstring_of(writer.buffer, writer.allocator);
}