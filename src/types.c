#include "types.h"
#include "log.h"
#include "error.h"

#include <jansson.h>

#define map_property(obj, prop, prop_str, key, val) \
	do { \
		if (string_is_equal(key, prop_str)) { \
			obj->prop = val; \
		} \
} while (0)


int cord_user_init(cord_user_t *user) {
    user = malloc(sizeof(cord_user_t));
    if (!user) {
        return ERR_MALLOC;
    }

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
    return ERR_NONE;
}

int cord_user_serialize(cord_user_t *author, json_t *data) {
    int err = discord_user_init(author);
    if (err != ERR_NONE) {
        return err;
    }

    const char *key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds value_copy = sdsnew(json_string_value(value));
            map_property(author, id, "id", key, value_copy);
            map_property(author, username, "username", key, value_copy);
            map_property(author, discriminator, "discriminator", key, value_copy);
            map_property(author, avatar, "avatar", key, value_copy);
            map_property(author, locale, "locale", key, value_copy);
            map_property(author, email, "email", key, value_copy);
        } else if (json_is_boolean(value)) {
            bool val = json_boolean_value(value);
            map_property(author, bot, "bot", key, val);
            map_property(author, system_, "system", key, val);
            map_property(author, mfa_enabled, "mfa_enabled", key, val);
            map_property(author, verified, "varified", key, val);
        } else if (json_is_integer(value)) {
            int val = (int)json_integer_value(value);
            map_property(author, flags, "flags", key, val);
            map_property(author, premium_type, "premium_type", key, val);
            map_property(author, public_flags, "public_flags", key, val);
        }
    }
    return ERR_NONE;
}

void cord_user_free(cord_user_t *user) {
    sdsfree(user->id);
    sdsfree(user->username);
    sdsfree(user->discriminator);
    sdsfree(user->avatar);
    sdsfree(user->locale);
    sdsfree(user->email);
}


int cord_guild_member_init(cord_guild_member_t *member) {
    member = malloc(sizeof(cord_guild_member_t));
    if (!member) {
        return ERR_MALLOC;
    }

    member->user = NULL;
    member->nick = NULL;
    member->roles = NULL;
    member->joined_at = NULL;
    member->premium_since = NULL;
    member->deaf = false;
    member->mute = false;
    member->pending = false;
    return ERR_NONE;
}

int cord_guild_member_serialize(cord_guild_member_t *member, json_t *data) {
    int err = discord_guild_member_init(member);
    if (err != ERR_NONE) {
        return err;
    }

    const char *key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds copy_value = sdsnew(json_string_value(value));
            map_property(member, nick, "nick", key, copy_value);
            // TODO: Map roles array
            map_property(member, nick, "joined_at", key, copy_value);
            map_property(member, nick, "premium_since", key, copy_value);
        } else if (json_is_boolean(value)) {
            bool val = json_boolean_value(value);
            map_property(member, deaf, "deaf", key, val);
            map_property(member, mute, "mute", key, val);
            map_property(member, pending, "pending", key, val);
        } else if (json_is_object(value)) {
            cord_user_t *user = NULL;
            if (serialize_user(user, value) < 0) {
                return ERR_USER_SERIALIZATION;
            }
        }
    }

    return ERR_NONE;
}

void cord_guild_member_free(cord_guild_member_t *member) {
    discord_user_free(member->user);
    sdsfree(member->nick);
    sdsfree(member->joined_at);
    sdsfree(member->premium_since);
}


int cord_role_init(cord_role_t *role) {
    role = malloc(sizeof(cord_role_t));
    if (!role) {
        return ERR_MALLOC;
    }

    role->id = NULL;
    role->name = NULL;
    role->color = 0;
    role->hoist = false;
    role->position = 0;
    role->permissions = NULL;
    role->managed = false;
    role->mentionable = false;
    return ERR_NONE;
}

int cord_role_serialize(cord_role_t *role, json_t *data) {
    int err = discord_role_init(role);
    if (err != ERR_NONE) {
        return err;
    }

    const char *key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds value_copy = sdsnew(json_string_value(value));
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
    return ERR_NONE;
}

void cord_role_free(cord_role_t *role) {
    sdsfree(role->id);
    sdsfree(role->name);
    sdsfree(role->permissions);
}


