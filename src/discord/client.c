#include "client.h"
#include "../cord/cord.h"
#include "../core/log.h"
#include "../core/typedefs.h"
#include "client.h"
#include "events.h"
#include "serialization.h"

#include <assert.h>
#include <ev.h>
#include <jansson.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uwsc/config.h>
#include <uwsc/uwsc.h>

const i32 GUILDS = (1 << 0);
const i32 GUILD_MODERATION = (1 << 2);
const i32 GUILD_EMOJIS_AND_STICKERS = (1 << 3);
const i32 GUILD_INTEGRATIONS = (1 << 4);
const i32 GUILD_WEBHOOKS = (1 << 5);
const i32 GUILD_INVITES = (1 << 6);
const i32 GUILD_MESSAGES = (1 << 9);
const i32 GUILD_MESSAGE_REACTIONS = (1 << 10);
const i32 GUILD_MESSAGE_TYPING = (1 << 11);

const i32 DIRECT_MESSAGES = (1 << 12);
const i32 DIRECT_MESSAGE_REACTIONS = (1 << 13);
const i32 DIRECT_MESSAGE_TYPING = (1 << 14);
const i32 MESSAGE_CONTENT = (1 << 15);

const i32 GUILD_SCHEDULE_EVENTS = (1 << 16);
const i32 AUTO_MODERATION_CONFIGURATION = (1 << 20);
const i32 AUTO_MODERATION_EXECUTION = (1 << 21);
const i32 GUILD_MESSAGE_POLLS = (1 << 24);
const i32 DIRECT_MESSAGE_POLLS = (1 << 25);

static void load_identity_info(identity_info_t *identity) {
    identity->token = getenv("CORD_APPLICATION_TOKEN");
    if (!identity->token) {
        logger_error(
            "Bot token not found. Please set CORD_APPLICATION_TOKEN envar");
        exit(1);
    }

    identity->os = (char *)OS;
    identity->device = LIBRARY_NAME;
    identity->library = LIBRARY_NAME;
}

/*
 * Assumes that sequence is initialized to -1
 */
static bool is_valid_sequence(i32 s) {
    return s >= 0;
}

static f64 heartbeat_to_double(i32 interval) {
    i32 decimal = interval / 1000;
    i32 floating_ms = interval % 1000;

    f64 decimal_d = (f64)decimal;
    f64 floating_d = (f64)floating_ms * 0.001;
    return decimal_d + floating_d;
}

typedef struct buf {
    char *data;
    size_t length;
} buf;

static bool is_valid_payload(buf payload) {
    return payload.data && payload.length > 0;
}

static buf buf_from_json(json_t *json) {
    char *json_string = json_dumps(json, 0);
    if (!json_string) {
        return (buf){"", 0};
    }

    size_t length = strlen(json_string);
    return (buf){json_string, length};
}

static void buf_destroy(buf payload) {
    if (payload.data) {
        free(payload.data);
        payload.data = NULL;
    }
}

static void send_buf(cord_client_t *client, buf buffer) {
    client->ws_client->send(
        client->ws_client, buffer.data, buffer.length, UWSC_OP_TEXT);
}

static json_t *heartbeat_json_create(i32 sequence) {
    json_t *heartbeat_json = json_object();
    if (!heartbeat_json) {
        return NULL;
    }
    json_object_set_new(heartbeat_json, PAYLOAD_KEY_OPCODE, json_integer(1));

    // Send sequence number unless we havent received one
    json_t *sequence_json =
        is_valid_sequence(sequence) ? json_integer(sequence) : json_null();

    json_object_set_new(heartbeat_json, PAYLOAD_KEY_DATA, sequence_json);
    return heartbeat_json;
}

static void send_heartbeat(cord_client_t *client) {
    json_t *heartbeat_json = heartbeat_json_create(client->sequence);
    if (!heartbeat_json) {
        logger_error("Failed to create heartbeat json object");
        return;
    }

    buf heartbeat = buf_from_json(heartbeat_json);
    if (!is_valid_payload(heartbeat)) {
        logger_error("Failed to create json payload for heartbeat");
        return;
    }
    send_buf(client, heartbeat);
    client->heartbeat_acknowledged = false;

    buf_destroy(heartbeat);
    json_decref(heartbeat_json);
}

static void heartbeat_cb(struct ev_loop *loop, ev_timer *timer, i32 revents) {
    (void)revents;

    cord_client_t *client = timer->data;

    send_heartbeat(client);
    ev_timer_again(loop, timer);

    logger_debug("Heartbeat");
}

