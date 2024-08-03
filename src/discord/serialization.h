#ifndef JSON_H
#define JSON_H

#include "../core/array.h"
#include "../core/errors.h"
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
bool cord_json_writer_write_number(cord_json_writer_t writer, cord_str_t key,
                                   i64 value);
bool cord_json_writer_write_boolean(cord_json_writer_t writer, cord_str_t key,
                                    bool value);
bool cord_json_writer_write_object(cord_json_writer_t writer, cord_str_t key,
                                   json_t *object);
bool cord_json_writer_write_null(cord_json_writer_t writer, cord_str_t key);
bool cord_json_writer_write_array(cord_json_writer_t writer, cord_str_t key,
                                  cord_array_t *array);

char *cord_message_to_json(cord_json_writer_t writer, cord_message_t *message);

#define map_property_object_collectible(object, property, property_str, key,   \
                                        value, allocator, type, serialize)     \
    if (cstring_is_equal(key, property_str)) {                                 \
        cord_serialize_result_t __result = serialize(value, allocator, NULL);  \
        if (has_serialization_error(__result)) {                               \
            logger_error("Failed to serialize " #type ": %s",                  \
                         cord_error(__result.error));                          \
            continue;                                                          \
        }                                                                      \
        object->property = __result.obj;                                       \
    }

#define map_property_object(object, property, property_str, key, value,        \
                            allocator, type, serialize)                        \
    if (cstring_is_equal(key, property_str)) {                                 \
        cord_serialize_result_t __result = serialize(value, allocator);        \
        if (has_serialization_error(__result)) {                               \
            logger_error("Failed to serialize " #type ": %s",                  \
                         cord_error(__result.error));                          \
        }                                                                      \
        object->property = __result.obj;                                       \
    }

typedef struct cord_serialize_result_t {
    void *obj;
    cord_error_t error;
} cord_serialize_result_t;

bool cord_serialize_result_is_valid(cord_serialize_result_t result);
cord_serialize_result_t cord_user_serialize(char *data, cord_bump_t *allocator);
cord_serialize_result_t cord_role_serialize(json_t *data,
                                            cord_bump_t *allocator,
                                            cord_role_t *array_slot);

cord_serialize_result_t cord_role_tag_serialize(json_t *json_role_tag,
                                                cord_bump_t *allocator,
                                                cord_role_tag_t *array_slot);

cord_serialize_result_t cord_guild_member_serialize(json_t *data,
                                                    cord_bump_t *allocator);
cord_serialize_result_t cord_channel_mention_serialize(json_t *data,
                                                       cord_bump_t *allocator);

cord_serialize_result_t cord_attachment_serialize(json_t *data,
                                                  cord_bump_t *allocator);
cord_serialize_result_t
cord_embed_footer_serialize(json_t *data, cord_bump_t *allocator,
                            cord_embed_footer_t *array_slot);

cord_serialize_result_t
cord_embed_image_serialize(json_t *data, cord_bump_t *allocator,
                           cord_embed_image_t *array_slot);

cord_serialize_result_t
cord_embed_thumbnail_serialize(json_t *data, cord_bump_t *allocator,
                               cord_embed_thumbnail_t *array_slot);

cord_serialize_result_t
cord_embed_video_serialize(json_t *data, cord_bump_t *allocator,
                           cord_embed_video_t *array_slot);

cord_serialize_result_t
cord_embed_provider_serialize(json_t *data, cord_bump_t *allocator,
                              cord_embed_provider_t *array_slot);

cord_serialize_result_t
cord_embed_author_serialize(json_t *data, cord_bump_t *allocator,
                            cord_embed_author_t *array_slot);

cord_serialize_result_t
cord_embed_field_serialize(json_t *data, cord_bump_t *allocator,
                           cord_embed_field_t *array_slot);

cord_serialize_result_t cord_embed_serialize(json_t *data,
                                             cord_bump_t *allocator);
cord_serialize_result_t cord_emoji_serialize(json_t *data,
                                             cord_bump_t *allocator);
cord_serialize_result_t cord_reaction_serialize(json_t *data,
                                                cord_bump_t *allocator);
cord_serialize_result_t cord_message_activity_serialize(json_t *data,
                                                        cord_bump_t *allocator);

cord_serialize_result_t
cord_message_application_serialize(json_t *data, cord_bump_t *allocator);

cord_serialize_result_t cord_message_sticker_serialize(json_t *data,
                                                       cord_bump_t *allocator);

cord_serialize_result_t cord_message_serialize(json_t *data,
                                               cord_bump_t *allocator);
cord_serialize_result_t cord_guild_serialize(json_t *data,
                                             cord_bump_t *allocator);

#endif
