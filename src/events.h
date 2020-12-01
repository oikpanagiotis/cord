#ifndef EVENTS_H
#define EVENTS_H

#include <jansson.h>
#include "discord.h"

typedef void (*event_handler)(discord_t *ctx, json_t *obj, char *event);

#define MAX_EVENT_NAME_LEN 32
typedef struct receiving_event {
	char name[MAX_EVENT_NAME_LEN];
	event_handler handler;
} receiving_event;

receiving_event *get_all_receicing_events(void);

bool event_has_handler(receiving_event *event);
void on_message_create(discord_t *ctx, json_t *data, char *event);
receiving_event *get_receiving_event(receiving_event *events, char *key);

void on_message_create(discord_t *ctx, json_t *data, char *event);

#endif