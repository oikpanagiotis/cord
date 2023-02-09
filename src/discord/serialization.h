#ifndef JSON_H
#define JSON_H

#include "../core/array.h"
#include "../core/memory.h"
#include "../core/strings.h"

#include <jansson.h>

// TODO!!! Use this to build a json payload builder with an api that is
// imperative example
/*
    Domain specific functions use the json_payload_writer_t struct and form
   their own high level json_payload_t
   *payload_from_cord_message(json_payload_writer_t *writer, cord_message_t
   *message); json_payload_t *payload_from_cord_user(cord_message_t *message);
*/

typedef enum json_payload_field_type_t {
    JSON_PAYLOAD_FIELD_STRING,
    JSON_PAYLOAD_FIELD_BOOL,
    JSON_PAYLOAD_FIELD_NUMBER,
    JSON_PAYLOAD_FIELD_OBJECT,
    JSON_PAYLOAD_FIELD_ARRAY,
    JSON_PAYLOAD_FIELD_NULL
} json_payload_field_type_t;

typedef struct json_payload_t {
    cord_strbuf_t buffer;
    cord_bump_t *allocator;
} json_payload_t;

json_payload_t json_payload_create(cord_bump_t *allocator);

void json_payload_start_writing(json_payload_t payload);
void json_payload_finish_writing(json_payload_t payload);
bool json_payload_write_string(json_payload_t payload, cord_str_t key, cord_str_t value);
bool json_payload_write_number(json_payload_t payload, cord_str_t key, i64 value);
bool json_payload_write_boolean(json_payload_t payload, cord_str_t key, bool value);
bool json_payload_write_object(json_payload_t payload, cord_str_t key, json_t *object);
bool json_payload_write_null(json_payload_t payload, cord_str_t key);
bool json_payload_write_array(json_payload_t payload, cord_str_t key,
                              cord_array_t *array);

// void json_payload_apply_transformer(json_payload_t payload, transformer_t
// *transformer);

#endif