int cord_channel_mention_init(cord_channel_mention_t *mention) {
    mention = malloc(sizeof(cord_channel_mention_t));
    if (!mention) {
        return ERR_MALLOC;
    }

    mention->id = NULL;
    mention->guild_id = NULL;
    mention->type = 0;
    mention->name = NULL;
    return ERR_NONE;
}

int cord_channel_mention_serialize(cord_channel_mention_t *mention, json_t *data) {
    int err = channel_mention_init(mention);
    if (err != ERR_NONE) {
        return err;
    }

    const char *key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds copy_value = sdsnew(json_string_value(value));
            map_property(mention, id, "id", key, copy_value);
            map_property(mention, guild_id, "guild_id", key, copy_value);
            map_property(mention, name, "name", key, copy_value);
        } else if (json_is_integer(value)) {
            int val = (int)json_integer_value(value);
            map_property(mention, type, "type", key, val);
        }
    }
    return ERR_NONE;
}

void cord_channel_mention_free(cord_channel_mention_t *mention) {
    sdsfree(mention->id);
    sdsfree(mention->guild_id);
    sdsfree(mention->name);
}


int cord_attachment_init(cord_attachment_t *at) {
    at = malloc(sizeof(cord_attachment_t));
    if (!at) {
        return ERR_MALLOC;
    }

    at->id = NULL;
    at->filename = NULL;
    at->size = 0;
    at->url = NULL;
    at->proxy_url = NULL;
    at->height = 0;
    at->width = 0;
    return ERR_NONE;
}

int cord_attachment_serialize(cord_attachment_t *at, json_t *data) {
    int err = cord_attachment_init(at);
    if (err != ERR_NONE) {
        return err;
    }

    const char *key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds value_copy = sdsnew(json_string_value(value));
            map_property(at, id, "id", key, value_copy);
            map_property(at, filename, "filename", key, value_copy);
            map_property(at, url, "url", key, value_copy);
            map_property(at, proxy_url, "proxy_url", key, value_copy);
        } else if (json_is_integer(value)) {
            int val = (int)json_integer_value(value);
            map_property(at, size, "size", key, val);
            map_property(at, height, "height", key, val);
            map_property(at, width, "width", key, val);
        }
    }
    return ERR_NONE;
}

void cord_attachment_free(cord_attachment_t *at) {
    sdsfree(at->id);
    sdsfree(at->filename);
    sdsfree(at->url);
    sdsfree(at->proxy_url);
}

int cord_embed_footer_init(cord_embed_footer_t *ft) {
    ft = malloc(sizeof(ft));
    if (!ft) {
        return ERR_MALLOC;
    }

    ft->text = NULL;
    ft->icon_url = NULL;
    ft->proxy_icon_url = NULL;
}

int cord_embed_footer_serialize(cord_embed_footer_t *ft, json_t *data) {
    int err = cord_embed_footer_init(ft);
    if (err != ERR_NONE) {
        return err;
    }

    const char *key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds value_copy = sdsnew(json_string_value(value));
            map_property(ft, text, "text", key, value_copy);
            map_property(ft, icon_url, "icon_url", key, value_copy);
            map_property(ft, proxy_icon_url, "proxy_icon_url", key, value_copy);
        }
    }
    return ERR_NONE;
}

void cord_embed_footer_free(cord_embed_footer_t *ft) {
    sdsfree(ft->text);
    sdsfree(ft->icon_url);
    sdsfree(ft->proxy_icon_url);
}

int cord_embed_image_init(cord_embed_image_t *img) {
    img = malloc(sizeof(cord_embed_image_t));
    if (!img) {
        return ERR_MALLOC;
    }

    img->url = NULL;
    img->proxy_url = NULL;
    img->height = 0;
    img->width = 0;
    return ERR_NONE;
}

int cord_embed_image_serialize(cord_embed_image_t *img, json_t *data) {
    int err = cord_embed_image_init(img);
    if (err != ERR_NONE) {
        return err;
    }

    const char *key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds value_copy = sdsnew(json_string_value(value));
            map_property(img, url, "url", key, value_copy);
            map_property(img, proxy_url, "proxy_url", key, value_copy);
        } else if (json_is_integer(value)) {
            int val = (int)json_integer_value(value);
            map_property(img, height, "height", key, val);
            map_property(img, width, "width", key, val);
        }
    }
    return ERR_NONE;
}

void cord_embed_image_free(cord_embed_image_t *img) {
    sdsfree(img->url);
    sdsfree(img->proxy_url);
}

