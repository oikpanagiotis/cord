#include "types.h"
#include "../core/array.h"
#include "../core/errors.h"
#include "../core/log.h"
#include "../core/strings.h"
#include "../core/typedefs.h"
#include "error.h"

#include <assert.h>
#include <jansson.h>

/*
 * Utility macro to set a struct field value using the field name as key
 */
#define map_property(obj, prop, prop_str, key, val)                            \
    do {                                                                       \
        if (cstring_is_equal(key, prop_str)) {                                  \
            obj->prop = val;                                                   \
        }                                                                      \
    } while (0)

#define map_property_array(object, property, property_str, key, value,         \
                           allocator, type, serialize)                         \
    do {                                                                       \
        if (cstring_is_equal(key, property_str)) {                              \
            size_t __idx = 0;                                                  \
            json_t *__item = NULL;                                             \
            object->property = cord_array_create(allocator, sizeof(type));     \
            json_array_foreach(value, __idx, __item) {                         \
                type *__array_slot = cord_array_push(object->property);        \
                cord_serialize_result_t __result =                             \
                    serialize(__item, allocator, __array_slot);                \
                if (has_serialization_error(__result)) {                       \
                    logger_error("Failed to serialize " #type ": %s",          \
                                 cord_error(__result.error));                  \
                    break;                                                     \
                }                                                              \
            }                                                                  \
        }                                                                      \
    } while (0)

static bool has_serialization_error(cord_serialize_result_t result) {
    return result.error == CORD_OK;
}

static cord_serialize_result_t serialized(void *obj) {
    return (cord_serialize_result_t){obj, CORD_OK};
}

static cord_serialize_result_t serialize_error(cord_error_t error) {
    return (cord_serialize_result_t){NULL, error};
}

static void user_read_boolean_json_field(cord_user_t *author, const char *key,
                                         bool value) {
    map_property(author, bot, "bot", key, value);
    map_property(author, system_, "system", key, value);
    map_property(author, mfa_enabled, "mfa_enabled", key, value);
    map_property(author, verified, "varified", key, value);
}

static void user_read_string_json_field(cord_user_t *user, string_ref key,
                                        string_ref value) {
    cord_strbuf_t *field_value =
        cord_strbuf_create_with_allocator(user->allocator);
    cord_strbuf_append(field_value, cstr(value));

    map_property(user, id, "id", key, field_value);
    map_property(user, username, "username", key, field_value);
    map_property(user, discriminator, "discriminator", key, field_value);
    map_property(user, avatar, "avatar", key, field_value);
    map_property(user, locale, "locale", key, field_value);
    map_property(user, email, "email", key, field_value);
}

static void user_read_integer_json_field(cord_user_t *user, string_ref key,
                                         i64 value) {
    map_property(user, flags, "flags", key, value);
    map_property(user, premium_type, "premium_type", key, value);
    map_property(user, public_flags, "public_flags", key, value);
}

// Jansson
// docs:https://jansson.readthedocs.io/en/latest/apiref.html#custom-memory-allocation
// Use custom arena just for json
cord_serialize_result_t cord_user_serialize(json_t *user,
                                            cord_bump_t *allocator) {
    cord_user_t *author = balloc(allocator, sizeof(cord_user_t));
    if (!author) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_user_init(author, allocator);
    const char *key = NULL;
    json_t *value = NULL;

    json_object_foreach(user, key, value) {
        if (json_is_string(value)) {
            string_ref cstring = json_string_value(value);
            user_read_string_json_field(author, key, cstring);
        } else if (json_is_boolean(value)) {
            bool builder = json_boolean_value(value);
            user_read_boolean_json_field(author, key, builder);
        } else if (json_is_integer(value)) {
            i64 builder = (i64)json_integer_value(value);
            user_read_integer_json_field(author, key, builder);
        }
    }

    return serialized(author);
}

static void guild_member_booleans(cord_guild_member_t *member, string_ref key,
                                  bool value) {
    map_property(member, deaf, "deaf", key, value);
    map_property(member, mute, "mute", key, value);
    map_property(member, pending, "pending", key, value);
}

