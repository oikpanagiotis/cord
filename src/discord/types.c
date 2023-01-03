#include "types.h"
#include "../core/strings.h"
#include "../core/log.h"
#include "../core/errors.h"
#include "../core/util.h"
#include "../core/array.h"
#include "../core/typedefs.h"

#include <jansson.h>
#include <assert.h>


static cord_serialize_result_t serialized_result(void *obj, cord_error_t code) {
    return (cord_serialize_result_t){obj, code};
}

cord_object_field_t cord_object_field_from_string(cord_bump_t *allocator, const char *cstring) {
    cord_object_field_t field = {0};
    field.is_optional = false;
    field.is_valid = true;
    field.type = CORD_OBJECT_FIELD_TYPE_STRING;
    assert(cstring && "cstring must not be null to read it into a cord object");
    const size_t string_size = strlen(cstring);
    cord_strbuf_t *builder = cord_strbuf_create_with_allocator(allocator);
    cord_strbuf_append(builder, cstr(cstring));
    field._string = builder;
    return field;
}

cord_object_field_t cord_object_field_from_number(cord_bump_t *allocator, i64 value) {
    cord_object_field_t field = {0};
    field.is_optional = false;
    field.is_valid = true;
    field.type = CORD_OBJECT_FIELD_TYPE_NUMBER;
    field._integer = value;
    return field;
}

cord_object_field_t cord_object_field_from_bool(cord_bump_t *allocator, bool value) {
    cord_object_field_t field = {0};
    field.is_optional = false;
    field.is_valid = true;
    field.type = CORD_OBJECT_FIELD_TYPE_BOOL;
    field._bool = value;
    return field;
}

static void init_object_field(cord_object_field_t *field) {
    field->is_optional = false;
    field->is_valid = false;
    field->_bool = false;
    field->type = CORD_OBJECT_FIELD_TYPE_STRING;
}

void cord_user_init(cord_user_t *user, cord_bump_t *allocator) {
    init_object_field(&user->id);
    init_object_field(&user->username);
    init_object_field(&user->discriminator);
    init_object_field(&user->avatar);
    init_object_field(&user->bot);
    init_object_field(&user->system_);
    init_object_field(&user->mfa_enabled);
    init_object_field(&user->locale);
    init_object_field(&user->verified);
    init_object_field(&user->email);
    init_object_field(&user->flags);
    init_object_field(&user->premium_type);
    init_object_field(&user->public_flags);

    assert(allocator && "Allocator passed to user init must not be null");
    user->allocator = allocator;
}

static void user_read_boolean_json_field(cord_user_t *author, const char *key, bool value) {
    cord_object_field_t field = cord_object_field_from_bool(author->allocator, value);
    map_property(author, bot, "bot", key, field);
    map_property(author, system_, "system", key, field);
    map_property(author, mfa_enabled, "mfa_enabled", key, field);
    map_property(author, verified, "varified", key, field);
}

static void user_read_string_json_field(cord_user_t *author, string_ref key, char *value) {
    cord_object_field_t field = cord_object_field_from_string(author->allocator, value);
    // map_property(author, id, "id", key, value);
    // map_property(author, username, "username", key, value);
    map_property(author, discriminator, "discriminator", key, field);
    map_property(author, avatar, "avatar", key, field);
    map_property(author, locale, "locale", key, field);
    map_property(author, email, "email", key, field);
}

static void user_read_integer_json_field(cord_user_t *author, string_ref key, i64 value) {
    cord_object_field_t field = cord_object_field_from_number(author->allocator, value);
    map_property(author, flags, "flags", key, field);
    map_property(author, premium_type, "premium_type", key, field);
    map_property(author, public_flags, "public_flags", key, field);
}

cord_serialize_result_t cord_user_serialize(json_t *json_user, cord_bump_t *allocator) {
    cord_error_t error = 0;
    cord_user_t *author = balloc(allocator, sizeof(cord_user_t));
    if (!author) {
        return serialized_result(author, CORD_ERR_MALLOC);
    }

    cord_user_init(author, allocator);
    const char *key = NULL;
    json_t *value = NULL;   

    json_object_foreach(json_user, key, value) {
        if (json_is_string(value)) {
            char *value_copy = strdup(json_string_value(value));
            user_read_string_json_field(author, key, value_copy);
        } else if (json_is_boolean(value)) {
            bool value_copy = json_boolean_value(value);
            user_read_boolean_json_field(author, key, value_copy);
        } else if (json_is_integer(value)) {
            i64 value_copy = (i64)json_integer_value(value);
            user_read_integer_json_field(author, key, value_copy);
        }
    }

    return serialized_result(author, CORD_OK);
}