typedef struct cord_t cord_t;

static void sigint_cb(struct ev_loop *loop, ev_signal *signal, i32 revents) {
    (void)revents;

    // Break out of the loop
    if (signal->signum == SIGINT) {
        ev_break(loop, EVBREAK_ALL);
    }
    ev_default_destroy();
}

static void on_open(struct uwsc_client *ws_client) {
    (void)ws_client;
    logger_debug("Connection established");
}

static void on_heartbeat(cord_client_t *client, json_t *data) {
    cord_bump_t *allocator = client->persistent_allocator;

    json_t *hb_interval_json = json_object_get(data, "heartbeat_interval");
    if (!hb_interval_json) {
        logger_error("Failed to get heartbeat object");
        return;
    }
    client->hb_interval = (int)json_integer_value(hb_interval_json);
    send_heartbeat(client);

    if (!client->sent_initial_heartbeat) {
        logger_debug("Sent initial heartbeat");
        client->sent_initial_heartbeat = true;

        struct ev_timer *timer_watcher =
            balloc(allocator, sizeof(struct ev_timer));

        ev_init(timer_watcher, heartbeat_cb);
        timer_watcher->data = client;
        timer_watcher->repeat = heartbeat_to_double(client->hb_interval);
        ev_timer_again(client->ws_client->loop, timer_watcher);
    }
}

static i32 default_intents(void) {
    const i32 guild_intents = GUILDS | GUILD_MODERATION |
                              GUILD_EMOJIS_AND_STICKERS | GUILD_INTEGRATIONS |
                              GUILD_WEBHOOKS | GUILD_INVITES | GUILD_MESSAGES |
                              GUILD_MESSAGE_REACTIONS;

    const i32 message_intents =
        DIRECT_MESSAGES | DIRECT_MESSAGE_REACTIONS | MESSAGE_CONTENT;

    const i32 schedule_intents =
        GUILD_SCHEDULE_EVENTS | AUTO_MODERATION_CONFIGURATION |
        AUTO_MODERATION_EXECUTION | GUILD_MESSAGE_POLLS | DIRECT_MESSAGE_POLLS;

    const i32 intents = guild_intents | message_intents | schedule_intents;
    return intents;
}

static json_t *json_make_child(json_t *obj, const char *key) {
    json_t *child = json_object();
    json_object_set_new(obj, key, child);
    return child;
}

static void send_identify(cord_client_t *client) {
    json_t *payload_json = json_object();
    json_object_set_new(
        payload_json, PAYLOAD_KEY_OPCODE, json_integer(OP_IDENTIFY));

    json_t *d = json_make_child(payload_json, PAYLOAD_KEY_DATA);
    json_object_set_new(d, "token", json_string(client->identity.token));
    json_object_set_new(d, "intents", json_integer(default_intents()));
    json_object_set_new(d, "large_threshold", json_integer(50));
    json_object_set_new(d, "compress", json_boolean(false));

    json_t *properties = json_make_child(d, "properties");
    json_object_set_new(properties, "os", json_string(client->identity.os));
    json_object_set_new(
        properties, "browser", json_string(client->identity.library));
    json_object_set_new(
        properties, "device", json_string(client->identity.device));

    buf payload = buf_from_json(payload_json);
    if (!is_valid_payload(payload)) {
        logger_error("Failed to parse identity json payload");
        json_decref(payload_json);
        return;
    }

    send_buf(client, payload);
    buf_destroy(payload);
}

typedef struct gateway_payload_t {
    i32 op;    // opcode
    i32 s;     // sequence
    char *t;   // event
    json_t *d; // json data
} gateway_payload_t;

void gateway_payload_init(gateway_payload_t *payload) {
    payload->op = -1;
    payload->s = -1;
    payload->t = NULL;
    payload->d = NULL;
}

i32 deserialize_payload(json_t *payload_json, gateway_payload_t *payload) {
    json_t *opcode = json_object_get(payload_json, PAYLOAD_KEY_OPCODE);
    if (!opcode) {
        logger_error("Failed to get \"op\"");
        return -1;
    }

    json_int_t num_opcode = json_integer_value(opcode);
    payload->op = (int)num_opcode;

    if (num_opcode != OP_DISPATCH) {
        payload->t = "";
    } else {
        json_t *t = json_object_get(payload_json, PAYLOAD_KEY_EVENT);
        if (!t) {
            logger_error("Failed to get \"t\"");
            return -1;
        }
        payload->t = json_string_value(t) ? strdup(json_string_value(t)) : "";
    }

    json_t *s = json_object_get(payload_json, PAYLOAD_KEY_SEQUENCE);
    if (s) {
        payload->s = json_integer_value(s);
    }

    json_t *d = json_object_get(payload_json, PAYLOAD_KEY_DATA);
    if (!d) {
        logger_error("Failed to get \"d\"");
        return -1;
    }

    payload->d = json_deep_copy(d);
    return 0;
}