static void guild_member_strings(cord_guild_member_t *member, string_ref key,
                                 string_ref cstring) {
    cord_strbuf_t *builder =
        cord_strbuf_create_with_allocator(member->allocator);
    cord_strbuf_append(builder, cstr(cstring));

    map_property(member, nick, "nick", key, builder);
    map_property(member, joined_at, "joined_at", key, builder);
    map_property(member, premium_since, "premium_since", key, builder);
}

static void guild_member_arrays(cord_guild_member_t *member, string_ref key,
                                json_t *value) {
    map_property_array(member, roles, "roles", key, value, member->allocator,
                       cord_role_t, cord_role_serialize);
}

cord_serialize_result_t cord_guild_member_serialize(json_t *json_message,
                                                    cord_bump_t *allocator) {
    cord_guild_member_t *member =
        balloc(allocator, sizeof(cord_guild_member_t));
    if (!member) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_guild_member_init(member, member->allocator);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(json_message, key, value) {
        if (json_is_string(value)) {
            guild_member_strings(member, key, json_string_value(value));
        } else if (json_is_boolean(value)) {
            guild_member_booleans(member, key, json_boolean_value(value));
        } else if (json_is_array(value)) {
            guild_member_arrays(member, key, value);
        } else if (json_is_object(value)) {
            if (cstring_is_equal(key, "user")) {
                cord_serialize_result_t result =
                    cord_user_serialize(value, allocator);
                if (is_posix_error(result.error)) {
                    // Skip field and keep serializing the rest of the object
                    logger_error("Failed to serialize user as part of guild "
                                 "member serialize (%s)",
                                 cord_error(result.error));
                } else {
                    member->user = (cord_user_t *)result.obj;
                }
            }
        }
    }

    return serialized(member);
}

static void role_strings(cord_role_t *role, string_ref key,
                         string_ref cstring) {
    cord_strbuf_t *builder = cord_strbuf_create_with_allocator(role->allocator);
    cord_strbuf_append(builder, cstr(cstring));

    map_property(role, id, "id", key, builder);
    map_property(role, name, "name", key, builder);
    map_property(role, icon, "icon", key, builder);
    map_property(role, unicode_emoji, "unicode_emoji", key, builder);
    map_property(role, permissions, "permissions", key, builder);
}

static void role_booleans(cord_role_t *role, string_ref key, bool value) {
    map_property(role, hoist, "hoist", key, value);
    map_property(role, managed, "managed", key, value);
    map_property(role, mentionable, "mentionable", key, value);
}

static void role_numbers(cord_role_t *role, string_ref key, i64 number) {
    map_property(role, color, "color", key, number);
    map_property(role, position, "position", key, number);
}

cord_serialize_result_t cord_role_serialize(json_t *json_message,
                                            cord_bump_t *allocator,
                                            cord_role_t *array_slot) {
    cord_role_t *role =
        array_slot ? array_slot : balloc(allocator, sizeof(cord_role_t));
    if (!role) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_role_init(role, allocator);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(json_message, key, value) {
        if (json_is_string(value)) {
            string_ref cstring = json_string_value(value);
            role_strings(role, key, cstring);
        } else if (json_is_boolean(value)) {
            bool builder = json_boolean_value(value);
            role_booleans(role, key, builder);
        } else if (json_is_integer(value)) {
            i32 number = (i32)json_integer_value(value);
            role_numbers(role, key, number);
        } else if (json_is_array(value)) {
            size_t index = 0;
            json_t *item = NULL;

            if (cstring_is_equal(key, "tags")) {
                role->tags =
                    cord_array_create(allocator, sizeof(cord_role_tag_t));
                json_array_foreach(value, index, item) {

                    cord_role_tag_t *array_slot = cord_array_push(role->tags);
                    cord_serialize_result_t role_tag =
                        cord_role_tag_serialize(item, allocator, array_slot);

                    if (has_serialization_error(role_tag)) {
                        logger_error("Failed to serialize cord_role_tag_t: %s",
                                     cord_error(role_tag.error));
                        break;
                    }
                }
            }
        }
    }
    return serialized(role);
}

