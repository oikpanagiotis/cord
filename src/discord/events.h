#ifndef EVENTS_H
#define EVENTS_H

#include <jansson.h>
#include "client.h"

typedef void (*event_handler)(cord_client_t *ctx, json_t *obj, char *event);

#define MAX_EVENT_NAME_LEN 32
typedef struct cord_gateway_event_t {
	char name[MAX_EVENT_NAME_LEN];
	event_handler handler;
} cord_gateway_event_t;

cord_gateway_event_t *get_all_gateway_events(void);

bool event_has_handler(cord_gateway_event_t *event);
void on_message_create(cord_client_t *ctx, json_t *data, char *event);
cord_gateway_event_t *get_gateway_event(cord_gateway_event_t *events, char *key);

#endif