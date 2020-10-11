#include "cord.h"
#include "discord.h"
#include <uwsc/uwsc.h>

cord_t *cord_create(void) {
	cord_t *c = malloc(sizeof(cord_t));
	c->disc = discord_create();	
	return c;
}

void cord_connect(cord_t *c, const char *url) {
	discord_connect(c->disc, url);
}

void cord_destroy(cord_t *c) {
	if (c) {
		if (c->disc) {
			discord_destroy(c->disc);
		}
		free(c);
	}
}


void cord_on_message(cord_t *c, on_msg_cb func) {
	c->disc->on_message_callback = func;
}