static void role_tag_strings(cord_role_tag_t *role_tag, string_ref key,
                             string_ref cstring) {
    cord_strbuf_t *builder =
        cord_strbuf_create_with_allocator(role_tag->allocator);
    cord_strbuf_append(builder, cstr(cstring));

    map_property(role_tag, bot_id, "bot_id", key, builder);
    map_property(role_tag, integration_id, "integration_id", key, builder);
    map_property(role_tag, subscription_listing_id, "subscription_listing_id",
                 key, builder);
}

static void role_tag_booleans(cord_role_tag_t *role_tag, string_ref key,
                              bool builder) {
    map_property(role_tag, premium_subscriber, "premium_subscriber", key,
                 builder);
    map_property(role_tag, available_for_purchase, "available_for_purchase",
                 key, builder);
    map_property(role_tag, guild_connections, "guild_connections", key,
                 builder);
}

cord_serialize_result_t cord_role_tag_serialize(json_t *json_role_tag,
                                                cord_bump_t *allocator,
                                                cord_role_tag_t *array_slot) {
    cord_role_tag_t *role_tag =
        array_slot ? array_slot : balloc(allocator, sizeof(cord_role_t));

    if (!role_tag) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_role_tag_init(role_tag, allocator);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(json_role_tag, key, value) {
        if (json_is_string(value)) {
            string_ref cstring = json_string_value(value);
            role_tag_strings(role_tag, key, cstring);
        } else if (json_is_boolean(value)) {
            bool builder = json_boolean_value(value);
            role_tag_booleans(role_tag, key, builder);
        }
    }

    return serialized(role_tag);
}

static void channel_mention_strings(cord_channel_mention_t *mention,
                                    string_ref key, string_ref value) {
    cord_strbuf_t *string_buffer =
        cord_strbuf_create_with_allocator(mention->allocator);
    cord_strbuf_append(string_buffer, cstr(value));

    map_property(mention, id, "id", key, string_buffer);
    map_property(mention, guild_id, "guild_id", key, string_buffer);
    map_property(mention, name, "name", key, string_buffer);
}

static void channel_mention_numbers(cord_channel_mention_t *mention,
                                    string_ref key, i64 value) {
    map_property(mention, type, "type", key, value);
}

cord_serialize_result_t cord_channel_mention_serialize(json_t *json_mention,
                                                       cord_bump_t *allocator) {
    cord_channel_mention_t *mention =
        balloc(allocator, sizeof(cord_channel_mention_t));
    if (!mention) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_channel_mention_init(mention, allocator);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(json_mention, key, value) {
        if (json_is_string(value)) {
            string_ref cstring = json_string_value(value);
            channel_mention_strings(mention, key, cstring);
        } else if (json_is_integer(value)) {
            i64 number = json_integer_value(json_mention);
            channel_mention_numbers(mention, key, number);
        }
    }

    return serialized(mention);
}

static void attachment_strings(cord_attachment_t *attachment, string_ref key,
                               string_ref cstring) {
    cord_strbuf_t *builder =
        cord_strbuf_create_with_allocator(attachment->allocator);
    cord_strbuf_append(builder, cstr(cstring));

    map_property(attachment, id, "id", key, builder);
    map_property(attachment, filename, "filename", key, builder);
    map_property(attachment, url, "url", key, builder);
    map_property(attachment, proxy_url, "proxy_url", key, builder);
}

static void attachment_numbers(cord_attachment_t *attachment, string_ref key,
                               i32 number) {
    map_property(attachment, size, "size", key, number);
    map_property(attachment, height, "height", key, number);
    map_property(attachment, width, "width", key, number);
}

cord_serialize_result_t cord_attachment_serialize(json_t *json_attachment,
                                                  cord_bump_t *allocator) {
    cord_attachment_t *attachment =
        balloc(allocator, sizeof(cord_attachment_t));
    if (!attachment) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_attachment_init(attachment, allocator);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(json_attachment, key, value) {
        if (json_is_string(value)) {
            attachment_strings(attachment, key, json_string_value(value));
        } else if (json_is_integer(value)) {
            attachment_numbers(attachment, key, json_integer_value(value));
        }
    }

    return serialized(attachment);
}