void cord_guild_member_init(cord_guild_member_t *member, cord_bump_t *allocator) {
    cord_user_init(member->user, allocator);
    init_object_field(&member->nick);
    init_object_field(&member->joined_at);
    init_object_field(&member->premium_since);
    init_object_field(&member->deaf);
    init_object_field(&member->mute);
    init_object_field(&member->pending);
    member->_roles_count = 0;

    assert(allocator && "Allocator for cord_guild_member_t can not be null");
    member->allocator = allocator;
}

static void guild_member_json_boolean_mappings(cord_guild_member_t *member,
                                               string_ref key,
                                               bool value) {
    cord_object_field_t field = cord_object_field_from_bool(member->allocator, value);
    map_property(member, deaf, "deaf", key, field);
    map_property(member, mute, "mute", key, field);
    map_property(member, pending, "pending", key, field);
}

static void guild_member_string_mappings(cord_guild_member_t *member, const char *key, char *value) {
    cord_object_field_t field = cord_object_field_from_string(member->allocator, value);
    map_property(member, nick, "nick", key, field);
    map_property(member, joined_at, "joined_at", key, field);
    map_property(member, premium_since, "premium_since", key, field);
}

cord_serialize_result_t cord_guild_member_serialize(json_t *data, cord_bump_t *allocator) {
    cord_guild_member_t *member = balloc(allocator, (sizeof(cord_guild_member_t)));
    if (!member) {
        return serialized_result(member, CORD_ERR_MALLOC);
    }

    cord_guild_member_init(member, member->allocator);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            char *value_copy = strdup(json_string_value(value));
            guild_member_string_mappings(member, key, value_copy);
        } else if (json_is_boolean(value)) {
            bool value_copy = json_boolean_value(value);
            guild_member_json_boolean_mappings(member, key, value_copy);
        } else if (json_is_array(value)) {
            int i = 0;
            json_t *slot = NULL;
            if (string_is_equal(key, "roles")) {
                json_array_foreach(value, i, slot) {
                    // TODO: FIX AFTER REFACTOR
                    // int err = cord_role_serialize(member->roles[i], slot);
                    // if (err != CORD_OK) {
                        // logger_error("Failed to serialize cord role");
                        // return NULL;
                    // }
                    // member->_roles_count++;
                }
            }

        } else if (json_is_object(value)) {
            cord_user_t *user = NULL;
            (void)user;
            if (string_is_equal(key, "user")) {
                // cord_user_t *usr = cord_user_serialize(user, value);
                // if (!usr) log_err; store_status; return NULL;
                // TODO: FIX AFTER REFACTOR
                // if (cord_user_serialize(user, value) < 0) {
                    // log_warning("%s", cord_error(ERR_USER_SERIALIZATION));
                    // store_status(err, ERR_USER_SERIALIZATION);
                    // return NULL;
                // }
            }
        }
    }

    return serialized_result(member, CORD_OK);
}

void cord_role_init(cord_role_t *role) {
    
    role->id = cord_strbuf_undefined;
    role->name = cord_strbuf_undefined;
    role->color = 0;
    role->hoist = false;
    role->position = 0;
    role->permissions = cord_strbuf_undefined;
    role->managed = false;
    role->mentionable = false;
}

cord_role_t *cord_role_serialize(json_t *data, cord_error_t *err) {
    cord_role_t *role = malloc(sizeof(cord_role_t));
    if (!role) {
        return serialized_result(err, CORD_ERR_MALLOC);
    }
    
    cord_role_init(role);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            cord_strbuf_t value_copy = cord_strbuf_from_cstring(json_string_value(value));
            map_property(role, id, "id", key, value_copy);
            map_property(role, name, "name", key, value_copy);
            map_property(role, permissions, "permissions", key, value_copy);
        } else if (json_is_boolean(value)) {
            bool val = json_boolean_value(value);
            map_property(role, hoist, "hoist", key, val);
            map_property(role, managed, "managed", key, val);
            map_property(role, mentionable, "mentionable", key, val);
        } else if (json_is_integer(value)) {
            int val = (int)json_integer_value(value);
            map_property(role, color, "color", key, val);
            map_property(role, position, "position", key, val);
        }
    }
    return role;
}

void cord_channel_mention_init(cord_channel_mention_t *mention) {
    mention->id = NULL;
    mention->guild_id = NULL;
    mention->type = 0;
    mention->name = NULL;
}

cord_channel_mention_t *cord_channel_mention_serialize(json_t *data, cord_error_t *err) {
    cord_channel_mention_t *mention = malloc(sizeof(cord_channel_mention_t));
    if (!mention) {
        return serialized_result(err, CORD_ERR_MALLOC);
    }

    cord_channel_mention_init(mention);
    return mention;
}

void cord_attachment_init(cord_attachment_t *at) {
}