static cord_str_t resolve_message_url(cord_temp_memory_t memory,
                                      cord_message_t *msg) {
    cord_url_builder_t url_builder = cord_url_builder_create(memory.allocator);
    cord_url_builder_add_route(url_builder, cstr(DISCORD_API_URL));
    cord_url_builder_add_route(url_builder, cstr("channels"));
    cord_url_builder_add_route(url_builder,
                               cord_strbuf_to_str(*msg->channel_id));
    cord_url_builder_add_route(url_builder, cstr("messages"));
    return cord_url_builder_build(url_builder);
}

void cord_client_send_message(cord_client_t *client, cord_message_t *msg) {
    assert(msg);

    cord_temp_memory_t memory =
        cord_temp_memory_start(client->persistent_allocator);
    assert(memory.allocator);
    cord_json_writer_t writer = cord_json_writer_create(memory.allocator);
    char *json = cord_message_to_json(writer, msg);
    cord_http_post(
        client->http, memory.allocator, resolve_message_url(memory, msg), json);
    cord_temp_memory_end(memory);
}

void discord_message_destroy(cord_message_t *msg) {
    if (msg) {
        free(msg);
    }
}

static void log_on_message_status(cord_client_t *client) {
    if (client->sent_initial_heartbeat) {
        if (client->heartbeat_acknowledged) {
            logger_debug("ACK");
        } else {
            logger_debug("Zombie");
        }
    }
}

static gateway_payload_t *
parse_gateway_payload(cord_client_t *client, void *data, size_t length) {
    json_error_t err = {};

    json_t *payload_json = json_loadb(data, length, 0, &err);
    if (!payload_json) {
        logger_error("Failed to serialize payload: %s", err.text);
        return NULL;
    }

    gateway_payload_t *payload =
        balloc(client->temporary_allocator, sizeof(gateway_payload_t));

    gateway_payload_init(payload);

    i32 rc = deserialize_payload(payload_json, payload);
    if (rc < 0) {
        logger_error("Failed to parse gateway payload");
        json_decref(payload_json);
        return NULL;
    }

    assert(payload_json->refcount == 1 && "payload referenced more than once");
    json_decref(payload_json);
    return payload;
}

static void on_message(struct uwsc_client *ws_client,
                       void *data,
                       size_t length,
                       bool binary) {
    (void)binary;

    cord_client_t *client = ws_client->ext;
    gateway_payload_t *payload = parse_gateway_payload(client, data, length);

    char *event_name = payload->t;
    json_t *payload_data = json_deep_copy(payload->d);
    json_decref(payload->d);

    switch (payload->op) {
        case OP_DISPATCH:
            if (!cstring_is_empty(event_name)) {
                cord_gateway_event_t *event =
                    get_gateway_event_from_cstring(event_name);
                if (cord_gateway_event_has_handler(event)) {
                    event->handler(client, payload_data, event_name);
                } else {
                    logger_warn("No handler for event: %s", event);
                }
            }
            break;
        case OP_HEARTBEAT:
            logger_debug("Server is requesting Hearbeat");
            on_heartbeat(client, payload_data);
            break;
        case OP_RECONNECT:
            logger_debug("Server is requesting Reconnect");
            client->must_reconnect = true;
            break;
        case OP_INVALID_SESSION:
            logger_warn("Invalid Session");
            break;
        case OP_HELLO:
            if (!client->sent_initial_heartbeat) {
                on_heartbeat(client, payload_data);
                send_identify(client);
            } else {
                on_heartbeat(client, payload_data);
            }
            break;
        case OP_HEARTBEAT_ACK:
            client->heartbeat_acknowledged = true;
            break;
        default: // fallthrough
            logger_error("Default switch case sentinel");
            break;
    }

    json_decref(payload_data);
    log_on_message_status(client);
    cord_bump_clear(client->temporary_allocator);
}

static void on_error(struct uwsc_client *ws_client, i32 err, const char *msg) {
    (void)ws_client;

    logger_error("Connection error (%d): %s", err, msg);
}

