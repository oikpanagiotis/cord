#include "client.h"
#include "../core/errors.h"
#include "../core/log.h"
#include "../core/typedefs.h"
#include "../core/util.h"
#include "events.h"
#include "types.h"

#include <assert.h>
#include <ev.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <uwsc/config.h>

static string_ref OS_NAME = "Linux";

static void load_identification_info(identification *id) {
    id->token = getenv("CORD_APPLICATION_TOKEN");
    assert(id->token);
    if (!id->token) {
        logger_error(
            "Bot token not found. Please set CORD_APPLICATION_TOKEN envar");
        exit(1);
    }

    id->os = (char *)OS_NAME;
    id->device = LIBRARY_NAME;
    id->library = LIBRARY_NAME;
}

// Assumes that sequence is initialized to -1
static bool is_valid_sequence(int s) {
    return s >= 0;
}

static f64 heartbeat_to_double(i32 interval) {
    i32 decimal = interval / 1000;
    i32 floating_ms = interval % 1000;

    f64 decimal_d = (f64)decimal;
    f64 floating_d = (f64)floating_ms * 0.001;
    return decimal_d + floating_d;
}

typedef struct json_payload_t {
    char *json_string;
    size_t length;
} json_payload_t;

static bool is_valid_json_payload(json_payload_t payload) {
    return payload.json_string && payload.length > 0;
}

static json_payload_t json_payload_from_json(json_t *json) {
    char *json_string = json_dumps(json, 0);
    if (!json_string) {
        return (json_payload_t){"", 0};
    }

    size_t length = strlen(json_string);
    return (json_payload_t){json_string, length};
}

static void json_payload_destroy(json_payload_t payload) {
    if (payload.json_string) {
        free(payload.json_string);
        payload.json_string = NULL;
    }
}

static void debug_sent_payload(json_payload_t payload) {
    logger_debug("Sent JSON payload of size(%d): %s", payload.length,
                 payload.json_string);
}

static void send_json_payload(cord_client_t *client, json_payload_t payload) {
    client->ws_client->send(client->ws_client, payload.json_string,
                            payload.length, UWSC_OP_TEXT);
}

static json_t *heartbeat_json_object_create(i32 sequence) {
    json_t *heartbeat_object = json_object();
    if (!heartbeat_object) {
        return NULL;
    }
    json_object_set_new(heartbeat_object, PAYLOAD_KEY_OPCODE, json_integer(1));

    // Send sequence number unless we havent received one
    json_t *sequence_object =
        is_valid_sequence(sequence) ? json_integer(sequence) : json_null();

    json_object_set_new(heartbeat_object, PAYLOAD_KEY_DATA, sequence_object);
    return heartbeat_object;
}

static void send_heartbeat(cord_client_t *client) {
    json_t *heartbeat_object = heartbeat_json_object_create(client->sequence);
    if (!heartbeat_object) {
        logger_error("Failed to create heartbeat json object");
        return;
    }

    json_payload_t heartbeat = json_payload_from_json(heartbeat_object);
    if (!is_valid_json_payload(heartbeat)) {
        logger_error("Failed to create json payload for heartbeat");
        return;
    }
    send_json_payload(client, heartbeat);
    client->heartbeat_acknowledged = false;

    logger_debug("Heartbeat");

    json_payload_destroy(heartbeat);
    json_decref(heartbeat_object);
}

static void heartbeat_cb(struct ev_loop *loop, ev_timer *timer, int revents) {
    (void)revents;

    struct uwsc_client *ws_client = timer->data;
    cord_client_t *client = ws_client->ext;

    send_heartbeat(client);
    ev_timer_again(loop, timer);
}

static void sigint_cb(struct ev_loop *loop, ev_signal *signal, int revents) {
    (void)revents;

    cord_client_t *client = signal->data;

    // 1. Close any open log files
    // ...

    // 2. Destroy arenas
    cord_bump_destroy(client->temporary_allocator);
    cord_bump_destroy(client->message_allocator);
    cord_bump_destroy(client->persistent_allocator);
    // cord_bump_destroy(client->message_lifecycle_allocator);

    // 3. Break out of the loop
    if (signal->signum == SIGINT) {
        ev_break(loop, EVBREAK_ALL);
    }
    ev_default_destroy();
}

static void on_open(struct uwsc_client *ws_client) {
    (void)ws_client;
    logger_debug("Connection established");
}

