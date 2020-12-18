#include "types.h"
#include "log.h"

#include <jansson.h>

#define map_property(obj, prop, prop_str, key, val) \
	do { \
		if (string_is_equal(key, prop_str)) { \
			obj->prop = val; \
		} \
} while (0)

int serialize_user(discord_author_t *author, json_t *data) {
    int rc = -1;

    author = malloc(sizeof(discord_author_t));
    if (!author) {
        log_error("Failed to allocate author");
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
    return 0;
}

int serialize_message(discord_message_t *msg, json_t *data) {
    int rc = -1;

   	msg = malloc(sizeof(discord_message_t));
	if (!msg) {
		log_error("Failed to allocate discord message");
		return rc;
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
    return 0;
}