#ifndef JSON_H
#define JSON_H

#include "../core/array.h"
#include "../core/memory.h"
#include "../core/strings.h"
#include "entities.h"

#include <jansson.h>

/*
   json_payload_t *payload_from_cord_message(writer *w, cord_message_t *m);
*/

typedef enum cord_json_writer_field_type_t {
    JSON_WRITER_FIELD_STRING,
    JSON_WRITER_FIELD_BOOL,
    JSON_WRITER_FIELD_NUMBER,
    JSON_WRITER_FIELD_OBJECT,
    JSON_WRITER_FIELD_ARRAY,
    JSON_WRITER_FIELD_NULL
} cord_json_writer_field_type_t;

typedef struct cord_json_writer_t {
    cord_strbuf_t *buffer;
    cord_bump_t *allocator;
} cord_json_writer_t;

cord_json_writer_t cord_json_writer_create(cord_bump_t *allocator);

void cord_json_writer_start(cord_json_writer_t writer);
void cord_json_writer_end(cord_json_writer_t writer);
bool cord_json_writer_write_string(cord_json_writer_t writer, cord_str_t key,
                                   cord_str_t value);
bool cord_json_writer_write_number(cord_json_writer_t writer, cord_str_t key, i64 value);
bool cord_json_writer_write_boolean(cord_json_writer_t writer, cord_str_t key,
                                    bool value);
bool cord_json_writer_write_object(cord_json_writer_t writer, cord_str_t key,
                                   json_t *object);
bool cord_json_writer_write_null(cord_json_writer_t writer, cord_str_t key);
bool cord_json_writer_write_array(cord_json_writer_t writer, cord_str_t key,
                                  cord_array_t *array);

char *cord_message_get_json(cord_json_writer_t writer, cord_message_t *message);

#endif