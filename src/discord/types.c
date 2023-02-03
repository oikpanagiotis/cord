#include "types.h"
#include "../core/strings.h"
#include "../core/log.h"
#include "../core/errors.h"
#include "../core/util.h"
#include "../core/array.h"
#include "../core/typedefs.h"
#include "error.h"

#include <jansson.h>
#include <assert.h>


static bool has_error(cord_serialize_result_t result) {
    return result.error == CORD_OK;
}

static cord_serialize_result_t serialized(void *obj) {
    return (cord_serialize_result_t){obj, CORD_OK};
}

static cord_serialize_result_t serialize_error(cord_error_t error) {
    return (cord_serialize_result_t){NULL, error};
}

void cord_user_init(cord_user_t *user, cord_bump_t *allocator) {
    user->id = NULL;
    user->username = NULL;
    user->discriminator = NULL;
    user->avatar = NULL;
    user->bot = false;
    user->system_ = false;
    user->mfa_enabled = false;
    user->locale = NULL;
    user->verified = false;
    user->email = NULL;
    user->flags = 0;
    user->premium_type = 0;
    user->public_flags = 0;

    assert(allocator && "Allocator passed to user init must not be null");
    user->allocator = allocator;
}

static void user_read_boolean_json_field(cord_user_t *author, const char *key, bool value) {
    map_property(author, bot, "bot", key, value);
    map_property(author, system_, "system", key, value);
    map_property(author, mfa_enabled, "mfa_enabled", key, value);
    map_property(author, verified, "varified", key, value);
}

static void user_read_string_json_field(cord_user_t *user, string_ref key, string_ref value) {
    cord_strbuf_t *field_value = cord_strbuf_create_with_allocator(user->allocator);
    cord_strbuf_append(field_value, cstr(value));

    map_property(user, id, "id", key, field_value);
    map_property(user, username, "username", key, field_value);
    map_property(user, discriminator, "discriminator", key, field_value);
    map_property(user, avatar, "avatar", key, field_value);
    map_property(user, locale, "locale", key, field_value);
    map_property(user, email, "email", key, field_value);
}

static void user_read_integer_json_field(cord_user_t *user, string_ref key, i64 value) {
    map_property(user, flags, "flags", key, value);
    map_property(user, premium_type, "premium_type", key, value);
    map_property(user, public_flags, "public_flags", key, value);
}

// Jansson docs:https://jansson.readthedocs.io/en/latest/apiref.html#custom-memory-allocation
// Use custom arena just for json
cord_serialize_result_t cord_user_serialize(json_t *user, cord_bump_t *allocator) {
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
            bool boolean = json_boolean_value(value);
            user_read_boolean_json_field(author, key, boolean);
        } else if (json_is_integer(value)) {
            i64 boolean = (i64)json_integer_value(value);
            user_read_integer_json_field(author, key, boolean);
        }
    }

    return serialized(author);
}

void cord_guild_member_init(cord_guild_member_t *member, cord_bump_t *allocator) {
    cord_user_init(member->user, allocator);
    member->nick = NULL;
    member->roles = NULL;
    member->joined_at = NULL;
    member->premium_since = NULL;
    member->deaf = false;
    member->mute = false;
    member->pending = false;

    assert(allocator && "Allocator for cord_guild_member_t can not be null");
    member->allocator = allocator;
}

static void guild_member_booleans(cord_guild_member_t *member,
                                               string_ref key,
                                               bool value) {
    map_property(member, deaf, "deaf", key, value);
    map_property(member, mute, "mute", key, value);
    map_property(member, pending, "pending", key, value);
}

static void guild_member_strings(cord_guild_member_t *member, string_ref key, string_ref cstring) {
    cord_strbuf_t *builder = cord_strbuf_create_with_allocator(member->allocator);
    cord_strbuf_append(builder, cstr(cstring));

    map_property(member, nick, "nick", key, builder);
    map_property(member, joined_at, "joined_at", key, builder);
    map_property(member, premium_since, "premium_since", key, builder);
}

