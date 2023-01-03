#include "cord.h"
#include "../core/log.h"
#include "../core/errors.h"
#include "../discord/client.h"

#include <jansson.h>
#include <uwsc/uwsc.h>


cord_t *cord_create(void) {
	// TODO: Move this to a global context initialization routine
	global_logger_init();

	cord_t *c = malloc(sizeof(cord_t));
	c->gateway_client = discord_create();
	return c;
}

void cord_connect(cord_t *c, const char *url) {
	discord_connect(c->gateway_client, url);
}

void cord_destroy(cord_t *c) {
	if (c) {
		if (c->gateway_client) {
			discord_destroy(c->gateway_client);
		}
		free(c);
	}
}


void cord_on_message(cord_t *c, on_msg_cb func) {
	c->gateway_client->event_callbacks.on_message = func;
}