cord_serialize_result_t
cord_embed_footer_serialize(json_t *json_embed_footer, cord_bump_t *allocator,
                            cord_embed_footer_t *array_slot) {
    cord_embed_footer_t *embed_footer =
        array_slot ? array_slot
                   : balloc(allocator, sizeof(cord_embed_footer_t));
    if (!embed_footer) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_embed_footer_init(embed_footer, allocator);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(json_embed_footer, key, value) {
        if (json_is_string(value)) {
            cord_strbuf_t *builder =
                cord_strbuf_create_with_allocator(allocator);
            cord_strbuf_append(builder, cstr(json_string_value(value)));

            map_property(embed_footer, text, "text", key, builder);
            map_property(embed_footer, icon_url, "icon_url", key, builder);
            map_property(embed_footer, proxy_icon_url, "proxy_icon_url", key,
                         builder);
        }
    }

    return serialized(embed_footer);
}

cord_serialize_result_t
cord_embed_image_serialize(json_t *json_embed_image, cord_bump_t *allocator,
                           cord_embed_image_t *array_slot) {
    cord_embed_image_t *embed_image =
        array_slot ? array_slot : balloc(allocator, sizeof(cord_embed_image_t));
    if (!embed_image) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_embed_image_init(embed_image, allocator);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(json_embed_image, key, value) {
        if (json_is_string(value)) {
            cord_strbuf_t *builder =
                cord_strbuf_create_with_allocator(allocator);
            cord_strbuf_append(builder, cstr(json_string_value(value)));

            map_property(embed_image, url, "url", key, builder);
            map_property(embed_image, proxy_url, "proxy_url", key, builder);
        } else if (json_is_integer(value)) {
            map_property(embed_image, height, "height", key,
                         (i32)json_integer_value(value));
            map_property(embed_image, width, "width", key,
                         (i32)json_integer_value(value));
        }
    }

    return serialized(embed_image);
}

cord_serialize_result_t
cord_embed_thumbnail_serialize(json_t *json_embed_thumbnail,
                               cord_bump_t *allocator,
                               cord_embed_thumbnail_t *array_slot) {
    cord_embed_thumbnail_t *embed_thumbnail =
        array_slot ? array_slot
                   : balloc(allocator, sizeof(cord_embed_thumbnail_t));

    if (!embed_thumbnail) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_embed_thumbnail_init(embed_thumbnail, allocator);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(json_embed_thumbnail, key, value) {
        if (json_is_string(value)) {
            cord_strbuf_t *builder =
                cord_strbuf_create_with_allocator(allocator);
            cord_strbuf_append(builder, cstr(json_string_value(value)));

            map_property(embed_thumbnail, url, "url", key, builder);
            map_property(embed_thumbnail, proxy_url, "proxy_url", key, builder);
        } else if (json_is_integer(value)) {
            map_property(embed_thumbnail, height, "height", key,
                         (i32)json_integer_value((value)));
            map_property(embed_thumbnail, width, "width", key,
                         (i32)json_integer_value((value)));
        }
    }

    return serialized(embed_thumbnail);
}

cord_serialize_result_t
cord_embed_video_serialize(json_t *json_embed_video, cord_bump_t *allocator,
                           cord_embed_video_t *array_slot) {
    cord_embed_video_t *embed_video =
        array_slot ? array_slot : balloc(allocator, sizeof(cord_embed_video_t));
    if (!embed_video) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_embed_video_init(embed_video, allocator);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(json_embed_video, key, value) {
        if (json_is_string(value)) {
            cord_strbuf_t *builder =
                cord_strbuf_create_with_allocator(allocator);
            cord_strbuf_append(builder, cstr(json_string_value(value)));

            map_property(embed_video, url, "url", key, builder);
        } else if (json_is_integer(value)) {
            map_property(embed_video, height, "height", key,
                         (i32)json_integer_value(value));
            map_property(embed_video, width, "width", key,
                         (i32)json_integer_value(value));
        }
    }
    return serialized(embed_video);
}