cord_serialize_result_t cord_guild_member_serialize(json_t *data, cord_bump_t *allocator) {
    cord_guild_member_t *member = balloc(allocator, sizeof(cord_guild_member_t));
    if (!member) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_guild_member_init(member, member->allocator);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            string_ref cstring = json_string_value(value);
            guild_member_strings(member, key, cstring);
        } else if (json_is_boolean(value)) {
            bool boolean = json_boolean_value(value);
            guild_member_booleans(member, key, boolean);
        } else if (json_is_array(value)) {
            size_t index = 0;
            json_t *item = NULL;

            if (string_is_equal(key, "roles")) {
                member->roles = cord_array_create(allocator, sizeof(cord_role_t));
                json_array_foreach(value, index, item) {

                    cord_role_t *array_slot = cord_array_push(member->roles);
                    cord_serialize_result_t role = cord_role_serialize(item, allocator, array_slot);

                    if (has_error(role)) {
                        logger_error("Failed to serialize cord_role_t: %s", cord_error(role.error));
                        break;
                    }
                }
            }
        } else if (json_is_object(value)) {
            if (string_is_equal(key, "user")) {
                cord_serialize_result_t result = cord_user_serialize(value, allocator);
                if (is_posix_error(result.error)) {
                    // Skip field and keep serializing the rest of the object
                    logger_error("Failed to serialize user as part of guild member serialize (%s)",
                            cord_error(result.error));
                } else {
                    member->user = (cord_user_t *)result.obj;
                }
            }
        }
    }

    return serialized(member);
}

void cord_role_init(cord_role_t *role, cord_bump_t *allocator) {
    role->id = NULL;
    role->name = NULL;
    role->color = 0;
    role->hoist = false;
    role->position = 0;
    role->permissions = NULL;
    role->managed = false;
    role->mentionable = false;

    assert(allocator && "Allocator passed to role init must not be null");
    role->allocator = allocator;
}

static void role_strings(cord_role_t *role, string_ref key, string_ref cstring) {
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

cord_serialize_result_t cord_role_serialize(json_t *data, cord_bump_t *allocator, cord_role_t *array_slot) {
    cord_role_t *role = array_slot ? array_slot :
                                     balloc(allocator, sizeof(cord_role_t));
    if (!role) {
        return serialize_error(CORD_ERR_MALLOC);
    }
    
    cord_role_init(role, allocator);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            string_ref cstring = json_string_value(value);
            role_strings(role, key, cstring);
        } else if (json_is_boolean(value)) {
            bool boolean = json_boolean_value(value);
            role_booleans(role, key, boolean);
        } else if (json_is_integer(value)) {
            i32 number = (i32)json_integer_value(value);
            role_numbers(role, key, number);
        } else if (json_is_array(value)) {
            size_t index = 0;
            json_t *item = NULL;

            if (string_is_equal(key, "tags")) {
                role->tags = cord_array_create(allocator, sizeof(cord_role_tag_t));
                json_array_foreach(value, index, item) {

                    cord_role_tag_t *array_slot = cord_array_push(role->tags);
                    cord_serialize_result_t role_tag = cord_role_tag_serialize(item, allocator, array_slot);

                    if (has_error(role_tag)) {
                        logger_error("Failed to serialize cord_role_tag_t: %s", cord_error(role_tag.error));
                        break;
                    }
                }
            }
        }
    }
    return serialized(role);
}

static void role_tag_strings(cord_role_tag_t *role_tag, string_ref key, string_ref cstring) {
    cord_strbuf_t *builder = cord_strbuf_create_with_allocator(role_tag->allocator);
    cord_strbuf_append(builder, cstr(cstring));

    map_property(role_tag, bot_id, "bot_id", key, builder);
    map_property(role_tag, integration_id, "integration_id", key, builder);
    map_property(role_tag, subscription_listing_id, "subscription_listing_id", key, builder);
}

