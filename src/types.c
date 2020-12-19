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


int discord_user_init(discord_user_t *user) {
    user = malloc(sizeof(discord_user_t));
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

int serialize_user(discord_user_t *author, json_t *data) {
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

void discord_user_free(discord_user_t *user) {
    sdsfree(user->id);
    sdsfree(user->username);
    sdsfree(user->discriminator);
    sdsfree(user->avatar);
    sdsfree(user->locale);
    sdsfree(user->email);
}


int discord_guild_member_init(discord_guild_member_t *member) {
    member = malloc(sizeof(discord_guild_member_t));
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

int serialize_guild_member(discord_guild_member_t *member, json_t *data) {
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
            discord_user_t *user = NULL;
            if (serialize_user(user, value) < 0) {
                return ERR_USER_SERIALIZATION;
            }
        }
    }

    return ERR_NONE;
}

void discord_guild_member_free(discord_guild_member_t *member) {
    discord_user_free(member->user);
    sdsfree(member->nick);
    sdsfree(member->joined_at);
    sdsfree(member->premium_since);
}


int discord_role_init(discord_role_t *role) {
    role = malloc(sizeof(discord_role_t));
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

int serialize_discord_role(discord_role_t *role, json_t *data) {
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

void discord_role_free(discord_role_t *role) {
    sdsfree(role->id);
    sdsfree(role->name);
    sdsfree(role->permissions);
}


int channel_mention_init(channel_mention_t *mention) {
    mention = malloc(sizeof(channel_mention_t));
    if (!mention) {
        return ERR_MALLOC;
    }

    mention->id = NULL;
    mention->guild_id = NULL;
    mention->type = 0;
    mention->name = NULL;
    return ERR_NONE:
}

int channel_mention_serialize(channel_mention_t *mention, json_t *data) {
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

void channel_mention_free(channel_mention_t *mention) {
    sdsfree(mention->id);
    sdsfree(mention->guild_id);
    sdsfree(mention->name);
}



int discord_message_init(discord_message_t *msg) {
    msg = malloc(sizeof(discord_message_t));
    if (!msg) {
        return ERR_MALLOC;
    }

    // map->...
    return ERR_NONE;
}

int serialize_message(discord_message_t *msg, json_t *data) {
   	msg = malloc(sizeof(discord_message_t));
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
                discord_user_t *author = NULL;
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