cord_serialize_result_t
cord_embed_provider_serialize(json_t *json_message, cord_bump_t *allocator,
                              cord_embed_provider_t *array_slot) {
    cord_embed_provider_t *embed_provider =
        array_slot ? array_slot
                   : balloc(allocator, sizeof(cord_embed_provider_t));
    if (!embed_provider) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_embed_provider_init(embed_provider, allocator);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(json_message, key, value) {
        if (json_is_string(value)) {
            cord_strbuf_t *builder =
                cord_strbuf_create_with_allocator(allocator);
            cord_strbuf_append(builder, cstr(json_string_value(value)));

            map_property(embed_provider, name, "name", key, builder);
            map_property(embed_provider, url, "url", key, builder);
        }
    }
    return serialized(embed_provider);
}

cord_serialize_result_t
cord_embed_author_serialize(json_t *json_embed_author, cord_bump_t *allocator,
                            cord_embed_author_t *array_slot) {
    cord_embed_author_t *embed_author =
        array_slot ? array_slot
                   : balloc(allocator, sizeof(cord_embed_author_t));
    if (!embed_author) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_embed_author_init(embed_author, allocator);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(json_embed_author, key, value) {
        if (json_is_string(value)) {
            cord_strbuf_t *builder =
                cord_strbuf_create_with_allocator(allocator);
            cord_strbuf_append(builder, cstr(json_string_value(value)));

            map_property(embed_author, name, "name", key, builder);
            map_property(embed_author, url, "url", key, builder);
            map_property(embed_author, icon_url, "icon_url", key, builder);
            map_property(embed_author, proxy_icon_url, "proxy_icon_url", key,
                         builder);
        }
    }
    return serialized(embed_author);
}

cord_serialize_result_t
cord_embed_field_serialize(json_t *json_embed_field, cord_bump_t *allocator,
                           cord_embed_field_t *array_slot) {
    cord_embed_field_t *embed_field =
        array_slot ? array_slot : balloc(allocator, sizeof(cord_embed_field_t));
    if (!embed_field) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_embed_field_init(embed_field, allocator);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(json_embed_field, key, value) {
        if (json_is_string(value)) {
            cord_strbuf_t *builder =
                cord_strbuf_create_with_allocator(allocator);
            cord_strbuf_append(builder, cstr(json_string_value(value)));

            map_property(embed_field, name, "name", key, builder);
            map_property(embed_field, value, "value", key, builder);
        } else if (json_is_boolean(value)) {
            map_property(embed_field, inline_, "inline", key,
                         (bool)json_boolean_value(value));
        }
    }
    return serialized(embed_field);
}

cord_serialize_result_t cord_embed_serialize(json_t *json_embed,
                                             cord_bump_t *allocator) {
    cord_embed_t *embed = balloc(allocator, sizeof(cord_embed_t));
    if (!embed) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_embed_init(embed, allocator);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(json_embed, key, value) {
        if (json_is_string(value)) {
            cord_strbuf_t *builder =
                cord_strbuf_create_with_allocator(allocator);
            cord_strbuf_append(builder, cstr(json_string_value(value)));

            map_property(embed, title, "title", key, builder);
            map_property(embed, type, "type", key, builder);
            map_property(embed, description, "description", key, builder);
            map_property(embed, url, "url", key, builder);
            map_property(embed, timestamp, "timestamp", key, builder);
        } else if (json_is_integer(value)) {
            map_property(embed, color, "color", key,
                         (i32)json_integer_value(value));
        } else if (json_is_object(value)) {
            map_property_object_collectible(embed, footer, "footer", key, value,
                                            allocator, cord_embed_footer_t,
                                            cord_embed_footer_serialize);

            map_property_object_collectible(embed, image, "image", key, value,
                                            allocator, cord_embed_image_t,
                                            cord_embed_image_serialize);

            map_property_object_collectible(
                embed, thumbnail, "thumbnail", key, value, allocator,
                cord_embed_thumbnail_t, cord_embed_thumbnail_serialize);

            map_property_object_collectible(
                embed, video, "video", key, value, allocator,
                cord_embed_video_serialize_t, cord_embed_video_serialize);

            map_property_object_collectible(
                embed, provider, "provider", key, value, allocator,
                cord_embed_provider_t, cord_embed_provider_serialize);

            map_property_object_collectible(embed, author, "author", key, value,
                                            allocator, cord_embed_author_t,
                                            cord_embed_author_serialize);
        } else if (json_is_array(value)) {
            map_property_array(embed, fields, "fields", key, value, allocator,
                               cord_embed_field_t, cord_embed_field_serialize);
        }
    }

    return serialized(embed);
}