static void role_tag_booleans(cord_role_tag_t *role_tag, string_ref key, bool boolean) {
    map_property(role_tag, premium_subscriber, "premium_subscriber", key, boolean);
    map_property(role_tag, available_for_purchase, "available_for_purchase", key, boolean);
    map_property(role_tag, guild_connections, "guild_connections", key, boolean);
}

void cord_role_tag_init(cord_role_tag_t *role_tag, cord_bump_t *allocator) {
    role_tag->available_for_purchase = false;
    role_tag->bot_id = NULL;
    role_tag->guild_connections = false;
    role_tag->integration_id = NULL;
    role_tag->premium_subscriber = false;
    role_tag->subscription_listing_id = NULL;
    role_tag->allocator = allocator;
}

cord_serialize_result_t cord_role_tag_serialize(json_t *json_role_tag, cord_bump_t *allocator, cord_role_tag_t *array_slot) {
    cord_role_tag_t *role_tag = array_slot ? array_slot :
                                                balloc(allocator, sizeof(cord_role_t));

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
            bool boolean = json_boolean_value(value);
            role_tag_booleans(role_tag, key, boolean);
        }
    }

    return serialized(role_tag);
}


void cord_channel_mention_init(cord_channel_mention_t *mention, cord_bump_t *allocator) {
    mention->id = NULL;
    mention->guild_id = NULL;
    mention->type = 0;
    mention->name = NULL;
    mention->allocator = allocator;
}


static void channel_mention_strings(cord_channel_mention_t *mention, string_ref key, string_ref value) {
    cord_strbuf_t *string_buffer = cord_strbuf_create_with_allocator(mention->allocator);
    cord_strbuf_append(string_buffer, cstr(value));
    
    map_property(mention, id, "id", key, string_buffer);
    map_property(mention, guild_id, "guild_id", key, string_buffer);
    map_property(mention, name, "name", key, string_buffer);
}

static void channel_mention_numbers(cord_channel_mention_t *mention, string_ref key, i64 value) {
    map_property(mention, type, "type", key, value);
}

cord_serialize_result_t cord_channel_mention_serialize(json_t *json_mention, cord_bump_t *allocator) {
    cord_channel_mention_t *mention = balloc(allocator, sizeof(cord_channel_mention_t));
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
        } else if (json_is_number(value)) {
            i64 number = json_number_value(json_mention);
            channel_mention_numbers(mention, key, number);
        }
    }

    return serialized(mention);
}

void cord_attachment_init(cord_attachment_t *at) {
}

cord_attachment_t *cord_attachment_serialize(json_t *data, cord_error_t *err) {
    cord_attachment_t *at = malloc(sizeof(cord_attachment_t));
    if (!at) {
        return serialize_error(CORD_ERR_MALLOC);
    }

    cord_attachment_init(at);
    return at;
}

void cord_embed_footer_init(cord_embed_footer_t *ft) {
}

cord_embed_footer_t *cord_embed_footer_serialize(json_t *data, cord_error_t *err) {
    cord_embed_footer_t *ft = malloc(sizeof(cord_embed_footer_t));
    if (!ft) {
        return serialized(err, CORD_ERR_MALLOC);
    }

    cord_embed_footer_init(ft);

    return ft;
}

void cord_embed_image_init(cord_embed_image_t *img) {
}

cord_embed_image_t *cord_embed_image_serialize(json_t *data, cord_error_t *err) {
    cord_embed_image_t *img = malloc(sizeof(cord_embed_image_t));
    if (!img) {
        return serialized(err, CORD_ERR_MALLOC);
    }

    cord_embed_image_init(img);
    return img;
}

void cord_embed_thumbnail_init(cord_embed_thumbnail_t *tn) {
}

