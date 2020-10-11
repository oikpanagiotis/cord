#ifndef CORD_H
#define CORD_H

#include "discord.h"

typedef struct cord_t cord_t;

typedef struct cord_user_t {

} cord_user_t;

typedef struct cord_message_t {

} cord_message_t;

typedef struct cord_guild_t {

} cord_guild_t;

typedef void (*on_msg_cb)(discord_t *c, discord_message_t *msg);
typedef struct cord_t {
	// should be the first field in the struct	
	discord_t *disc;
	on_msg_cb msg_cb;
} cord_t;


cord_t *cord_create(void);
void cord_connect(cord_t *c, const char *url);
void cord_destroy(cord_t *c);

void cord_on_message(cord_t *c, on_msg_cb func);

#endif

