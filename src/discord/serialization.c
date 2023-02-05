#include "serialization.h"
#include "../core/log.h"
#include "../core/util.h"
#include "../strings.h"

#include <jansson.h>
#include <stdio.h>

static void append_quote(json_payload_t payload) {
    cord_strbuf_append(&payload.buffer, cstr("\""));
}

static void end_append_key(json_payload_t payload) {
    cord_strbuf_append(&payload.buffer, cstr(": "));
}

static void append_key(json_payload_t payload, cord_str_t key) {
    append_quote(payload);
    cord_strbuf_append(&payload.buffer, key);
    append_quote(payload);
    end_append_key(payload);
}

static void append_comma(json_payload_t payload) {
    cord_strbuf_append(&payload.buffer, cstr(","));
}

static void append_final_value(json_payload_t payload, char *cstring) {
    cord_strbuf_append(&payload.buffer, cstr(cstring));
}

static void append_non_final_value(json_payload_t payload, char *cstring) {
    append_final_value(payload, cstring);
    append_comma(payload);
}

json_payload_t json_payload_create(cord_bump_t *allocator) {
    return (json_payload_t){.buffer =
                                cord_strbuf_create_with_allocator(allocator),
                            .allocator = allocator};
}

void json_payload_start_writing(json_payload_t payload) {
    cord_strbuf_append(&payload.buffer, cstr("{"));
}

void json_payload_finish_writing(json_payload_t payload) {
    cord_strbuf_append(&payload.buffer, cstr("}"));
}

bool json_payload_write_string(json_payload_t payload, cord_str_t key,
                               cord_str_t value) {
    append_key(payload, key);
    append_quote(payload);
    cord_strbuf_append(&payload.buffer, value);
    append_quote(payload);
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

static bool copy_bool_to_string(char *format, bool value) {
    i32 rc =
        snprintf(format, MAX_FORMAT_BUFFER_LENGTH, "%s", bool_to_string(value));
    if (is_posix_error(rc)) {
        logger_warn("Could not convert bool (%s) to string",
                    bool_to_string(value));
    }
    return is_posix_error(rc);
}

bool json_payload_write_number(json_payload_t payload, cord_str_t key,
                               i64 value) {
    append_key(payload, key);
    char format[MAX_FORMAT_BUFFER_LENGTH] = {0};
    bool success = copy_i64_to_string(format, value);
    if (!success) {
        logger_warn("Failed to convert i64: %ld to string", value);
        return false;
    }
    append_non_final_value(payload, format);
    return true;
}

bool json_payload_write_boolean(json_payload_t payload, cord_str_t key,
                                bool value) {
    append_key(payload, key);
    char format[MAX_FORMAT_BUFFER_LENGTH] = {0};
    bool success = copy_bool_to_string(format, value);
    if (!success) {
        logger_warn("Failed to convert bool: %s to string",
                    bool_to_string(value));
        return false;
    }
    append_non_final_value(payload, format);
    return true;
}

bool json_payload_write_null(json_payload_t payload, cord_str_t key) {
    append_key(payload, key);
    append_non_final_value(payload, "null");
    return true;
}

bool json_payload_write_object(json_payload_t payload, cord_str_t key,
                               json_t *object) {
    append_key(payload, key);
    // foreach field write using write functions
    // for each object write using itself
}

bool json_payload_write_array(json_payload_t payload, cord_str_t key,
                              cord_array_t *array) {
    return false;
}