cord_embed_thumbnail_t *cord_embed_thumbnail_serialize(json_t *data, cord_error_t *err) {
    cord_embed_thumbnail_t *tn = malloc(sizeof(cord_embed_thumbnail_t));
    if (!tn) {
        return serialized(err, CORD_ERR_MALLOC);
    }

    cord_embed_thumbnail_init(tn);
    return tn;
}

void cord_embed_video_init(cord_embed_video_t *evid) {
}

cord_embed_video_t *cord_embed_video_serialize(json_t *data, cord_error_t *err) {
    cord_embed_video_t *evid = malloc(sizeof(cord_embed_video_t));
    if (!evid) {
        return serialized(err, CORD_ERR_MALLOC);
    }

    cord_embed_video_init(evid);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds boolean = sdsnew(json_string_value(value));
            map_property(evid, url, "url", key, boolean);
        } else if (json_is_integer(value)) {
            int val = (int)json_integer_value(value);
            map_property(evid, height, "height", key, val);
            map_property(evid, width, "width", key, val);
        }
    }
    return evid;
}

void cord_embed_provider_init(cord_embed_provider_t *epr) {
    epr->name = NULL;
    epr->url = NULL;
}

cord_embed_provider_t *cord_embed_provider_serialize(json_t *data, cord_error_t *err) {
    cord_embed_provider_t *epr = malloc(sizeof(cord_embed_provider_t));
    if (!epr) {
        return serialized(err, CORD_ERR_MALLOC);
    }

    cord_embed_provider_init(epr);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds boolean = sdsnew(json_string_value(value));
            map_property(epr, name, "name", key, boolean);
            map_property(epr, url, "url", key, boolean);
        }
    }
    return epr;
}

void cord_embed_author_init(cord_embed_author_t *eauth) {
    eauth->name = NULL;
    eauth->url = NULL;
    eauth->icon_url = NULL;
    eauth->proxy_icon_url = NULL;
}

cord_embed_author_t *cord_embed_author_serialize(json_t *data, cord_error_t *err) {
    cord_embed_author_t *eauth = malloc(sizeof(cord_embed_author_t));
    if (!eauth) {
        return serialized(err, CORD_ERR_MALLOC);
    }
    
    cord_embed_author_init(eauth);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds boolean = sdsnew(json_string_value(value));
            map_property(eauth, name, "name", key, boolean);
            map_property(eauth, url, "url", key, boolean);
            map_property(eauth, icon_url, "icon_url", key, boolean);
            map_property(eauth, proxy_icon_url, "proxy_icon_url", key, boolean);
        }
    }
    return eauth;
}

void cord_embed_field_init(cord_embed_field_t *efield) {
    efield->name = NULL;
    efield->value = NULL;
    efield->inline_ = false;
}

cord_embed_field_t *cord_embed_field_serialize(json_t *data, cord_error_t *err) {
    cord_embed_field_t *efield = malloc(sizeof(cord_embed_field_t));
    if (!efield) {
        return serialized(err, CORD_ERR_MALLOC);
    }

    cord_embed_field_init(efield);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds boolean = sdsnew(json_string_value(value));
            map_property(efield, name, "name", key, boolean);
            map_property(efield, value, "value", key, boolean);
        } else if (json_is_boolean(value)) {
            bool val = json_boolean_value(value);
            map_property(efield, inline_, "inline", key, val);
        }
    }
    return efield;
}

void cord_emoji_init(cord_emoji_t *emj) {
    emj->id = NULL;
    emj->name = NULL;
    emj->roles = NULL;
    emj->user = NULL;
    emj->require_colons = false;
    emj->managed = false;
    emj->animated = false;
    emj->available = false;
}

