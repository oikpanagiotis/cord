#ifndef CLIENT_H
#define CLIENT_H

#include "../core/memory.h"
#include "../http/http.h"
#include "types.h"

#include <ev.h>
#include <jansson.h>
#include <stdbool.h>
#include <stdlib.h>
#include <uwsc/uwsc.h>

#define PAYLOAD_KEY_OPCODE "op"
#define PAYLOAD_KEY_DATA "d"
#define PAYLOAD_KEY_SEQUENCE "s"
#define PAYLOAD_KEY_EVENT "t"

#define OP_DISPATCH 0
#define OP_HEARTBEAT 1
#define OP_IDENTIFY 2
#define OP_PRESENCE_UPDATE 3
#define OP_VOICE_STATUS_UPDATE 4
#define OP_RESUME 6
#define OP_RECONNECT 7
#define OP_REQUEST_GUILD_MEMBERS 8
#define OP_INVALID_SESSION 9
#define OP_HELLO 10
#define OP_HEARTBEAT_ACK 11

enum {
    ACTION_NONE,
    ACTION_SEND_MESSAGE,

    ACTION_COUNT
};

typedef struct cord_t cord_t;

typedef struct cord_gateway_event_callbacks_t {
    void (*on_message_cb)(cord_t *ctx, cord_message_t *message);
} cord_gateway_event_callbacks_t;

typedef struct identity_info_t {
    char *token;
    char *os;
    char *library;
    char *device;
} identity_info_t;

typedef struct discord_event_t {
    int type;
    char *token;
    int token_length;
} discord_event_t;

typedef struct cord_client_t {
    struct uwsc_client *ws_client;
    struct ev_loop *loop;

    struct ev_timer *health_report_scheduler;

    struct ev_timer *hb_watcher;
    struct ev_signal *sigint_watcher;
    struct ev_check *reconnect_watcher;

    // Architect so we can use memory pool
    // Implement these as bump allocators
    cord_bump_t *persistent_allocator;
    cord_bump_t *message_allocator;
    cord_bump_t *temporary_allocator;
    cord_bump_t *message_lifecycle_allocator;

    bool heartbeat_acknowledged;
    bool must_reconnect;

    i32 hb_interval;
    i32 sequence;
    bool sent_initial_heartbeat;

    identity_info_t identity;
    cord_http_client_t *http;

    cord_gateway_event_callbacks_t event_callbacks;

    void *user_data;
} cord_client_t;

cord_client_t *cord_client_create(void);

i32 cord_client_connect(cord_client_t *client, const char *url);
void discord_destroy(cord_client_t *client);

void cord_client_send_message(cord_client_t *client, cord_message_t *message);

#endif