cord_serialize_result_t cord_emoji_serialize(json_t *json_emoji,
                                             cord_bump_t *allocator) {
    cord_emoji_t *emoji = balloc(allocator, sizeof(cord_emoji_t));
    if (!emoji) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_emoji_init(emoji, allocator);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(json_emoji, key, value) {
        if (json_is_string(value)) {
            cord_strbuf_t *builder =
                cord_strbuf_create_with_allocator(allocator);
            cord_strbuf_append(builder, cstr(json_string_value(value)));

            map_property(emoji, id, "id", key, builder);
            map_property(emoji, name, "name", key, builder);
        } else if (json_is_boolean(value)) {
            map_property(emoji, require_colons, "require_colons", key,
                         (bool)json_boolean_value(value));
            map_property(emoji, managed, "managed", key,
                         (bool)json_boolean_value(value));
            map_property(emoji, animated, "animated", key,
                         (bool)json_boolean_value(value));
            map_property(emoji, available, "available", key,
                         (bool)json_boolean_value(value));
        } else if (json_is_object(value)) {
            map_property_object(emoji, user, "user", key, value, allocator,
                                cord_user_t, cord_user_serialize);
        } else if (json_is_array(value)) {
            map_property_array(emoji, roles, "roles", key, value, allocator,
                               cord_role_t, cord_role_serialize);
        }
    }
    return serialized(emoji);
}

cord_serialize_result_t cord_reaction_serialize(json_t *json_reaction,
                                                cord_bump_t *allocator) {
    cord_reaction_t *reaction = balloc(allocator, sizeof(cord_reaction_t));
    if (!reaction) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_reaction_init(reaction, allocator);
    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(json_reaction, key, value) {
        if (json_is_integer(value)) {
            map_property(reaction, count, "count", key,
                         (i32)json_integer_value(value));
        } else if (json_is_boolean(value)) {
            map_property(reaction, me, "me", key,
                         (bool)json_boolean_value(value));
        } else if (json_is_object(value)) {
            map_property_object(reaction, emoji, "emoji", key, value, allocator,
                                cord_emoji_t, cord_emoji_serialize);
        }
    }
    return serialized(reaction);
}

cord_serialize_result_t
cord_message_activity_serialize(json_t *json_message_activity,
                                cord_bump_t *allocator) {
    cord_message_activity_t *message_activity =
        balloc(allocator, sizeof(cord_message_activity_t));
    if (!message_activity) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_message_activity_init(message_activity, allocator);
    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(json_message_activity, key, value) {
        if (json_is_integer(value)) {
            map_property(message_activity, type, "type", key,
                         (i32)json_integer_value(value));
        } else if (json_is_string(value)) {
            cord_strbuf_t *builder =
                cord_strbuf_create_with_allocator(allocator);
            cord_strbuf_append(builder, cstr(json_string_value(value)));

            map_property(message_activity, party_id, "party_id", key, builder);
        }
    }
    return serialized(message_activity);
}

cord_serialize_result_t
cord_message_application_serialize(json_t *json_message,
                                   cord_bump_t *allocator) {
    cord_message_application_t *message_app =
        balloc(allocator, sizeof(cord_message_application_t));
    if (!message_app) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_message_application_init(message_app, allocator);
    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(json_message, key, value) {
        if (json_is_string(value)) {
            cord_strbuf_t *builder =
                cord_strbuf_create_with_allocator(allocator);
            ;
            cord_strbuf_append(builder, cstr(json_string_value(value)));

            map_property(message_app, id, "id", key, builder);
            map_property(message_app, cover_image, "cover_image", key, builder);
            map_property(message_app, description, "description", key, builder);
            map_property(message_app, icon, "icon", key, builder);
            map_property(message_app, name, "name", key, builder);
        }
    }
    return serialized(message_app);
}