int cord_embed_thumbnail_init(cord_embed_thumbnail_t *tn) {
    tn = malloc(sizeof(cord_embed_thumbnail_t));
    if (!tn) {
        return ERR_MALLOC;
    }

    tn->url = NULL;
    tn->proxy_url = NULL;
    tn->height = 0;
    tn->width = 0;
    return ERR_NONE;
}

int cord_embed_thumbnail_serialize(cord_embed_thumbnail_t *tn, json_t *data) {
    int err = cord_embed_thumbnail_init(tn);
    if (err != ERR_NONE) {
        return err;
    }

    const char *key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds value_copy = sdsnew(json_string_value(value));
            map_property(tn, url, "url", key, value_copy);
            map_property(tn, proxy_url, "proxy_url", key, value_copy);
        } else if (json_is_integer(value)) {
            int val = (int)json_integer_value(value);
            map_property(tn, height, "height", key, val);
            map_property(tn, width, "width", key, val);
        }
    }
    return ERR_NONE;
}

void cord_embed_thumbnail_free(cord_embed_thumbnail_t *tn) {
    sdsfree(tn->url);
    sdsfree(tn->proxy_url);
}

int cord_embed_video_init(cord_embed_video_t *evid) {
    evid = malloc(sizeof(cord_embed_video_t));
    if (!evid) {
        return ERR_MALLOC;
    }

    evid->url = NULL;
    evid->height = 0;
    evid->width = 0;
    return ERR_NONE;
}

int cord_embed_video_serialize(cord_embed_video_t *evid, json_t *data) {
    int err = cord_embed_video_init(evid);
    if (err != ERR_NONE) {
        return err;
    }

    const char *key = NULL;
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
}

void cord_embed_video_free(cord_embed_video_t *evid) {
    sdsfree(evid->url);
}

int cord_embed_provider_init(cord_embed_provider_t *epr) {
    epr = malloc(sizeof(cord_embed_provider_t));
    if (!epr) {
        return ERR_MALLOC;
    }

    epr->name = NULL;
    epr->url = NULL;
    return ERR_NONE;
}

int cord_embed_provider_serialize(cord_embed_provider_t *epr, json_t *data) {
    int err = cord_embed_provider_init(epr);
    if (err != ERR_NONE) {
        return err;
    }

    const char *key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds value_copy = sdsnew(json_string_value(value));
            map_property(epr, name, "name", key, value_copy);
            map_property(epr, url, "url", key, value_copy);
        }
    }
    return ERR_NONE;
}

void cord_embed_provider_free(cord_embed_provider_t *epr) {
    sdsfree(epr->name);
    sdsfree(epr->url);
}

int cord_embed_author_init(cord_embed_author_t *eauth) {
    eauth = malloc(sizeof(cord_embed_author_t));
    if (!eauth) {
        return ERR_MALLOC;
    }

    eauth->name = NULL;
    eauth->url = NULL;
    eauth->icon_url = NULL;
    eauth->proxy_icon_url = NULL;
    return ERR_NONE;
}

int cord_embed_author_serialize(cord_embed_author_t *eauth, json_t *data) {
    int err = cord_embed_author_init(eauth);
    if (err != ERR_NONE) {
        return err;
    }

    const char *key = NULL;
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
    return ERR_NONE;
}

void cord_embed_author_free(cord_embed_author_t *eauth) {
    sdsfree(eauth->name);
    sdsfree(eauth->url);
    sdsfree(eauth->icon_url);
    sdsfree(eauth->proxy_icon_url);
}

int cord_embed_field_init(cord_embed_field_t *efield) {
    efield = malloc(sizeof(cord_embed_field_t));
    if (!efield) {
        return ERR_MALLOC;
    }

    efield->name = NULL;
    efield->value = NULL;
    efield->inline_ = false;
    return ERR_NONE;
}

int cord_embed_field_serialize(cord_embed_field_t *efield, json_t *data) {
    int err = cord_embed_field_init(efield);
    if (err != ERR_NONE) {
        return err;
    }

    const char *key = NULL;
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
    return ERR_NONE;
}

void cord_embed_field_free(cord_embed_field_t *efield) {
    sdsfree(efield->name);
    sdsfree(efield->value);
}

int cord_emoji_init(cord_emoji_t *emj) {
    emj = malloc(sizeof(cord_emoji_t));
    if (!emj) {
        return ERR_MALLOC;
    }

    emj->id = NULL;
    emj->name = NULL;
    emj->roles = NULL;
    emj->user = NULL;
    emj->require_colons = false;
    emj->managed = false;
    emj->animated = false;
    emj->available = false;
    return ERR_NONE;
}