static void on_heartbeat(struct uwsc_client *ws_client, json_t *data) {
    cord_client_t *client = ws_client->ext;

    json_t *hb_interval_json = json_object_get(data, "heartbeat_interval");
    if (!hb_interval_json) {
        logger_error("Failed to get heartbeat object");
        return;
    }
    client->hb_interval = (int)json_integer_value(hb_interval_json);

    // Send heartbeat
    send_heartbeat(client);

    if (!client->sent_initial_heartbeat) {
        logger_debug("Sent initial heartbeat");
        client->sent_initial_heartbeat = true;

        struct ev_timer *timer_watcher = malloc(sizeof(struct ev_timer));
        // Keep a reference to the watcher so we can free it later
        client->hb_watcher = timer_watcher;

        ev_init(timer_watcher, heartbeat_cb);
        timer_watcher->data = ws_client;
        timer_watcher->repeat = heartbeat_to_double(client->hb_interval);
        ev_timer_again(ws_client->loop, timer_watcher);
    }
}

static int send_to_gateway(cord_client_t *client, json_t *payload) {
    char *buffer = json_dumps(payload, 0);
    size_t length = strlen(buffer);
    int rc = client->ws_client->send(client->ws_client, buffer, length,
                                     UWSC_OP_TEXT);
    free(buffer);
    return rc;
}

static void debug_identify(json_t *payload) {
    char *debug_dump = json_dumps(payload, 0);
    logger_debug("Sending identify payload: %s", debug_dump);
    free(debug_dump);
}

static void send_identify(struct uwsc_client *ws_client) {
    cord_client_t *client = ws_client->ext;

    json_t *payload = json_object();
    json_object_set_new(payload, PAYLOAD_KEY_OPCODE, json_integer(OP_IDENTIFY));

    json_t *d = json_object();
    json_object_set_new(payload, PAYLOAD_KEY_DATA, d);

    json_object_set_new(d, "token", json_string(client->id.token));
    // json_object_set_new(d, "intents", json_integer(7));
    json_object_set_new(d, "large_threshold", json_integer(50));
    json_object_set_new(d, "compress", json_boolean(false));

    json_t *properties = json_object();
    json_object_set_new(d, "properties", properties);

    json_object_set_new(properties, "os", json_string(client->id.os));
    json_object_set_new(properties, "browser", json_string(client->id.library));
    json_object_set_new(properties, "device", json_string(client->id.device));

    send_to_gateway(client, payload);
    json_decref(payload);
}

typedef struct gateway_payload {
    int op;    // opcode
    int s;     // sequence
    char *t;   // event
    json_t *d; // json data
} gateway_payload;

void gateway_payload_init(gateway_payload *payload) {
    payload->op = -1;
    payload->s = -1;
    payload->t = NULL;
    payload->d = NULL;
}

// parse_gatway_payload() makes a copy of the payload data field
// so the user is safe to free the original json payload
int parse_gatway_payload(json_t *raw_payload, gateway_payload *payload) {
    json_t *opcode = json_object_get(raw_payload, PAYLOAD_KEY_OPCODE);
    if (!opcode) {
        logger_error("Failed to get \"op\"");
        return -1;
    }

    json_int_t num_opcode = json_integer_value(opcode);
    if (num_opcode != OP_DISPATCH) {
        payload->t = "";
    } else {
        json_t *t = json_object_get(raw_payload, PAYLOAD_KEY_EVENT);
        if (!t) {
            logger_error("Failed to get \"t\"");
            return -1;
        }
        payload->t = json_string_value(t) ? strdup(json_string_value(t)) : "";
    }
    payload->op = (int)num_opcode;

    json_t *s = json_object_get(raw_payload, PAYLOAD_KEY_SEQUENCE);
    if (s) {
        payload->s = json_integer_value(s);
    }

    json_t *d = json_object_get(raw_payload, PAYLOAD_KEY_DATA);
    if (!d) {
        logger_error("Failed to get \"d\"");
        return -1;
    }
    payload->d = json_deep_copy(d);
    return 0;
}

void discord_message_set_content(cord_message_t *msg, char *content) {
    cord_strbuf_append(msg->content, cstr(content));
}