cord_attachment_t *cord_attachment_serialize(json_t *data, cord_error_t *err) {
    cord_attachment_t *at = malloc(sizeof(cord_attachment_t));
    if (!at) {
        return serialized_result(err, CORD_ERR_MALLOC);
    }

    cord_attachment_init(at);
    return at;
}

void cord_embed_footer_init(cord_embed_footer_t *ft) {
}

cord_embed_footer_t *cord_embed_footer_serialize(json_t *data, cord_error_t *err) {
    cord_embed_footer_t *ft = malloc(sizeof(cord_embed_footer_t));
    if (!ft) {
        return serialized_result(err, CORD_ERR_MALLOC);
    }

    cord_embed_footer_init(ft);

    return ft;
}

void cord_embed_image_init(cord_embed_image_t *img) {
}

cord_embed_image_t *cord_embed_image_serialize(json_t *data, cord_error_t *err) {
    cord_embed_image_t *img = malloc(sizeof(cord_embed_image_t));
    if (!img) {
        return serialized_result(err, CORD_ERR_MALLOC);
    }

    cord_embed_image_init(img);
    return img;
}

void cord_embed_thumbnail_init(cord_embed_thumbnail_t *tn) {
}

cord_embed_thumbnail_t *cord_embed_thumbnail_serialize(json_t *data, cord_error_t *err) {
    cord_embed_thumbnail_t *tn = malloc(sizeof(cord_embed_thumbnail_t));
    if (!tn) {
        return serialized_result(err, CORD_ERR_MALLOC);
    }

    cord_embed_thumbnail_init(tn);
    return tn;
}

void cord_embed_video_init(cord_embed_video_t *evid) {
}

cord_embed_video_t *cord_embed_video_serialize(json_t *data, cord_error_t *err) {
    cord_embed_video_t *evid = malloc(sizeof(cord_embed_video_t));
    if (!evid) {
        return serialized_result(err, CORD_ERR_MALLOC);
    }

    cord_embed_video_init(evid);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds value_copy = sdsnew(json_string_value(value));
            map_property(evid, url, "url", key, value_copy);
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
        return serialized_result(err, CORD_ERR_MALLOC);
    }

    cord_embed_provider_init(epr);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds value_copy = sdsnew(json_string_value(value));
            map_property(epr, name, "name", key, value_copy);
            map_property(epr, url, "url", key, value_copy);
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
        return serialized_result(err, CORD_ERR_MALLOC);
    }
    
    cord_embed_author_init(eauth);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds value_copy = sdsnew(json_string_value(value));
            map_property(eauth, name, "name", key, value_copy);
            map_property(eauth, url, "url", key, value_copy);
            map_property(eauth, icon_url, "icon_url", key, value_copy);
            map_property(eauth, proxy_icon_url, "proxy_icon_url", key, value_copy);
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
        return serialized_result(err, CORD_ERR_MALLOC);
    }

    cord_embed_field_init(efield);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds value_copy = sdsnew(json_string_value(value));
            map_property(efield, name, "name", key, value_copy);
            map_property(efield, value, "value", key, value_copy);
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
        return serialized_result(err, CORD_ERR_MALLOC);
    }

    cord_emoji_init(emj);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds value_copy = sdsnew(json_string_value(value));
            map_property(emj, id, "id", key, value_copy);
            map_property(emj, name, "name", key, value_copy);
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
        return serialized_result(err, CORD_ERR_MALLOC);
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
                    return serialized_result(err, error);
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
        return serialized_result(err, CORD_ERR_MALLOC);
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

void cord_message_application_init(cord_message_application_t *app) {
    app->id = NULL;
    app->cover_image = NULL;
    app->description = NULL;
    app->icon = NULL;
    app->name = NULL;
}

cord_message_application_t *cord_message_application_serialize(json_t *data, cord_error_t *err) {
    cord_message_application_t *app = malloc(sizeof(cord_message_application_t));
    if (!app) {
        return serialized_result(err, CORD_ERR_MALLOC);
    }

    cord_message_application_init(app);
    
    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds copy_val = sdsnew(json_string_value(value));
            map_property(app, id, "id", key, copy_val);
            map_property(app, cover_image, "cover_image", key, copy_val);
            map_property(app, description, "description", key, copy_val);
            map_property(app, icon, "icon", key, copy_val);
            map_property(app, name, "name", key, copy_val);
        }
    }

    return app;
}

int cord_embed_init(cord_embed_t *emb) {
    // TODO: Implement
    (void)emb;
    return CORD_OK;
}

int cord_embed_serialize(cord_embed_t *emb, json_t *data) {
    // TODO: Implement
    (void)emb;
    (void)data;
    return CORD_OK;
}

