#include "events.h"
#include "client.h"
#include "types.h"
#include "../core/util.h"
#include "../core/log.h"
#include "../core/errors.h"

#include <assert.h>

/*
*  Dictionary of all the possible events as documented in Discord Gateway API
*/
static cord_gateway_event_t gateway_events[] = {
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
    return gateway_events;
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

static void init_message_lifecycle_allocator(cord_client_t *client) {
	cord_bump_t *allocator = cord_bump_create_with_size(KB(2));
	client->message_allocator = allocator;
}

static void free_message_lifecycle_allocator(cord_client_t *client) {
	cord_bump_destroy(client->message_allocator);
	client->message_allocator = NULL;
}

void on_message_create(cord_client_t *client, json_t *data, char *event) {
	logger_debug("Got event: %s", event);
	// init_message_lifecycle_allocator(client);

	

	cord_serialize_result_t message = cord_message_serialize(data, client->message_allocator);
	// Make sure we didn't corrupt the heap
	assert(message.obj);

	if (message.error) {
		logger_error("Failed to serialize message: %s", cord_error(message.error));
		return;
	}

	client->event_callbacks.on_message_cb(message.obj);
	// free_message_lifecycle_allocator(client);
}