char *DISCORD_API_URL = "https://discordapp.com/api";
i32 cord_client_send_message(cord_client_t *client, cord_message_t *msg) {
    assert(msg);
    // TODO: Prefix the channel and ids based on our cache
    const int URL_LEN = 1024;
    char *final_url = calloc(1, sizeof(char) * URL_LEN);
    if (!final_url) {
        logger_error("Failed to allocate final url");
        return -1;
    }

    snprintf(final_url, URL_LEN, "%s/channels/%.*s/messages", DISCORD_API_URL,
             (int)msg->channel_id->length, msg->channel_id->data);

    char *data = cord_strbuf_to_cstring(msg->content);
    cord_http_request_t *request = cord_http_request_create(
        HTTP_POST, final_url, cord_discord_api_header(client->http), data);

    cord_http_client_perform_request(client->http, request);
    free(data);
    free(final_url);
    return 0;
}

void discord_message_destroy(cord_message_t *msg) {
    if (msg) {
        // if (msg->content) free(msg->content);

        free(msg);
    }
}

static void debug_payload(gateway_payload *payload) {
    logger_debug("------------------");
    logger_debug("  Parser Payload");
    char *dump = json_dumps(payload->d, 0);
    logger_debug("  d %s", dump);
    logger_debug("  s %d", payload->s);
    logger_debug("  t %s", payload->t);
    logger_debug("  op %d", payload->op);
    logger_debug("------------------");
    free(dump);
}

static void on_message(struct uwsc_client *ws_client, void *data, size_t length,
                       bool binary) {
    cord_client_t *client = ws_client->ext;
    json_error_t err = {0};

    // Serialize payload
    json_t *raw_payload = json_loadb(data, length, 0, &err);
    if (!raw_payload) {
        logger_error("Failed to serialize payload: %s", err.text);
        return;
    }

    gateway_payload *payload =
        balloc(client->temporary_allocator, sizeof(gateway_payload));

    gateway_payload_init(payload);

    int rc = parse_gatway_payload(raw_payload, payload);
    if (rc < 0) {
        logger_error("Failed to parse gateway payload");
    }

    // When the reference count of a json object reaches decreases
    // by one, the reference count of all it's children is also
    // decreased by 1, so the whole object tree can be cleaned up
    json_decref(raw_payload);

    char *event = payload->t;
    switch (payload->op) {
        // Dispatch is returned if we've received an event from the gateway
        case OP_DISPATCH:
            if (!string_is_empty(event)) {
                logger_debug("Received Event: %s", event);

                cord_gateway_event_t *all_events = get_all_gateway_events();
                cord_gateway_event_t *ev = get_gateway_event(all_events, event);
                if (event_has_handler(ev)) {
                    ev->handler(client, payload->d, event);
                } else {
                    logger_warn("No handler for event(%s)", event);
                }
            }
            break;
        case OP_HEARTBEAT:
            logger_debug("Server is requesting Hearbeat");
            on_heartbeat(ws_client, payload->d);
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
                on_heartbeat(ws_client, payload->d);
                send_identify(ws_client);
            } else {
                on_heartbeat(ws_client, payload->d);
            }
            break;
        case OP_HEARTBEAT_ACK:
            client->heartbeat_acknowledged = true;
            break;
        default: // fallthrough
            logger_error("Default switch case sentinel");
            break;
    }

    if (client->sent_initial_heartbeat) {
        if (client->heartbeat_acknowledged) {
            logger_debug("ACK");
        } else {
            logger_debug("Zombie");
        }
    }

    // json_decref(payload->d);
    cord_bump_clear(client->temporary_allocator);
}

static void on_error(struct uwsc_client *ws_client, int err, const char *msg) {
    (void)ws_client;

    logger_error("Connection error (%d): %s", err, msg);
}

static void on_close(struct uwsc_client *ws_client, int code,
                     const char *reason) {
    (void)ws_client;
    logger_debug("Closing connection to gateway (%d): %s", code, reason);
}

