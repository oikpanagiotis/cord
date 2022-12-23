#include "events.h"
#include "discord.h"
#include "types.h"
#include "core/util.h"
#include "core/log.h"
#include "core/error.h"

#include <assert.h>

/*
*  Dictionary of all the possible events as documented in Discord Gateway API
*/
static cord_gateway_event_t receiving_events[] = {
	{ "CHANNEL_CREATE", NULL },
	{ "CHANNEL_UPDATE", NULL },
	{ "CHANNEL_DELETE", NULL },
	{ "CHANNEL_PINS_UPDATE", NULL },
	{ "GUILD_CREATE", NULL },
	{ "GUILD_UPDATE", NULL },
	{ "GUILD_DELETE", NULL },
	{ "GUILD_BAN_ADD", NULL },
	{ "GUILD_BAN_REMOVE", NULL },
	{ "GUILD_EMOJIS_UPDATE", NULL },
	{ "GUILD_MEMBER_ADD", NULL },
	{ "GUILD_MEMBER_UPDATE", NULL },
	{ "GUILD_MEMBERS_CHUNK", NULL },
	{ "GUILD_ROLE_CREATE", NULL },
	{ "GUILD_ROLE_UPDATE", NULL },
	{ "GUILD_ROLE_DELETE", NULL },
	{ "INVITE_CREATE", NULL },
	{ "INVITE_DELETE", NULL },
	{ "MESSAGE_CREATE", NULL },
	{ "MESSAGE_UPDATE", NULL },
	{ "MESSAGE_DELETE_BULK", NULL },
	{ "MESSAGE_REACTION_ADD", NULL },
	{ "MESSAGE_REACTION_REMOVE", NULL },
	{ "MESSAGE_REACTION_REMOVE_ALL", NULL },
	{ "MESSAGE_REACTION_REMOVE_EMOJI", NULL },
	{ "PRESENCE_UPDATE", NULL },

	{ "", NULL }
};

cord_gateway_event_t *get_all_gateway_events(void) {
    return receiving_events;
}

bool event_has_handler(cord_gateway_event_t *event) {
	if (event) {
		if (event->handler) {
			return true;
		}
	}
	return false;
}

cord_gateway_event_t *get_gateway_event(cord_gateway_event_t *events, char *key) {
	cord_gateway_event_t *iter = events;
	while (!string_is_empty(iter->name)) {
		if (string_is_equal(iter->name, key)) {
			return iter;
		}
		iter++;
	}

	return NULL;
}

void on_message_create(discord_t *ctx, json_t *data, char *event) {
	(void)ctx;
	(void)event;

	cord_error_t err;
	cord_message_t *msg = cord_message_serialize(data, &err);
	if (!msg) {
		log_error("Failed to serialize message: %s", cord_error(err));
		return;
	}

	// Make sure we didn't corrupt the heap
	assert(msg);
	ctx->event_callbacks.on_message(ctx, msg);
	// BUG: Fix initialization function so cord_message_free doesnt crash on NULL
	// cord_message_free(msg);
}
