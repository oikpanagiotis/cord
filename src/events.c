#include "events.h"
#include "discord.h"
#include "util.h"
#include "types.h"
#include "log.h"
#include "error.h"

#include <assert.h>

// (string, event_handler) dictionary to dispatch receiving events
static receiving_event receiving_events[] = {
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

receiving_event *get_all_receicing_events(void) {
    return receiving_events;
}

bool event_has_handler(receiving_event *event) {
	if (event) {
		if (event->handler) {
			return true;
		}
	}
	return false;
}

receiving_event *get_receiving_event(receiving_event *events, char *key) {
	receiving_event *iter = events;
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

	cord_err err;
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