cord_client_t *discord_create(void) {
    identification id = {0};
    load_identification_info(&id);

    cord_client_t *client = malloc(sizeof(cord_client_t));
    if (!client) {
        logger_error("Failed to allocate discord context");
        return NULL;
    }

    client->http = cord_http_client_create(id.token);
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
    client->id = id;
    client->hb_interval = -1;
    client->sequence = -1;
    client->sent_initial_heartbeat = false;
    client->loop = NULL;
    client->must_reconnect = false;

    // TODO: Refactor. Move this somewhere else
    // Assign callbacks
    cord_gateway_event_t *all_events = get_all_gateway_events();
    cord_gateway_event_t *on_message_event =
        get_gateway_event(all_events, "MESSAGE_CREATE");

    on_message_event->handler = on_message_create;

    return client;
}
static void report_memory(struct ev_loop *loop, ev_timer *timer, int revents) {
    (void)loop;
    (void)revents;

    cord_client_t *client = timer->data;

    logger_info("Memory Report");
    f64 message_allocator_used =
        (f64)client->message_allocator->used / (1024 * 1024);
    f64 temporary_allocator_used =
        (f64)client->temporary_allocator->used / (1024 * 1024);
    f64 persistent_allocator_used =
        (f64)client->persistent_allocator->used / (1024 * 1024);
    f64 message_lifecycle_allocator_used =
        (f64)client->message_lifecycle_allocator->used / (1024 * 1024);

    f64 message_allocator_capacity =
        (f64)client->message_allocator->capacity / (1024 * 1024);
    f64 temporary_allocator_capacity =
        (f64)client->temporary_allocator->capacity / (1024 * 1024);
    f64 persistent_allocator_capacity =
        (f64)client->persistent_allocator->capacity / (1024 * 1024);
    f64 message_lifecycle_allocator_capacity =
        (f64)client->message_lifecycle_allocator->capacity / (1024 * 1024);

    logger_info("    Message Allocator (%.2f / %.2f)MB", message_allocator_used,
                message_allocator_capacity);
    logger_info("    Temporary Allocator (%.2f / %.2f)MB",
                temporary_allocator_used, temporary_allocator_capacity);
    logger_info("    Persistent Allocator (%.2f / %.2f)MB",
                persistent_allocator_used, persistent_allocator_capacity);
    logger_info("    Capacity Allocator (%.2f / %.2f)MB",
                message_lifecycle_allocator_used,
                message_lifecycle_allocator_capacity);

    ev_timer_again(loop, timer);
}

static const int ping_interval = 5;

static void client_init(cord_client_t *client, const char *url) {
    client->ws_client = uwsc_new(client->loop, url, ping_interval, NULL);
    if (!client->ws_client) {
        logger_error("Failed to initialize websocket client");
        exit(1);
    }

    client->message_allocator = cord_bump_create_with_size(MB(10));
    client->persistent_allocator = cord_bump_create_with_size(MB(4));
    client->temporary_allocator = cord_bump_create_with_size(MB(1));
    client->message_lifecycle_allocator = cord_bump_create_with_size(KB(16));

    client->ws_client->onopen = on_open;
    client->ws_client->onmessage = on_message;
    client->ws_client->onerror = on_error;
    client->ws_client->onclose = on_close;
    client->sent_initial_heartbeat = false;
    client->must_reconnect = false;
    // Store the wrapper client pointer to be accessed in the callbacks
    client->ws_client->ext = client;
}

static void client_reconnect_to(cord_client_t *client, const char *url) {
    logger_debug("Attempting to reconnect");

    // Turn off timers
    ev_timer_stop(client->loop, client->hb_watcher);
    free(client->hb_watcher);
    free(client->ws_client);

    client_init(client, url);
    ev_run(client->loop, 0);
}

static void check_reconnect_cb(struct ev_loop *loop, ev_check *w, int revents) {
    (void)loop;
    (void)revents;
    cord_client_t *client = (cord_client_t *)w->data;
    assert(client && "Client must not be null");

    if (client) {
        if (client->must_reconnect) {
            client_reconnect_to(client, "wss://gateway.discord.gg");
        }
    }
}

i32 cord_client_connect(cord_client_t *client, const char *url) {
    client->loop = ev_default_loop(0);
    client_init(client, url);
    client->must_reconnect = false;

    logger_debug("Attempting to connect to gateway");
    struct ev_check *reconnect_watcher = malloc(sizeof(struct ev_check));
    reconnect_watcher->data = client;
    ev_check_init(reconnect_watcher, check_reconnect_cb);
    ev_check_start(client->loop, reconnect_watcher);
    client->reconnect_watcher = reconnect_watcher;

    struct ev_signal *sigint_watcher = malloc(sizeof(struct ev_signal));
    sigint_watcher->data = client;
    ev_signal_init(sigint_watcher, sigint_cb, SIGINT);
    ev_signal_start(client->loop, sigint_watcher);
    client->sigint_watcher = sigint_watcher;

    struct ev_timer *health_report_timer = malloc(sizeof(struct ev_timer));
    ev_init(health_report_timer, report_memory);
    health_report_timer->repeat = 120;
    health_report_timer->data = client;
    ev_timer_start(client->loop, health_report_timer);

    ev_run(client->loop, 0);
    return 0;
}

void discord_destroy(cord_client_t *client) {
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
        free(client);
    }
}