cord_emoji_t *cord_emoji_serialize(json_t *data, cord_error_t *err) {
    cord_emoji_t *emj = malloc(sizeof(cord_emoji_t));
    if (!emj) {
        return serialized(err, CORD_ERR_MALLOC);
    }

    cord_emoji_init(emj);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds boolean = sdsnew(json_string_value(value));
            map_property(emj, id, "id", key, boolean);
            map_property(emj, name, "name", key, boolean);
        } else if (json_is_boolean(value)) {
            bool val = json_boolean_value(value);
            map_property(emj, require_colons, "require_colons", key, val);
            map_property(emj, managed, "managed", key, val);
            map_property(emj, animated, "animated", key, val);
            map_property(emj, available, "available", key, val);
        } else if (json_is_object(value)) {
            json_t *obj = value;
            (void)obj;
            if (string_is_equal(key, "roles")) {
                // TODO: roles array
            } else if (string_is_equal(key, "user")) {
                // TODO: FIX AFTER REFACTOR

                // cord_user_t *user_obj = NULL;
                // int err = cord_user_init(user_obj);
                // if (err != CORD_OK) {
                    // return err;
                // }
                // err = cord_user_serialize(user_obj, obj);
                // if (err != CORD_OK) {
                    // return err;
                // }

                // In this case user is the name of the struct field
                // and user_obj is the serialized object
                // map_property(emj, user, "user", key, user_obj);
            }
        }
    }
    return emj;
}

void cord_reaction_init(cord_reaction_t *react) {
    react->count = 0;
    react->me = false;
    react->emoji = NULL;
}

cord_reaction_t *cord_reaction_serialize(json_t *data, cord_error_t *err) {
    cord_reaction_t *react = malloc(sizeof(cord_reaction_t));
    if (!react) {
        return serialized(err, CORD_ERR_MALLOC);
    }
    
    cord_reaction_init(react);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_integer(value)) {
            int val = (int)json_integer_value(value);
            map_property(react, count, "count", key, val);
        } else if (json_is_boolean(value)) {
            bool val = json_boolean_value(value);
            map_property(react, me, "me", key, val);
        } else if (json_is_object(value)) {
            json_t *obj = value;
            if (string_is_equal(key, "emoji")) {
                cord_error_t error = 0;
                cord_emoji_t *emoji_obj = cord_emoji_serialize(obj, &error);
                if (!emoji_obj) {
                    logger_error("%s", cord_error(error));
                    return serialized(err, error);
                }
                map_property(react, emoji, "emoji", key, emoji_obj);
            }
        }
    }
    return react;
}

void cord_message_activity_init(cord_message_activity_t *ma) {
    ma->type = 0;
    ma->party_id = NULL;
}

cord_message_activity_t *cord_message_activity_serialize(json_t *data, cord_error_t *err) {
    cord_message_activity_t *ma = malloc(sizeof(cord_message_activity_t));
    if (!ma) {
        return serialized(err, CORD_ERR_MALLOC);
    }

    cord_message_activity_init(ma);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_integer(value)) {
            int val = (int)json_integer_value(value);
            map_property(ma, type, "type", key, val);
        } else if (json_is_string(value)) {
            sds copy_value = sdsnew(json_string_value(value));
            map_property(ma, party_id, "party_id", key, copy_value);
        }
    }
    return ma;
}

void cord_message_application_init(cord_message_application_t *app, cord_bump_t *allocator) {
    app->id = NULL;
    app->cover_image = NULL;
    app->description = NULL;
    app->icon = NULL;
    app->name = NULL;

    app->allocator = NULL;
}

cord_serialize_result_t cord_message_application_serialize(json_t *data, cord_bump_t *allocator) {
    cord_message_application_t *app = malloc(sizeof(cord_message_application_t));
    if (!app) {
        return serialized(err, CORD_ERR_MALLOC);
    }

    cord_message_application_init(app);
    
    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            cord_strbuf_t *copy_val = cord_strbuf_create_with_allocator(app->allocator);;
            cord_strbuf_append(copy_val, cstr(json_string_value(value)));

            map_property(app, id, "id", key, copy_val);
            map_property(app, cover_image, "cover_image", key, copy_val);
            map_property(app, description, "description", key, copy_val);
            map_property(app, icon, "icon", key, copy_val);
            map_property(app, name, "name", key, copy_val);
        }
    }

    return serialized(app, CORD_OK);
}