void cord_message_reference_init(cord_message_reference_t *mr) {
    mr->message_id = NULL;
    mr->channel_id = NULL;
    mr->guild_id = NULL;
}

cord_message_reference_t *cord_message_reference_serialize(json_t *data, cord_error_t *err) {
    cord_message_reference_t *mr = malloc(sizeof(cord_message_reference_t));
    if (!mr) {
        return serialized_result(err, CORD_ERR_MALLOC);
    }

    cord_message_reference_init(mr);

    string_ref key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds copy_val = sdsnew(json_string_value(value));
            map_property(mr, message_id, "message_id", key, copy_val);
            map_property(mr, channel_id, "channel_id", key, copy_val);
            map_property(mr, guild_id, "guild_id", key, copy_val);
        }
    }
    return mr;
}

void cord_message_sticker_init(cord_message_sticker_t *ms) {
    ms->id = NULL;
    ms->pack_id = NULL;
    ms->name = NULL;
    ms->description = NULL;
    ms->tags = NULL;
    ms->asset = NULL;
    ms->preview_asset = NULL;
    ms->format_type = 0; // https://discord.com/developers/docs/resources/channel#message-object-message-sticker-format-types
}

cord_message_sticker_t *cord_message_sticker_serialize(json_t *data, cord_error_t *err) {
    cord_message_sticker_t *ms = malloc(sizeof(cord_message_sticker_t));
    if (!ms) {
        return serialized_result(err, CORD_ERR_MALLOC);
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

void cord_message_init(cord_message_t *msg) {
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
}

static void message_json_string_mappings(cord_message_t *message, string_ref key, sds value) {
    map_property(message, id, "id", key, value);
    map_property(message, channel_id, "channel_id", key, value);
    map_property(message, guild_id, "guild_id", key, value);
    map_property(message, content, "content", key, value);
    map_property(message, timestamp, "timestamp", key, value);
    map_property(message, edited_timestamp, "edited_timestamp", key, value);
    map_property(message, nonce, "nonce", key, value);
    map_property(message, webhook_id, "webhook_id", key, value);
}

static void message_json_bool_mappings(cord_message_t *message, string_ref key, bool value) {
    map_property(message, tts, "tts", key, value);
    map_property(message, mention_everyone, "mention_everyone", key, value);
    map_property(message, pinned, "pinned", key, value);
}

static void message_json_integer_mappings(cord_message_t *message, string_ref key, i64 value) {
    map_property(message, type, "type", key, value);
	map_property(message, flags, "flags", key, value);
}

cord_serialize_result_t cord_message_serialize(json_t *data, cord_bump_t *allocator) {
    cord_message_t *message = balloc(allocator, sizeof(cord_message_t));
    if (!message) {
		logger_error("Failed to allocate discord message");
        return serialized_result(message, CORD_ERR_MALLOC);
	}

    cord_message_init(message, allocator);

	string_ref key = NULL;
	json_t *value = NULL;
	json_object_foreach(data, key, value) {
		if (json_is_string(value)) {
			char *value_copy = strdup((char *)json_string_value(value));
            message_json_string_mappings(message, key, value_copy);
		} else if (json_is_boolean(value)) {
			bool value_copy = json_boolean_value(value);
            message_json_bool_mappings(message, key, value_copy);
        } else if (json_is_integer(value)) {
			i64 value_copy = (i64)json_integer_value(value);
            message_json_integer_mappings(message, key, value_copy);
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
                cord_user_t *author = cord_user_serialize(object, &error);
                if (!author) {
                    logger_error("%s", cord_error(error));
                }
            } else if (string_is_equal(key, "member")) {
                cord_guild_member_t *member = cord_guild_member_serialize(object, &error);
                if (!member) {
                    logger_error("%s", cord_error(error));
                }
            } else if (string_is_equal(key, "activity")) {
                cord_message_activity_t *activity = cord_message_activity_serialize(object, &error);
                if (!activity) {
                    logger_error("%s", cord_error(error));
                }
            } else if (string_is_equal(key, "application")) {
                cord_message_application_t *application = cord_message_application_serialize(object, &error);
                if (!application) {
                    logger_error("%s", cord_error(error));
                }
            } else if (string_is_equal(key, "message_reference")) {
                cord_message_reference_t *message_reference = cord_message_reference_serialize(object, &error);
                if (!message_reference) {
                    logger_error("%s", cord_error(error));
                }
            } else if (string_is_equal(key, "referenced_message")) {
                cord_message_t *referenced_message = cord_message_serialize(object, &error);
                if (!referenced_message) {
                    logger_error("%s", cord_error(error));
                }
            }
        }
	}
    return serialized_result(message, CORD_OK);
}

int cord_guild_init(cord_guild_t *g) {
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
