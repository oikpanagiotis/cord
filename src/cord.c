#include "cord.h"
#include "discord.h"
#include "error.h"
#include <jansson.h>
#include <uwsc/uwsc.h>

cord_t *cord_create(void) {
	cord_t *c = malloc(sizeof(cord_t));
	c->client = discord_create();	
	return c;
}

void cord_connect(cord_t *c, const char *url) {
	discord_connect(c->client, url);
}

void cord_destroy(cord_t *c) {
	if (c) {
		if (c->client) {
			discord_destroy(c->client);
		}
		free(c);
	}
}


void cord_on_message(cord_t *c, on_msg_cb func) {
	c->client->event_callbacks.on_message = func;
}