// int cord_embed_init(cord_embed_t *emb, cord_bump_t *allocator) {
//     // TODO: Implement
//     (void)emb;
//     return CORD_OK;
// }

// int cord_embed_serialize(json_t *emb, cord_embed_t *data) {
//     // TODO: Implement
//     (void)emb;
//     (void)data;
//     return CORD_OK;
// }

// void cord_message_reference_init(cord_message_reference_t *mr) {
//     mr->message_id = NULL;
//     mr->channel_id = NULL;
//     mr->guild_id = NULL;
// }

// cord_message_reference_t *cord_message_reference_serialize(json_t *data, cord_error_t *err) {
//     cord_message_reference_t *mr = malloc(sizeof(cord_message_reference_t));
//     if (!mr) {
//         return serialized(err, CORD_ERR_MALLOC);
//     }

//     cord_message_reference_init(mr);

//     string_ref key = NULL;
//     json_t *value = NULL;

//     json_object_foreach(data, key, value) {
//         if (json_is_string(value)) {
//             sds copy_val = sdsnew(json_string_value(value));
//             map_property(mr, message_id, "message_id", key, copy_val);
//             map_property(mr, channel_id, "channel_id", key, copy_val);
//             map_property(mr, guild_id, "guild_id", key, copy_val);
//         }
//     }
//     return mr;
// }

void cord_message_sticker_init(cord_message_sticker_t *ms, cord_bump_t *allocator) {
    ms->id = NULL;
    ms->pack_id = NULL;
    ms->name = NULL;
    ms->description = NULL;
    ms->tags = NULL;
    ms->asset = NULL;
    ms->preview_asset = NULL;
    ms->format_type = 0; // https://discord.com/developers/docs/resources/channel#message-object-message-sticker-format-types
}

/* UPDATE SIGNATURE
cord_message_sticker_t *cord_message_sticker_serialize(json_t *data, cord_error_t *err) {
    cord_message_sticker_t *ms = malloc(sizeof(cord_message_sticker_t));
    if (!ms) {
        return serialized(err, CORD_ERR_MALLOC);
    }

    cord_message_sticker_init(ms);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds copy_val = sdsnew(json_string_value(value));
            map_property(ms, id, "id", key, copy_val);
            map_property(ms, pack_id, "pack_id", key, copy_val);
            map_property(ms, description, "description", key, copy_val);
            map_property(ms, tags, "tags", key, copy_val);
            map_property(ms, asset, "asset", key, copy_val);
            map_property(ms, preview_asset, "preview_asset", key, copy_val);
        } else if (json_is_integer(value)) {
            int val = (int)json_integer_value(value);
            map_property(ms, format_type, "format_type", key, val);
        }
    }
    return ms;
}
*/

void cord_message_init(cord_message_t *msg, cord_bump_t *allocator) {
    msg->id = NULL;
    msg->channel_id = NULL;
    msg->guild_id = NULL;
    msg->author = NULL;
    msg->member = NULL;
    msg->content = NULL;
    msg->timestamp = NULL;
    msg->edited_timestamp = NULL;
    msg->tts = false;
    msg->mention_everyone = false;

    msg->nonce = NULL;
    msg->pinned = false;
    msg->webhook_id = NULL;
    msg->type = -1; // 0 is used by the API so we initialize type to 0
    msg->activity = NULL;
    msg->application = NULL;
    msg->message_reference = NULL;
    msg->flags = 0;
    msg->referenced_message = NULL;

    msg->allocator = allocator;
}

static void message_strings(cord_message_t *message, string_ref key, cord_strbuf_t *value) {
    map_property(message, id, "id", key, value);
    map_property(message, channel_id, "channel_id", key, value);
    map_property(message, guild_id, "guild_id", key, value);
    map_property(message, content, "content", key, value);
    map_property(message, timestamp, "timestamp", key, value);
    map_property(message, edited_timestamp, "edited_timestamp", key, value);
    map_property(message, nonce, "nonce", key, value);
    map_property(message, webhook_id, "webhook_id", key, value);
}