cord_serialize_result_t
cord_message_sticker_serialize(json_t *json_message_sticker,
                               cord_bump_t *allocator) {
    cord_message_sticker_t *message_sticker =
        balloc(allocator, sizeof(cord_message_sticker_t));
    if (!message_sticker) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_message_sticker_init(message_sticker, allocator);
    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(json_message_sticker, key, value) {
        if (json_is_string(value)) {
            cord_strbuf_t *builder =
                cord_strbuf_create_with_allocator(allocator);
            cord_strbuf_append(builder, cstr(json_string_value(value)));

            map_property(message_sticker, id, "id", key, builder);
            map_property(message_sticker, pack_id, "pack_id", key, builder);
            map_property(message_sticker, name, "name", key, builder);
            map_property(message_sticker, description, "description", key,
                         builder);
            map_property(message_sticker, tags, "tags", key, builder);
            map_property(message_sticker, asset, "asset", key, builder);
            map_property(message_sticker, preview_asset, "preview_asset", key,
                         builder);
        } else if (json_is_integer(value)) {
            map_property(message_sticker, format_type, "format_type", key,
                         (i32)json_integer_value(value));
        }
    }
    return serialized(message_sticker);
}

static void message_strings(cord_message_t *message, string_ref key,
                            string_ref cstring) {
    cord_strbuf_t *builder =
        cord_strbuf_create_with_allocator(message->allocator);
    cord_strbuf_append(builder, cstr(cstring));

    logger_debug("Message strings  %s %s", key, cstring);

    map_property(message, id, "id", key, builder);
    map_property(message, channel_id, "channel_id", key, builder);
    map_property(message, guild_id, "guild_id", key, builder);
    map_property(message, content, "content", key, builder);
    map_property(message, timestamp, "timestamp", key, builder);
    map_property(message, edited_timestamp, "edited_timestamp", key, builder);
    map_property(message, nonce, "nonce", key, builder);
    map_property(message, webhook_id, "webhook_id", key, builder);
}

static void message_booleans(cord_message_t *message, string_ref key,
                             bool value) {
    map_property(message, tts, "tts", key, value);
    map_property(message, mention_everyone, "mention_everyone", key, value);
    map_property(message, pinned, "pinned", key, value);
}

static void message_numbers(cord_message_t *message, string_ref key,
                            i32 value) {
    map_property(message, type, "type", key, value);
    map_property(message, flags, "flags", key, value);
}

cord_serialize_result_t cord_message_serialize(json_t *json_message,
                                               cord_bump_t *allocator) {
    cord_message_t *message = balloc(allocator, sizeof(cord_message_t));
    if (!message) {
        logger_error("Failed to allocate discord message");
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_message_init(message, allocator);

    string_ref key = NULL;
    json_t *value = NULL;
    json_object_foreach(json_message, key, value) {
        if (json_is_string(value)) {
            message_strings(message, key, json_string_value(value));
        } else if (json_is_boolean(value)) {
            message_booleans(message, key, (bool)json_boolean_value(value));
        } else if (json_is_integer(value)) {
            message_numbers(message, key, (i32)json_integer_value(value));
        } else if (json_is_array(value)) {
            // TODO: Add these
        } else if (json_is_object(value)) {
            // TODO: Add these
        }
    }
    return serialized(message);
}

cord_serialize_result_t cord_guild_serialize(json_t *json_message,
                                             cord_bump_t *allocator) {
    cord_guild_t *guild = balloc(allocator, sizeof(cord_guild_t));
    if (!guild) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_guild_init(guild, allocator);
    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(json_message, key, value) {
        if (json_is_string(value)) {
            cord_strbuf_t *builder =
                cord_strbuf_create_with_allocator(allocator);
            cord_strbuf_append(builder, cstr(json_string_value(value)));

            map_property(guild, id, "id", key, builder);
            map_property(guild, name, "name", key, builder);
            map_property(guild, icon, "icon", key, builder);
            map_property(guild, splash, "splash", key, builder);
            map_property(guild, discovery_splash, "discovery_splash", key,
                         builder);
        }
    }
    return serialized(guild);
}