int cord_emoji_serialize(cord_emoji_t *emj, json_t *data) {
    int err = cord_emoji_init(emj);
    if (err != ERR_NONE) {
        return err;
    }

    const char *key = NULL;
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
            if (string_is_equal(key, "roles")) {
                // TODO: roles array
            } else if (string_is_equal(key, "user")) {
                // Serializing sub object (User)
                cord_user_t *user_obj = NULL;
                int err = cord_user_init(user_obj);
                if (err != ERR_NONE) {
                    return err;
                }
                err = cord_user_serialize(user_obj, obj);
                if (err != ERR_NONE) {
                    return err;
                }

                // In this case user is the name of the struct field
                // and user_obj is the serialized object
                map_property(emj, user, "user", key, user_obj);
            }
        }
    }
    return ERR_NONE;
}

void cord_emoji_free(cord_emoji_t *emj) {
    sdsfree(emj->id);
    sdsfree(emj->name);
    //cord_role_free(roles);
    cord_user_free(emj->user);
}

int cord_reaction_init(cord_reaction_t *react) {
    react = malloc(sizeof(cord_reaction_t));
    if (!react) {
        return ERR_MALLOC;
    }

    react->count = 0;
    react->me = false;
    react->emoji = NULL;
    return ERR_NONE;
}

int cord_reaction_serialize(cord_reaction_t *react, json_t *data) {
    int err = cord_reaction_init(react);
    if (err != ERR_NONE) {
        return err;
    }

    const char *key = NULL;
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
                cord_emoji_t *emoji_obj = NULL;
                int err = cord_emoji_init(emoji_obj);
                if (err != ERR_NONE) {
                    return err;
                }
                err = cord_emoji_serialize(emoji_obj, obj);
                if (err != ERR_NONE) {
                    return err;
                }
                map_property(react, emoji, "emoji", key, emoji_obj);
            }
        }
    }
    return ERR_NONE;
}

int cord_message_activity_init(cord_message_activity_t *ma) {
    ma = malloc(sizeof(cord_message_activity_t));
    if (!ma) {
        return ERR_MALLOC;
    }

    ma->type = 0;
    ma->party_id = NULL;
    return ERR_NONE;
}

int cord_message_activity_serialize(cord_message_activity_t *ma, json_t *data) {
    int err = cord_message_activity_init(ma);
    if (err != ERR_NONE) {
        return err;
    }

    const char *key = NULL;
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
    return ERR_NONE;
}

void cord_cord_message_activity_free(cord_message_activity_t *ma) {
    sdsfree(ma->party_id);
}

void cord_reaction_free(cord_reaction_t *react) {
    cord_emoji_free(react->emoji);
}

int cord_message_application_init(cord_message_application_t *app) {
    app = malloc(sizeof(cord_message_application_t));
    if (!app) {
        return ERR_MALLOC;
    }

    app->id = NULL;
    app->cover_image = NULL;
    app->description = NULL;
    app->icon = NULL;
    app->name = NULL;
    return ERR_NONE;
}

int cord_message_application_serialize(cord_message_application_t *app, json_t *data) {
    int err = cord_message_application_init(app);
    if (err != ERR_NONE) {
        return err;
    }

    const char *key = NULL;
    json_t *value = NULL;

    json_object_foreach(app, key, value) {
        if (json_is_string(value)) {
            sds copy_val = sdsnew(json_string_value(value));
            map_property(app, id, "id", key, copy_val);
            map_property(app, cover_image, "cover_image", key, copy_val);
            map_property(app, description, "description", key, copy_val);
            map_property(app, icon, "icon", key, copy_val);
            map_property(app, name, "name", key, copy_val);
        }
    }

    return ERR_NONE;
}

void cord_message_application_free(cord_message_application_t *app) {
    sdsfree(app->id);
    sdsfree(app->cover_image);
    sdsfree(app->description);
    sdsfree(app->icon);
    sdsfree(app->name);
}

int cord_embed_init(cord_embed_t *emb) {
    // TODO: Implement
}

int cord_embed_serialize(cord_embed_t *emb, json_t *data) {
    // TODO: Implement
}

void cord_embed_free(cord_embed_t *emb) {
    // TODO: Implement
}