static void on_close(struct uwsc_client *ws_client, i32 code, const char *msg) {
    (void)ws_client;
    logger_debug("Closing connection to gateway (%d): %s", code, msg);
}

cord_client_t *cord_client_create(cord_bump_t *allocator) {
    identity_info_t identity = {};
    load_identity_info(&identity);

    cord_client_t *client = balloc(allocator, sizeof(cord_client_t));
    if (!client) {
        logger_error("Failed to allocate discord context");
        return NULL;
    }

    client->persistent_allocator = allocator;
    client->http = cord_http_client_create(allocator, identity.token);
    if (!client->http->bot_token) {
        logger_error("Failed to create bot token");
        free(client);
        return NULL;
    }

    if (!client->http) {
        logger_error("Failed to create http client");
        free(client);
        return NULL;
    }
    client->identity = identity;
    client->hb_interval = -1;
    client->sequence = -1;
    client->sent_initial_heartbeat = false;
    client->loop = NULL;
    client->must_reconnect = false;

    cord_gateway_event_t *on_message_event =
        get_gateway_event(GATEWAY_EVENT_MESSAGE_CREATE);

    on_message_event->handler = on_message_create;

    return client;
}

static const i32 ping_interval = 5;

static void client_init(cord_client_t *client, const char *url) {
    assert(client && "cord_client_t must not be null");

    client->loop = ev_default_loop(0);
    client->ws_client = uwsc_new(client->loop, url, ping_interval, NULL);
    if (!client->ws_client) {
        logger_error("Failed to initialize websocket client");
        exit(1);
    }

    client->message_allocator = cord_bump_create_with_size(MB(10));
    client->temporary_allocator = cord_bump_create_with_size(MB(1));

    client->ws_client->onopen = on_open;
    client->ws_client->onmessage = on_message;
    client->ws_client->onerror = on_error;
    client->ws_client->onclose = on_close;
    client->sent_initial_heartbeat = false;
    client->must_reconnect = false;
    client->ws_client->ext = client;
}

static void client_reconnect_to(cord_client_t *client, const char *url) {
    logger_debug("Attempting to reconnect");

    // Stop heartbeat timer
    ev_timer_stop(client->loop, client->hb_watcher);
    free(client->ws_client);

    client->loop = ev_default_loop(0);
    client->ws_client = uwsc_new(client->loop, url, ping_interval, NULL);
    if (!client->ws_client) {
        logger_error("Failed to initialize websocket client");
        exit(1);
    }

    ev_run(client->loop, 0);
    ev_timer_start(client->loop, client->hb_watcher);
}

static void check_reconnect_cb(struct ev_loop *loop, ev_check *w, i32 revents) {
    (void)loop;
    (void)revents;
    cord_client_t *client = (cord_client_t *)w->data;
    assert(client && "Client must not be null");

    if (client) {
        if (client->must_reconnect) {
            client_reconnect_to(client, DISCORD_WS_URL);
        }
    }
}

static void setup_event_watchers(cord_client_t *client) {
    cord_bump_t *allocator = client->persistent_allocator;
    assert(allocator && "allocator must not be null");

    struct ev_check *reconnect_watcher =
        balloc(allocator, sizeof(struct ev_check));
    reconnect_watcher->data = client;
    ev_check_init(reconnect_watcher, check_reconnect_cb);
    ev_check_start(client->loop, reconnect_watcher);
    client->reconnect_watcher = reconnect_watcher;

    struct ev_signal *sigint_watcher =
        balloc(allocator, sizeof(struct ev_signal));
    sigint_watcher->data = client;
    ev_signal_init(sigint_watcher, sigint_cb, SIGINT);
    ev_signal_start(client->loop, sigint_watcher);
    client->sigint_watcher = sigint_watcher;

    client->health_report_scheduler = NULL;
}

i32 cord_client_connect(cord_client_t *client) {
    logger_debug("Attempting to connect to gateway");

    client_init(client, DISCORD_WS_URL);
    setup_event_watchers(client);
    return ev_run(client->loop, 0);
}

void cord_client_destroy(cord_client_t *client) {
    if (client) {
        if (client->ws_client) {
            free(client->ws_client);
        }
        if (client->hb_watcher) {
            free(client->hb_watcher);
        }
        if (client->sigint_watcher) {
            free(client->sigint_watcher);
        }
        if (client->http) {
            cord_http_client_destroy(client->http);
        }

        cord_bump_destroy(client->temporary_allocator);
        cord_bump_destroy(client->message_allocator);
        free(client);
    }
}