static void message_booleans(cord_message_t *message, string_ref key, bool value) {
    map_property(message, tts, "tts", key, value);
    map_property(message, mention_everyone, "mention_everyone", key, value);
    map_property(message, pinned, "pinned", key, value);
}

static void message_numbers(cord_message_t *message, string_ref key, i64 value) {
    map_property(message, type, "type", key, value);
	map_property(message, flags, "flags", key, value);
}

cord_serialize_result_t cord_message_serialize(json_t *data, cord_bump_t *allocator) {
    cord_message_t *message = balloc(allocator, sizeof(cord_message_t));
    if (!message) {
		logger_error("Failed to allocate discord message");
        return serialized(message, CORD_ERR_MALLOC);
	}

    cord_message_init(message, allocator);

	string_ref key = NULL;
	json_t *value = NULL;
	json_object_foreach(data, key, value) {
		if (json_is_string(value)) {

            cord_strbuf_t *boolean = cord_strbuf_create_with_allocator(allocator);
            cord_strbuf_append(boolean, cstr(json_string_value(value)));
        
            message_strings(message, key, boolean);

		} else if (json_is_boolean(value)) {
			bool boolean = json_boolean_value(value);
            message_booleans(message, key, boolean);
        } else if (json_is_integer(value)) {
			i64 boolean = (i64)json_integer_value(value);
            message_numbers(message, key, boolean);
		} else if (json_is_array(value)) {
            if (string_is_equal("mentions", key)) {
                /*
                msg->mentions = cord_array_create(cord_pool_create(), sizeof(cord_channel_mention_t));

                int i = 0;
                json_t *item = NULL;
                json_array_foreach(value, i, item) {
                    cord_channel_mention_t *mention = cord_array_push(msg->mentions);
                    cord_channel_mention_serialize(item, NULL);
                }
                */
            }
        } else if (json_is_object(value)) {
            json_t *object = value;
            cord_error_t error = 0;

            if (string_is_equal(key, "author")) {
                cord_serialize_result_t author = cord_user_serialize(object, allocator);
                if (author.error) {
                    logger_error("%s", cord_error(author.error));
                }
            } else if (string_is_equal(key, "member")) {
                cord_serialize_result_t member = cord_guild_member_serialize(object, &error);
                if (member.error) {
                    logger_error("%s", cord_error(member.error));
                }
            } else if (string_is_equal(key, "activity")) {
                // cord_serialize_result_t activity = cord_message_activity_serialize(object, &error);
                // if (activity.error) {
                    // logger_error("%s", cord_error(activity.error));
                // }
            } else if (string_is_equal(key, "application")) {
                // cord_serialize_result_t application = cord_message_application_serialize(object, &error);
                // if (application.error) {
                    // logger_error("%s", cord_error(application.error));
                // }
            } else if (string_is_equal(key, "message_reference")) {
                // cord_serialize_result_t message_reference = cord_message_reference_serialize(object, &error);
                // if (message_reference.error) {
                    // logger_error("%s", cord_error(message_reference.error));
                // }
            } else if (string_is_equal(key, "referenced_message")) {
                // cord_serialize_result_t referenced_message = cord_message_serialize(object, allocator);
                // if (referenced_message.error) {
                    // logger_error("%s", cord_error(referenced_message.error));
                // }
            }
        }
	}
    return serialized(message, CORD_OK);
}

int cord_guild_init(cord_guild_t *g, cord_bump_t *allocator) {
    // TODO: Implement
    (void)g;
    return CORD_OK;
}

int cord_guild_serialize(cord_guild_t *g, json_t *data) {
    // TODO: Implement
    (void)g;
    (void)data;
    return CORD_OK;
}