int cord_message_reference_init(cord_message_reference_t *mr) {
    mr = malloc(sizeof(cord_message_reference_t));
    if (!mr) {
        return ERR_MALLOC;
    }

    mr->message_id = NULL;
    mr->channel_id = NULL;
    mr->guild_id = NULL;
    return ERR_NONE;
}

int cord_message_reference_serialize(cord_message_reference_t *mr, json_t *data) {
    int err = cord_message_reference_init(mr);
    if (err != ERR_NONE) {
        return err;
    }

    const char *key = NULL;
    json_t *value = NULL;

    json_object_foreach(data, key, value) {
        if (json_is_string(value)) {
            sds copy_val = sdsnew(json_string_value(value));
            map_property(mr, message_id, "message_id", key, copy_val);
            map_property(mr, channel_id, "channel_id", key, copy_val);
            map_property(mr, guild_id, "guild_id", key, copy_val);
        }
    }
    return ERR_NONE;
}

void cord_message_reference_free(cord_message_reference_t *mr) {
    sdsfree(mr->message_id);
    sdsfree(mr->channel_id);
    sdsfree(mr->guild_id);
}


int cord_message_init(cord_message_t *msg) {
    msg = malloc(sizeof(cord_message_t));
    if (!msg) {
        return ERR_MALLOC;
    }

    // map->...
    return ERR_NONE;
}

int cord_message_serialize(cord_message_t *msg, json_t *data) {
   	msg = malloc(sizeof(cord_message_t));
	if (!msg) {
		return ERR_MALLOC;
	}

	const char *key = NULL;
	json_t *value = NULL;

	json_object_foreach(data, key, value) {
		if (json_is_string(value)) {
			sds value_copy = sdsnew(json_string_value(value));
			map_property(msg, id, "id", key, value_copy);
			map_property(msg, channel_id, "channel_id", key, value_copy);
			map_property(msg, guild_id, "guild_id", key, value_copy);
			map_property(msg, content, "content", key, value_copy);
			map_property(msg, timestamp, "timestamp", key, value_copy);
			map_property(msg, edited_timestamp, "edited_timestamp", key, value_copy);
			map_property(msg, webhook_id, "webhook_id", key, value_copy);
		} else if (json_is_boolean(value)) {
			bool val = json_boolean_value(value);
			map_property(msg, tts, "tts", key, val);
			map_property(msg, pinned, "pinned", key, val);
			map_property(msg, mention_everyone, "mention_everyone", key, val);
		} else if (json_is_array(value)) {
			// TODO: Implement
            if (string_is_equal(key, "mentions")) {
                
            } else if (string_is_equal(key, "mention_roles")) {

            } else if (string_is_equal(key, "mention_channels")) {

            } else if (string_is_equal(key, "attachments")) {

            } else if (string_is_equal(key, "embeds")) {

            } else if (string_is_equal(key, "reactions")) {

            } else if (string_is_equal(key, "stickers")) {
		} else if (json_is_integer(value)) {
			json_int_t val = json_integer_value(value);
			int num = (int)val; // long long -> int
			map_property(msg, type, "type", key, num);
			map_property(msg, flags, "flags", key, num);
		} else if (json_is_object(value)) {
            // BUG, BUG!! We need to run foreach here and put these inside the block
            if (string_is_equal(key, "member")) {
                cord_user_t *author = NULL;
                if (serialize_user(author, value) < 0) {
                    log_warning("Failed to serialize author object");
                }
                msg->author = author;
            } else if (string_is_equal(key, "activity")) {
                void *ptr = NULL;
                if (serialize_message_activity(ptr, value) < 0) {
                    log_warning("Failed to serialize message activity object");
                }
            } else if (string_is_equal(key, "message_application")) {
                void *ptr = NULL;
                if (serialize_message_application(ptr, value) < 0) {
                    log_warning("Failed to serialize message application object");
                }
            } else if (string_is_equal(key, "message_reference")) {
                void *ptr = NULL;
                if (serialize_message_application(ptr, value) < 0) {
                    log_warning("Failed to serialize message reference object");
                }
            } else if (string_is_equal(key, "referenced_message")) {
                void *ptr = NULL;
                if (serialize_referenced_message(ptr, value) < 0) {
                    log_warning("Failed to serialize referenced message object");
                }
            }
        }
	}
    return ERR_NONE;
}

void cord_message_free(cord_message_t *msg) {
    // TODO: Implement
}