#ifndef CORD_H
#define CORD_H

#include "../core/log.h"
#include "../core/memory.h"
#include "../discord/client.h"
#include "../discord/events.h"
#include "../discord/types.h"
#include "../http/http.h"


typedef struct cord_t cord_t;

typedef struct cord_t {
	cord_client_t *gateway_client;
	cord_http_client_t *http_client;
	cord_logger_t *logger;
	on_msg_cb msg_cb;
	void *user_data;
} cord_t;


cord_t *cord_create(void);
void cord_connect(cord_t *cord, const char *url);
void cord_destroy(cord_t *cord);

void cord_on_message(cord_t *c, on_msg_cb func);

// void cord_on_event(Context, EventType, FunctionCallback)
// void cord_on_event(cord, EVENT_MESSAGE_RECEIVE, on_message);

#endif

